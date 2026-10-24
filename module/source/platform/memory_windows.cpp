#include "memory.hpp"

#include <Platform.hpp>

#if defined SYSTEM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdint>

namespace Vanish
{
	namespace Memory
	{
		bool WritePointer( void **slot, void *value )
		{
			if( reinterpret_cast<uintptr_t>( slot ) % sizeof( void * ) != 0 )
				return false;

			DWORD previous = 0;
			if( VirtualProtect( slot, sizeof( void * ), PAGE_READWRITE, &previous ) == 0 )
				return false;

			// an aligned exchange, so a thread calling through the slot sees
			// either the old function or the new one, never a torn pointer
			InterlockedExchangePointer( slot, value );

			VirtualProtect( slot, sizeof( void * ), previous, &previous );
			return true;
		}

		void PinModule( )
		{
			HMODULE self = nullptr;
			GetModuleHandleExA(
				GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
				reinterpret_cast<LPCSTR>( &PinModule ), &self );
		}
	}
}

#endif
