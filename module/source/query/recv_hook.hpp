#pragma once

#include <Platform.hpp>

#include <cstddef>

#if defined SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

namespace Vanish
{
	namespace Query
	{
		/// Puts a function between the engine and every datagram it reads.
		///
		/// Each build reaches recvfrom differently: the 32-bit Windows engine
		/// through tier0's VCR table, the 64-bit one through its import table,
		/// the 64-bit Linux one through its PLT and the 32-bit Linux one with
		/// direct calls. The types below match what each engine calls, which
		/// is what keeps the arguments and the return value intact.
		namespace RecvHook
		{
#if defined SYSTEM_WINDOWS && defined ARCHITECTURE_X86

			// VCR_t::Hook_recvfrom, a cdecl function on an int socket
			typedef int Socket;
			typedef int Length;
			typedef int Result;
			typedef int AddressLength;
#define VANISH_RECVFROM_CALL __cdecl

#elif defined SYSTEM_WINDOWS

			// ws2_32's recvfrom, whose SOCKET is pointer sized on x64
			typedef SOCKET Socket;
			typedef int Length;
			typedef int Result;
			typedef int AddressLength;
#define VANISH_RECVFROM_CALL WSAAPI

#else

			// libc's recvfrom, whose length and result are 64-bit on x86-64:
			// an int result there would hand the engine a garbage upper half
			typedef int Socket;
			typedef size_t Length;
			typedef ssize_t Result;
			typedef socklen_t AddressLength;
#define VANISH_RECVFROM_CALL

#endif

			typedef Result( VANISH_RECVFROM_CALL *Function )(
				Socket socket, char *buffer, Length length, int flags,
				sockaddr *from, AddressLength *fromlen );

			/// Routes the engine's reads through replacement.
			///@return the function replacement must call to really read, nullptr
			///when the hook could not be installed
			Function Install( Function replacement );

			/// Takes the hook back out.
			///@return false when something hooked the same slot after this module,
			///in which case the hook stays and the module must stay loaded
			bool Remove( );
		}
	}
}
