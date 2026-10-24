#include "gamemodes.hpp"

#include "../main.hpp"

#include <Platform.hpp>

#if !( defined SYSTEM_LINUX && defined ARCHITECTURE_X86 )

#include <scanning/symbolfinder.hpp>

#include <cstdint>
#include <cstring>

namespace Vanish
{
	namespace Query
	{
		namespace Gamemodes
		{
			// IFileSystem::Gamemodes( ) does not sit in the same slot on every
			// branch: 115 on the public 32-bit engine, 119 on the x86-64 branch,
			// which is what the SDK header declares. Calling the wrong slot
			// crashed the server, so the slot is read from the engine's own
			// calls instead: call [vtable + Gamemodes], then call [vtable +
			// Active] on the result.
#if defined SYSTEM_WINDOWS && defined ARCHITECTURE_X86
			static const char CALL[] = "\xFF\x90\x2A\x2A\x2A\x2A\x8B\xC8\x8B\x10\xFF\x52\x10";
			typedef Gamemode::System *( __thiscall *Gamemodes_t )( void * );
#elif defined SYSTEM_WINDOWS
			static const char CALL[] = "\xFF\x90\x2A\x2A\x2A\x2A\x48\x8B\xC8\x48\x8B\x10\xFF\x52\x20";
			typedef Gamemode::System *( *Gamemodes_t )( void * );
#else
			static const char CALL[] = "\xFF\x90\x2A\x2A\x2A\x2A\x48\x8B\x10\x48\x89\xC7\xFF\x52\x20";
			typedef Gamemode::System *( *Gamemodes_t )( void * );
#endif

			// no IFileSystem slot lies anywhere near this far in
			const int32_t MAX_OFFSET = 0x1000;

			///@return the byte offset of Gamemodes( ) in the vtable, -1 when the
			///engine's calls are missing or disagree
			static int32_t SlotOffset( )
			{
				SymbolFinder finder;
				int32_t offset = -1;
				const uint8_t *match = nullptr;
				while( ( match = static_cast<const uint8_t *>( finder.Resolve( global::engine_loader.GetModule( ),
					CALL, sizeof( CALL ) - 1, match != nullptr ? match + 1 : nullptr ) ) ) != nullptr )
				{
					int32_t found = 0;
					std::memcpy( &found, match + 2, sizeof( found ) );
					if( offset != -1 && found != offset )
						return -1;

					offset = found;
				}

				const bool sane = offset >= 0 && offset < MAX_OFFSET && offset % sizeof( void * ) == 0;
				return sane ? offset : -1;
			}

			const IGamemodeSystem::Information *Active( IFileSystem *filesystem )
			{
				static const int32_t offset = SlotOffset( );
				if( filesystem == nullptr || offset < 0 )
					return nullptr;

				void **vtable = *reinterpret_cast<void ***>( filesystem );
				const auto gamemodes = reinterpret_cast<Gamemodes_t>( vtable[offset / sizeof( void * )] );
				Gamemode::System *system = gamemodes( filesystem );
				return system != nullptr ? &system->Active( ) : nullptr;
			}
		}
	}
}

#endif
