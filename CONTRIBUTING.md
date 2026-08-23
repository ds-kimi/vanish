# Contributing

## Building the module

```sh
git submodule update --init --recursive
cd module/projects
premake5 --gmcommon=../../dependencies/garrysmod_common vs2022
```

Build **Release / Win32**. Anything else breaks ABI compatibility with the
game and the module will fail to load or crash.

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
  query/lua_api.*            the vanishquery table
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
and one not, and check each surface listed in the README table. `debug = true`
in the config traces command filtering and transmit state to the server console.
