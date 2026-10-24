# Contributing

## Building the module

```sh
git submodule update --init --recursive
cd module/projects
premake5 --gmcommon=../../dependencies/garrysmod_common vs2022
```

Build **Release / Win32** or **Release / x64**. Debug configurations break ABI
compatibility with the game and the module will fail to load or crash.

On Linux, generate `gmake` instead and build the release configurations:

```sh
premake5 --gmcommon=../../dependencies/garrysmod_common gmake
make -C projects/linux/gmake config=release_x86_64 gmsv_vanish
make -C projects/linux/gmake config=release_x86 gmsv_vanish
```

The 32-bit one needs `g++-multilib`. A build made on a recent distro asks for
that distro's glibc; CI builds in `ubuntu:20.04` so releases need glibc 2.29 at
most.

garrysmod_common is pinned to its `x86-64-support-sourcesdk` branch. Its
master branch pins a Source SDK without the 64-bit libraries, and the SDK on
that branch no longer has `vcrmode.h`, which is why the 32-bit Windows hook
declares the one VCR slot it uses itself.

## The optional module patches

`patches/` holds compatibility fixes for the two upstream modules. Apply them
to a fresh clone:

```sh
git clone --recursive https://github.com/danielga/gm_sourcenet.git
cd gm_sourcenet && git apply ../vanish/patches/gm_sourcenet-engine-compat.patch
```

What they fix:

- **gm_sourcenet** — `Steam3Server` and the `svc_Menu` vtable no longer resolve
  on current builds, and both were fatal at load. They now warn and disable the
  affected feature. `incoming.lua` also gained a return value: sourcenet only
  adopts a rewritten datagram when the hook returns true, so filters must call
  `MarkIncomingModified()`, and a datagram that failed mid-parse is never
  adopted. `CNetChan::m_NetMessages` is no longer where sourcenet's struct
  says it is, so reading it crashed the server on every disconnect: the
  message list is now reported as unavailable instead of walked. Correcting
  the layout would be the better fix and would restore `GetNetMessage`.
  On Linux, `linux_compat.cpp` defines every tier0 and vstdlib symbol the
  module uses and forwards it to the copy the engine loaded, since those
  libraries carry a `_srv` suffix on the public branch and none in
  `bin/linux32` of the x86-64 branch. It also implements the two interlocked
  helpers the x86-64 branch's 32-bit tier0 no longer exports.
gmsv_spoof is no longer a dependency: its query filtering now lives in
`module/source/query/`, rewritten against garrysmod_common interfaces. The
original needed four signature scans (two of which dereferenced their result
before the null check and crashed the server on load), carried a threaded
socket, packet sampling and a firewall this addon never used, and classified
split packets as hostile, which broke every client whose connect payload
exceeded one datagram. The derived files keep its BSD 3-Clause notice.

## Module layout

```
module/source/
  main.cpp                    module entry points and the shared engine loader
  common/lua_args.*           Lua numbers and table fields read without UB
  vanish/hidden_set*          which entities are hidden, which viewers are exempt
  vanish/check_transmit.*     clears transmit bits after the engine's own pass
  vanish/transmit_strip.*     the per client work of that hook
  vanish/delta_promote.*      decides when a leave-PVS header becomes a delete
  vanish/delta_header_windows SV_WriteDeltaHeader detour, both Windows engines
  vanish/delta_header_linux   splices a stub into WriteDeltaEntities on Linux
  vanish/delta_stub_linux*    that stub and its patch site, per architecture
  vanish/entity_write_info.*  the engine struct offsets, per architecture
  vanish/hooks.*              installs and removes the two hooks
  vanish/game_server.*        the engine's IServer on every build
  vanish/build_number.*       the build status prints, read off its getter
  vanish/serverinfo*, stats*  the vanishinfo table
  query/state.*, roster.*     the roster and flags queries are answered from
  query/recv_hook*            reaches recvfrom, one file per build
  query/game_socket.*         tells the game socket from the engine's others
  query/socket_filter.*       the hook that answers queries in the engine's place
  query/answer.*, sender.*    builds a reply and sends it, split when long
  query/replies*, gamemodes*  the A2S_INFO and A2S_PLAYER payloads
  query/challenge.*           the challenge a query must echo before it is answered
  query/rate_limit.*          the engine's per address and global query limits
  query/lua_*                 the vanishquery table
  userinfo/                   the vanishuserinfo table
  event/                      the vanishevent table
  platform/                   pointer writes, code patching, and the ELF and PE
                              readers the hooks above locate the engine with
```

One concern per file: state, the hooks, the engine layout, and the Lua surface
are all separate. `hidden_set` knows nothing about the engine, and the hooks
know nothing about Lua. Platform specific files guard their whole content with
`#if`, so every configuration compiles the same file list.

## Rules the module keeps

- **Nothing on the engine's read path may throw or block.** The recvfrom hook
  catches everything, takes one uncontended lock, and only ever sends
  non-blocking UDP.
- **Lua errors skip C++ destructors** on the 32-bit Windows build, where LuaJIT
  unwinds with longjmp. Every `CheckType` comes before anything with a
  destructor exists, table fields are read with `RawGet` so no metamethod can
  raise, and no Lua call is ever made while the query lock is held.
- **No double reaches an integer cast unclamped.** Lua hands over NaN, inf and
  1e300 as readily as anything else; `LuaArgs::ClampInteger` maps them.
- **No cached pointer outlives what it points into.** The userinfo string table
  is recreated every map, so it is looked up on every call.
- **A hook that cannot be taken back pins the module.** When something else has
  chained onto a slot after this module, restoring the slot would cut it out
  and unloading would leave it calling freed code, so the hook stays as a pass
  through and the module stays loaded.

## Working on engine hooks

Offsets and signatures in `module/source/vanish/` and `module/source/query/`
are specific to a GMod build. When they break:

1. **Analyse a copy of the binary that is actually running.** IDA loads an
   existing `.i64`/`.id0` database in preference to re-reading the file, so a
   stale database next to `engine.dll` will silently hand you addresses from an
   older build. Copy the DLL elsewhere first.
2. Find `SV_WriteEnterPVS` by the string
   `SV_CreatePacketEntities: GetEntServerClass failed for ent %d.`, and
   `CBaseServer::WriteDeltaEntities` by its VPROF string of the same name.
3. Confirm the `CEntityWriteInfo` layout in `CBaseServer::WriteDeltaEntities`:
   the struct is on its stack, the deletion bitvec is the `0x400`-byte field
   that gets `memset` to zero, and the client entity index is copied into it
   from the client. The pointer members ahead of them move both offsets
   between architectures:

   | | m_pBuf | client entity | deletion flags |
   |---|---|---|---|
   | x86 | `0x24` | `0x28` | `0x34` |
   | x86-64 | `0x38` | `0x40` | `0x58` |

4. Verify a new signature is unique **against the file on disk**, not just in
   the database.

A failed scan must never be fatal. Warn, disable the feature, and let the rest
of the addon run.

### Windows

- **SV_WriteDeltaHeader** is a real function on both engines and is detoured.
  Every leave-PVS header goes through it; only `SV_WriteEnterPVS` inlines its
  own copy, which never carries the leave flag.
- **recvfrom.** The 32-bit engine imports no `recvfrom`: `NET_ReceiveRawPacket`
  loads `Hook_recvfrom` from tier0's `g_pVCR` (index 11, `+0x2C`) and calls it,
  so that table entry is swapped. The 64-bit engine imports `recvfrom` from
  `ws2_32.dll` by ordinal 17, so its import address table slot is swapped.
- **No `sv` export on the 32-bit engine**, and garrysmod_common's fallback
  signature no longer matches it. `GameServer::Get` follows MSVC RTTI there:
  `.?AVCGameServer@@`, its complete object locator, the vtable, and the one
  object in a writable section pointing at it. The 64-bit engine exports `sv`.
- **`Gamemodes( )` is slot 115 on the 32-bit public engine** and 119 on the
  `x86-64` branch, which is what the SDK header declares. `gamemodes.cpp` reads
  the slot from the engine's own `call [vtable + Gamemodes]` sites and refuses
  to call anything when they disagree.

### Linux

The 64-bit engine is position independent and keeps its dynamic symbols; the
32-bit one is neither, so most hooks take a different route per architecture.
Analyse `bin/linux64/engine.so` and `bin/linux32/engine.so`.

- **SV_WriteDeltaHeader is inlined** into every caller on both, so there is
  nothing to detour. `delta_header_linux.cpp` patches the leave-PVS branch of
  `CBaseServer::WriteDeltaEntities` at the point both of its paths meet with
  the flags settled: the flags register is `edi` (x86) or `r13d` (x86-64), set
  to 1, or 3 on the explicit delete path, and the entity is in `ebx`. The
  instructions there (8 bytes on x86, 15 on x86-64, none of them relative) move
  into a stub that saves every register, the flags and the FPU/SSE state, calls
  `Promote`, and writes the result back into the flags register. On x86-64 the
  stub steps over the red zone first and is reached with an absolute jump,
  since it can be mapped anywhere. Stubs are built writable, then sealed read
  and execute.
- **recvfrom.** The 64-bit engine calls it through its PLT, so the
  `R_X86_64_JUMP_SLOT` entry in its GOT is swapped. Until the first call that
  slot holds the lazy binding stub, which must never be called as the
  original: the dynamic linker would resolve it and overwrite the hook. The
  32-bit engine calls libc directly through relocated `call rel32`
  instructions, so `ElfCalls::Redirect` rewrites those call sites from the
  `R_386_PC32` relocations in the file. libc's own `recvfrom` must not be
  detoured: it starts with a call to a PC thunk.
- **libc's `recvfrom` returns `ssize_t`** and takes a `size_t` length. The hook
  is declared with exactly those types; an `int` result would hand the 64-bit
  engine a garbage upper half.
- **`sv`** is exported by the 64-bit engine. The 32-bit one is stripped of it,
  so `GameServer::Get` follows RTTI there: the type name `11CGameServer`, its
  typeinfo, its vtable, and the one object in the engine's writable segments
  that points at that vtable.
- **No usable `ICvar`.** garrysmod_common's interface name does not match what
  every build's vstdlib exports with the SDK layout. The convars the query
  replies follow are read by Lua with `GetConVar` and handed over through
  `vanishquery.SetConVars` instead, on every platform.
- **`GetServerVersion` returns 0** on every engine, since a dedicated server's
  `steam.inf` has no `ServerVersion`. The build `status` prints comes from
  `build_number( )`, found through its one call inside the status command and
  only called when it is the plain getter the engines ship.

Patterns and offsets must be unique in the file on disk; check them with a
script over `engine.so` / `engine.dll` before relying on them.

## Addon layout

```
addon/lua/vanish/
  sv_config.lua      every setting, basic first and advanced below
  core/              state, the command, persistence, the captured originals
  hiding/            entities, the Lua libraries, queries, sound, voice, chat,
                     combat, userinfo
  fake/              the fake disconnect and rejoin, and the protections
  console/           status, ping, users and stats replies, and the filter
```

`core` owns the state, `hiding` makes a player disappear from one surface each,
`fake` stages the departure, and `console` deals with what the engine would
otherwise print. Load order is
declared once, in `autorun/server/vanish.lua`.

## Lua style

- EmmyLua annotations on functions, classes and non-obvious fields.
- Comment why, not what.
- Max 4 functions per file, max 30 lines per function; split the file instead of
  growing one.
- Validate everything a client can influence: type, range, length.

## Testing a change

There are no automated tests. Verify by hand with two clients, one whitelisted
and one not, and check each surface listed in the README table. On Linux, a
query tool on `127.0.0.1` may get no answer even from a stock server; query
the machine's LAN address instead. `debug = true`
in the config traces command filtering and transmit state to the server console.
