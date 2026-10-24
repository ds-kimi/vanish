#include "gamemodes.hpp"

#include "../main.hpp"

#include <Platform.hpp>

#if defined SYSTEM_LINUX && defined ARCHITECTURE_X86

#include <scanning/symbolfinder.hpp>

#include <cstdint>
#include <cstring>

namespace Vanish
{
	namespace Query
	{
		namespace Gamemodes
		{
			// The 32-bit Linux filesystem has four virtual functions ahead of
			// Gamemodes( ) that the SDK header does not declare, so calling it
			// through the header lands on the wrong slot and returns NULL. The
			// engine's own tag code is read instead, for both the object and
			// the slot it calls:
			//
			//   mov  eax, [g_pFullFileSystem]
			//   mov  edx, [eax]
			//   mov  [esp], eax
			//   call [edx + Gamemodes]
			//   mov  edx, [eax]
			//   mov  [esp], eax
			//   call [edx + 0x10]            Active( )
			static const char GAMEMODES_CALL[] =
				"\xA1\x2A\x2A\x2A\x2A\x8B\x10\x89\x04\x24\xFF\x92\x2A\x2A\x2A\x2A"
				"\x8B\x10\x89\x04\x24\xFF\x52\x10";

			const size_t FILESYSTEM_OPERAND = 1;
			const size_t GAMEMODES_OPERAND = 12;

			const IGamemodeSystem::Information *Active( IFileSystem * )
			{
				static const uint8_t *code = nullptr;
				if( code == nullptr )
				{
					SymbolFinder finder;
					code = static_cast<const uint8_t *>( finder.Resolve( global::engine_loader.GetModule( ),
						GAMEMODES_CALL, sizeof( GAMEMODES_CALL ) - 1 ) );
				}

				if( code == nullptr )
					return nullptr;

				void **global_filesystem = nullptr;
				int32_t offset = 0;
				std::memcpy( &global_filesystem, code + FILESYSTEM_OPERAND, sizeof( global_filesystem ) );
				std::memcpy( &offset, code + GAMEMODES_OPERAND, sizeof( offset ) );

				void *instance = *global_filesystem;
				if( instance == nullptr || offset < 0 || offset % sizeof( void * ) != 0 )
					return nullptr;

				typedef Gamemode::System *( *Gamemodes_t )( void * );
				void **vtable = *static_cast<void ***>( instance );
				Gamemode::System *gamemodes = reinterpret_cast<Gamemodes_t>( vtable[offset / sizeof( void * )] )( instance );
				return gamemodes != nullptr ? &gamemodes->Active( ) : nullptr;
			}
		}
	}
}

#endif
