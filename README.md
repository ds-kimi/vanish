# vanish-gmod

![build](https://github.com/ds-kimi/vanish-gmod/actions/workflows/build.yml/badge.svg)

Server-side player hiding for Garry's Mod. A whitelisted player runs `vanish`
and disappears from the world, from every scoreboard, from `status` and `ping`,
from server-side Lua, and from A2S query tools.

No client-side code. Nothing is shipped to clients, so nothing depends on a
client cooperating.

## What it hides

| Surface | How |
|---|---|
| Player entity, weapons, children, ragdoll | `CheckTransmit` hook clears the transmit bit per receiving client |
| Scoreboards and client `player.GetAll()` | `SV_WriteDeltaHeader` detour turns the leave-PVS header into a delete |
| `status`, `ping`, `users`, `stats` | command dropped at the netchannel, answered with a filtered listing |
| Server-side Lua | the roster, lookup, iterator, spatial search and team counter functions, listed below |
| A2S_INFO / A2S_PLAYER | `recvfrom` detour answers queries from the visible roster |
| Client leave notice, client side addons | a real `player_disconnect` game event is broadcast, so every client runs its own disconnect path |
| Admin mod logs | the selected server side `PlayerDisconnected` handlers are run for a player that never left, mirrored by the join handlers on unvanish |
| Footsteps, weapon fire, physics, voice, chat | suppressed at the hook that emits them, since sound and voice are networked by position and name rather than by transmit state |
| Kill feed, damage logs, death events | damage they deal is blocked, suicide is blocked, and the attack buttons are stripped so no tracer or decal is sent |
| userinfo string table | optional, blanks the last copy of their name and SteamID left on every client |

Staff listed in `allowed` keep seeing vanished players, and a vanished player
always sees themselves.

## Warning about A2S spoofing

`hideFromQueries` makes the server answer A2S_INFO and A2S_PLAYER itself, with
the hidden player removed and the player count reduced. That is the same
mechanism player-count spoofers use, and Facepunch treats a server whose query
replies disagree with its real state as fraudulent: servers doing this have
been delisted from the in-game server browser, and the ban applies to the
server, not to the addon.

The risk is yours to weigh. A private server nobody queries is a different
situation from a public one advertising on the master list. If you are not
prepared to be delisted, set `hideFromQueries = false` and accept that query
tools still see the vanished player; every other surface stays hidden.

It ships **enabled**. If your server is public, decide deliberately whether to
keep it that way.

## Why a module is needed

`Entity:SetPreventTransmit` does not hide players. `CBasePlayer` edicts are
marked `FL_EDICT_ALWAYS`, and the engine writes them to every client's snapshot
regardless. Clearing the transmit bit after the engine's own pass fixes that,
but only halfway: a per-client hide makes the client mark the entity *dormant*
rather than delete it, because the engine only sets `FHDR_DELETE` when an
entity leaves the **global** frame snapshot. A dormant entity keeps its name
and SteamID in the client's entity list, which is why it still appears on
scoreboards. `gmsv_vanish` detours `SV_WriteDeltaHeader` and promotes the
header to a delete for hidden entities, per receiving client.

## Requirements

- Garry's Mod dedicated server, **Windows 32-bit (`win32`) only for now**.
  Linux and 64-bit builds are coming: the engine offsets the hooks rely on were
  verified against the x86 Windows engine and each other platform needs its own
  check before it can be trusted.
- `gmsv_vanish` — this repository. Hides entities and answers A2S queries.
- [gm_sourcenet](https://github.com/danielga/gm_sourcenet) — optional, needed
  to filter `status` and `ping`. It needs the compatibility patch in
  [`patches/`](patches) to load on current GMod builds; see
  [CONTRIBUTING.md](CONTRIBUTING.md).

Each feature degrades on its own: without a module the matching config option
does nothing and the rest keeps working.

## Downloads

Every commit publishes a release with one archive per platform, currently
`vanish-gmod-win32.zip`. Each archive mirrors the `garrysmod` directory:

```
garrysmod/lua/bin/gmsv_vanish_win32.dll
garrysmod/lua/bin/gmsv_sourcenet_win32.dll
garrysmod/addons/vanish/lua/...
```

Every feature still degrades on its own: with the `sourcenet` Lua missing,
`status` and `ping` filtering reports itself unavailable and the rest of the
addon carries on.

Extract it over your `garrysmod` folder, set `allowed` in the config, restart.
gm_sourcenet is built from its pinned upstream commit with the patch in
[`patches/`](patches) already applied.

## Building

```sh
git clone --recursive https://github.com/ds-kimi/vanish-gmod.git
cd vanish-gmod/module/projects
premake5 --gmcommon=../../dependencies/garrysmod_common vs2022   # or gmake
```

Build the generated project in **Release / Win32**, then copy
`gmsv_vanish_win32.dll` into `garrysmod/lua/bin/`.

That builds this repository's module only. `gmsv_sourcenet_win32.dll` has to
come from the release archive or from your own build of the pinned commit with
the patch in [`patches/`](patches) applied; the CI workflow does exactly that,
see [.github/workflows/build.yml](.github/workflows/build.yml).

The Lua half of gm_sourcenet is vendored in
[`addon/lua/sourcenet/`](addon/lua/sourcenet), patched, so a plain clone is
installable. CI fails the build if it drifts from the pinned upstream.

## Installing

1. Copy [`addon/`](addon) to `garrysmod/addons/vanish/`. The patched
   `sourcenet` Lua is vendored in [`addon/lua/sourcenet/`](addon/lua/sourcenet)
   and comes along with it.
2. Copy the module DLLs to `garrysmod/lua/bin/`.
3. Put your SteamID in `allowed` in
   [`addon/lua/vanish/sv_config.lua`](addon/lua/vanish/sv_config.lua).
4. Restart the server and run `vanish` in your console to toggle.

## Layout

```
addon/lua/
  sourcenet/                  vendored gm_sourcenet Lua, patched
  autorun/server/vanish.lua   loads everything in order
  vanish/sv_config.lua        every setting lives here
  vanish/core/                vanish state and the console command
  vanish/hiding/              entities, the Lua libraries, A2S queries,
                              sound, voice, chat, combat, userinfo
  vanish/fake/                the fake disconnect and vanished protections
  vanish/console/             status and ping replies, command filtering
```

## Configuration

Basic settings sit at the top of `vanish/sv_config.lua`:

```lua
allowed         -- SteamIDs that may toggle vanish, and that keep seeing others
hideEntities    -- hide the player entity          (needs gmsv_vanish)
filterCommands  -- filter status and ping          (needs gmsv_sourcenet)
hideFromQueries -- hide from A2S                   (needs gmsv_vanish)
fakeDisconnect  -- broadcast a fake player_disconnect     (needs gmsv_vanish)
protect         -- godmode, noclip, notarget, no collisions
persist         -- keep the vanish across a map change
hideUserinfo    -- blank the userinfo entry             (needs gmsv_vanish)
debug           -- server console tracing
```

Advanced settings follow in the same file: the blocked command list, the
transmit refresh interval, and the `status` header fields. By default the
synthesised `status` reports the server's real version, public address and
SteamID; set `status.spoof` to `true` to send fixed strings instead.

## Faking the disconnect

Hiding the entity removes a player from the world, but a client that simply
loses an entity says nothing about it. `fakeDisconnect` broadcasts the engine's
own `player_disconnect` event, which is what makes the client print
"Name has left the game (reason)", drop the player from its own tracking and
run every client side addon listening for that event. Lua cannot raise a game
event, so the module exposes `vanishevent.Fire(name, fields)` for it.

Server side, `disconnect.hooks` selects which `PlayerDisconnected` handlers
run, so ULX, SAM and similar log the departure:

- `"none"` - nothing runs, only clients are told
- `"match"` (default) - handlers whose identifier contains one of
  `disconnect.hookPatterns`, by default `ulx`, `ulib`, `log`, which reaches the
  logging handlers, ULX's and DarkRP's FAdmin among them, and nothing that
  changes player state
- `"all"` - every handler, gamemode logic included. DarkRP sells doors, drops
  money and saves the player's data on disconnect, and none of that is undone
  when the player unvanishes. Opt in deliberately.

Unvanishing broadcasts `player_connect` so the reappearance reads as a
rejoin; set `disconnect.announceRejoin` to `false` to return silently.

Every client is told, staff included: the engine has no way to send a game
event to a chosen subset of clients, so an allowed viewer sees the leave
message even though the player stays visible to them.

## Protection

`protect` gives a vanished player godmode, noclip, notarget and pass-through
collisions, restored on unvanish from the state captured when they vanished.
All four are engine level and reachable from Lua - `GodEnable` sets
`FL_GODMODE`, `SetMoveType` writes `MOVETYPE_NOCLIP` - so no module is needed;
`EntityTakeDamage` additionally blocks code that damages players without
consulting `FL_GODMODE`. Individual pieces toggle under
`Vanish.Config.protection`.

## What server-side Lua sees

Every function that can reach a player without listing the roster is masked,
because filtering only the list functions leaves each of these wide open:

| Library | Masked |
|---|---|
| `player` | `GetAll`, `GetHumans`, `GetCount`, `Iterator`, `GetBySteamID`, `GetBySteamID64`, `GetByAccountID`, `GetByUniqueID`, `GetByID` |
| `ents` | `GetAll`, `Iterator`, `FindByClass`, `FindByModel`, `FindByName`, `FindInSphere`, `FindInBox`, `FindInCone`, `FindAlongRay` |
| `team` | `GetPlayers`, `NumPlayers`, `TotalFrags`, `TotalDeaths` |

`player.Iterator` and `ents.Iterator` matter most: they are what addons written
since 2023 use instead of `GetAll`, and an unfiltered iterator hands out every
vanished player no matter how well the list functions are masked. The team
counters matter for the same reason in reverse: a team whose list is one
shorter but whose count is not tells any addon comparing them that somebody
is hidden.

## Silence

A hidden entity is still an audible one. Sounds are networked by position, not
by the transmit state of the entity that made them, so `Vanish.Config.silence`
stops them at the hook that emits them:

| Setting | Stops |
|---|---|
| `sounds` | footsteps, weapon fire, physics impacts, anything the player or their weapons emit |
| `voice` | voice chat, staff still hear a vanished player |
| `chat` | chat messages, echoed to staff and the speaker instead |
| `blockOutgoingDamage` | damage they deal, which is what the kill feed and damage logs react to |
| `blockSuicide` | a death that would broadcast an event and drop a named ragdoll |
| `blockAttacks` | the attack buttons, since firing sends tracers and decals to everyone in range regardless of damage. Set `false` to keep the physgun and toolgun usable |

## Persistence

`persist` writes the vanish set to `data/vanish_state.txt` and restores it at
`PlayerAuthed`, before the first spawn, so the player is hidden without ever
having been transmitted. A restore is silent: announcing a departure for a
player nobody has seen yet would invent one. A real disconnect while vanished
carries the state forward too, since leaving is not a decision to be visible.

## Limitations

- `hideUserinfo` blanks the string table entry for everybody, the vanished
  player included, so their own client draws a blank name in any HUD reading
  `LocalPlayer():Nick()`. Off by default for that reason.
- Every client is told about the fake disconnect, staff included: the engine
  cannot send a game event to a subset of clients.
- Doors opening, props moving and buttons being pressed still happen, and
  props spawned while vanished stay visible.
- The server console and RCON see the real state. Filtering there was left out
  deliberately: it hides nothing from players, only from co-owners.
- Addons that cache the player list at join time instead of calling
  `player.GetAll()` will still hold a reference.

## License

BSD 3-Clause. See [LICENSE](LICENSE).

The A2S query filtering under `module/source/query/` is derived from
[gmsv_spoof](https://github.com/OmarAssadi/gmsv_spoof), itself derived from
gmsv_serversecure by Daniel Almeida, BSD 3-Clause: see
[`licenses/gmsv_spoof.txt`](licenses/gmsv_spoof.txt).

`garrysmod_common` and gm_sourcenet are the property of their respective
authors and carry their own licenses.
