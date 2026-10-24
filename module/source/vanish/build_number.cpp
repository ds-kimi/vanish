#include "build_number.hpp"

#include "../main.hpp"

#include <Platform.hpp>

#include <scanning/symbolfinder.hpp>

#include <cstdint>
#include <cstring>

namespace Vanish
{
	namespace BuildNumber
	{
		// GetServerVersion reads a ServerVersion key that a dedicated server's
		// steam.inf does not carry, so it returns 0, and no engine exports
		// build_number( ). The build is taken from the one call to it inside
		// the status command instead, whose result becomes the second %d of
		// the version line, and the callee must be the plain getter it is on
		// every shipping engine: a load of the global, then a return.
#if defined SYSTEM_LINUX && defined ARCHITECTURE_X86_64
		// push rbp; mov eax, [rip + build]; mov rbp, rsp; pop rbp; ret
		static const char CALL[] =
			"\xE8\x2A\x2A\x2A\x2A\x48\x8B\x95\x2A\x2A\x2A\x2A\x4C\x8B\x8D\x2A\x2A\x2A\x2A\x89\xC1\x4D\x89\xF0";
		static const uint8_t HEAD[] = { 0x55, 0x8B, 0x05 };
		static const uint8_t TAIL[] = { 0x48, 0x89, 0xE5, 0x5D, 0xC3 };
		const size_t TAIL_AT = 7;
#elif defined SYSTEM_LINUX
		// push ebp; mov eax, [build]; mov ebp, esp; pop ebp; ret
		static const char CALL[] =
			"\xE8\x2A\x2A\x2A\x2A\x8B\x95\x2A\x2A\x2A\x2A\x89\x7C\x24\x14\x89\x44\x24\x0C";
		static const uint8_t HEAD[] = { 0x55, 0xA1 };
		static const uint8_t TAIL[] = { 0x89, 0xE5, 0x5D, 0xC3 };
		const size_t TAIL_AT = 6;
#elif defined ARCHITECTURE_X86_64
		// mov eax, [rip + build]; ret
		static const char CALL[] =
			"\xE8\x2A\x2A\x2A\x2A\x48\x89\x7C\x24\x30\x48\x8D\x15\x2A\x2A\x2A\x2A\x44\x8B\xC8"
			"\x48\x89\x74\x24\x28\x41\xB8\x18\x00\x00\x00";
		static const uint8_t HEAD[] = { 0x8B, 0x05 };
		static const uint8_t TAIL[] = { 0xC3 };
		const size_t TAIL_AT = 6;
#else
		// mov eax, [build]; ret
		static const char CALL[] =
			"\xE8\x2A\x2A\x2A\x2A\x50\x6A\x18\x68\x2A\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xFF\xD3\x83\xC4\x1C";
		static const uint8_t HEAD[] = { 0xA1 };
		static const uint8_t TAIL[] = { 0xC3 };
		const size_t TAIL_AT = 5;
#endif

		///@return the build_number( ) getter, nullptr when not found
		static int ( *FindGetter( ) )( )
		{
			SymbolFinder finder;
			const uint8_t *call = static_cast<const uint8_t *>( finder.Resolve(
				global::engine_loader.GetModule( ), CALL, sizeof( CALL ) - 1 ) );
			if( call == nullptr )
				return nullptr;

			int32_t relative = 0;
			std::memcpy( &relative, call + 1, sizeof( relative ) );
			const uint8_t *function = call + 5 + relative;

			// anything but the plain getter means the signature landed somewhere
			// it was not written for, and calling it would be a guess
			if( std::memcmp( function, HEAD, sizeof( HEAD ) ) != 0 ||
				std::memcmp( function + TAIL_AT, TAIL, sizeof( TAIL ) ) != 0 )
				return nullptr;

			return reinterpret_cast<int ( * )( )>( const_cast<uint8_t *>( function ) );
		}

		int Get( )
		{
			// only the getter is cached: the value can still be 0 if this runs
			// before the engine has stored the build
			static int ( *const getter )( ) = FindGetter( );
			return getter != nullptr ? getter( ) : 0;
		}
	}
}
