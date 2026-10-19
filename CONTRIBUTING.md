# Contributing

## Building the module

```sh
git submodule update --init --recursive
cd module/projects
premake5 --gmcommon=../../dependencies/garrysmod_common vs2022
```

Build **Release / Win32**. Anything else breaks ABI compatibility with the
game and the module will fail to load or crash.

On Linux, generate `gmake` instead and build the 32-bit release configuration:

```sh
premake5 --gmcommon=../../dependencies/garrysmod_common gmake
make -C projects/linux/gmake config=release_x86 gmsv_vanish
```

This needs `g++-multilib`. A build made on a recent distro asks for that
distro's glibc; CI builds in `ubuntu:20.04` so releases need glibc 2.29 at most.

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
  main.cpp                   module entry points and the shared engine loader
  vanish/hidden_set.*        which entities are hidden, which viewers are exempt
  vanish/check_transmit.*    clears transmit bits after the engine's own pass
  vanish/delta_header.*      promotes a leave-PVS header into a delete
  vanish/entity_write_info.* the engine struct offsets both hooks depend on
  vanish/hooks.*             installs and removes the two hooks
  vanish/lua_api.*           the transmitcontrol table
  vanish/serverinfo.*        the vanishinfo table
  query/state.*              the roster and flags queries are answered from
  query/replies.*            builds the A2S_INFO and A2S_PLAYER payloads
  query/socket_filter.*      the recvfrom detour that answers them
  query/challenge.*          the challenge a query must echo before it is answered
  query/lua_api.*            the vanishquery table
  vanish/game_server.*       the engine's IServer, found through RTTI on Linux
  platform/                  Linux only: code patching, ELF relocations and RTTI
                             lookups, and the tier0 forwarders
```

One concern per file: state, the two hooks, the engine layout, and the Lua
surface are all separate. `hidden_set` knows nothing about the engine, and the
hooks know nothing about Lua.

## Working on engine hooks

Offsets and signatures in `module/source/vanish/` are specific to a GMod
build. When they break:

1. **Analyse a copy of the binary that is actually running.** IDA loads an
   existing `.i64`/`.id0` database in preference to re-reading the file, so a
   stale database next to `engine.dll` will silently hand you addresses from an
   older build. Copy the DLL elsewhere first.
2. Find `SV_WriteEnterPVS` by the string
   `SV_CreatePacketEntities: GetEntServerClass failed for ent %d.`; the first
   call in it is `SV_WriteDeltaHeader`.
3. Confirm the `CEntityWriteInfo` layout in `CBaseServer::WriteDeltaEntities`:
   the client entity index is loaded from `client+0x10`, and the deletion
   bitvec is the `0x400`-byte field that gets `memset` to zero.
4. Verify a new signature is unique **against the file on disk**, not just in
   the database.

A failed scan must never be fatal. Warn, disable the feature, and let the rest
of the addon run.

### Linux

The 32-bit Linux engine is built differently enough that most hooks take a
different route there. Analyse `bin/linux32/engine.so`; its `.symtab` is
stripped down to a few hundred entries, but RTTI and relocations survive.

- **SV_WriteDeltaHeader is inlined** into every caller, so there is nothing to
  detour. `delta_header.cpp` patches the leave-PVS branch of
  `CBaseServer::WriteDeltaEntities` instead: find the function by its VPROF
  string `CBaseServer::WriteDeltaEntities`, then the branch taken when the
  update type is 1, which sets `edi` to the flags (`mov edi, 1`, or 3 on the
  explicit delete path) and `ebx` to the entity, and meets at a `mov eax, ebx;
  mov esi, [ebp+m_pBuf]` pair. Those eight bytes are moved into a stub that
  calls `Promote` and writes the result back into `edi`. The
  `CEntityWriteInfo` offsets are the same as on Windows.
- **No VCR table.** The engine calls libc's `recvfrom` directly through
  relocated `call rel32` instructions, so `ElfModule::RedirectCalls` rewrites
  those call sites from the `R_386_PC32` relocations in the file. libc's own
  `recvfrom` must not be detoured: it starts with a call to a PC thunk.
- **No `sv` symbol.** `GameServer::Get` follows RTTI: the type name
  `11CGameServer`, its typeinfo, its vtable, and the one object in the engine's
  writable segments that points at that vtable.
- **The filesystem vtable has four extra slots** before `Gamemodes( )`, so
  anything past them in `IFileSystem` must not be called through the SDK
  header. `replies.cpp` reads the slot from the engine's own tag code, and
  reads `steam.inf` with `IBaseFileSystem::Read` rather than `ReadLine`.
- **`GetServerVersion` returns 0**, since a dedicated server's `steam.inf` has
  no `ServerVersion`. The build `status` prints comes from `build_number( )`,
  found through its one call inside the status command.

Patterns and offsets must be unique in the file on disk; check them with a
script over `engine.so` before relying on them.

## Addon layout

```
addon/lua/vanish/
  sv_config.lua      every setting, basic first and advanced below
  core/              state.lua, command.lua
  hiding/            entities.lua, lua_players.lua, lua_entities.lua, queries.lua
  console/           netchannel.lua, serverinfo.lua, status.lua, ping.lua, filter.lua
```

`core` owns the state, `hiding` makes a player disappear from one surface each,
and `console` deals with what the engine would otherwise print. Load order is
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
