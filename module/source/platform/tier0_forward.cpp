// tier0 is libtier0_srv.so on the public Linux branch and libtier0.so in
// bin/linux32 of the x86-64 branch. Linking either by name makes the module
// fail to load on the other branch, or load a second copy of tier0 whose
// output never reaches the engine's console. The few tier0 functions the
// module uses are defined here instead and forwarded to the copy the engine
// already loaded, so the module itself needs neither name.

#include <tier0/dbg.h>
#include <tier0/platform.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <dlfcn.h>

namespace
{
	///@return the named function from the tier0 the engine loaded
	void *Tier0( const char *name )
	{
		// libtier0.so first: on the x86-64 branch the _srv name may exist as
		// a hand made link, and that copy is not the one the engine talks to
		const char *libraries[] = { "libtier0.so", "libtier0_srv.so" };
		for( const char *library : libraries )
		{
			void *handle = dlopen( library, RTLD_LAZY | RTLD_NOLOAD );
			if( handle == nullptr )
				continue;

			void *function = dlsym( handle, name );
			dlclose( handle );

			if( function != nullptr )
				return function;
		}

		return nullptr;
	}

	typedef void ( *Print_t )( const char *format, ... );

	/// Formats once here and hands tier0 the finished line, since tier0
	/// exports no va_list form of its printers.
	void Forward( Print_t &target, const char *name, const char *format, va_list arguments )
	{
		if( target == nullptr )
			target = reinterpret_cast<Print_t>( Tier0( name ) );

		char line[2048];
		vsnprintf( line, sizeof( line ), format, arguments );

		if( target != nullptr )
			target( "%s", line );
		else
			fputs( line, stderr );
	}
}

void Msg( const tchar *format, ... )
{
	static Print_t target = nullptr;

	va_list arguments;
	va_start( arguments, format );
	Forward( target, "Msg", format, arguments );
	va_end( arguments );
}

void Warning( const tchar *format, ... )
{
	static Print_t target = nullptr;

	va_list arguments;
	va_start( arguments, format );
	Forward( target, "Warning", format, arguments );
	va_end( arguments );
}

void Error( const tchar *format, ... )
{
	static Print_t target = nullptr;

	va_list arguments;
	va_start( arguments, format );
	Forward( target, "Error", format, arguments );
	va_end( arguments );

	// tier0's Error does not return, and neither may this when it cannot
	// reach tier0 to do the exiting
	std::abort( );
}

struct tm *Plat_localtime( const time_t *timep, struct tm *result )
{
	typedef struct tm *( *localtime_t )( const time_t *, struct tm * );
	static localtime_t target = reinterpret_cast<localtime_t>( Tier0( "Plat_localtime" ) );

	return target != nullptr ? target( timep, result ) : localtime_r( timep, result );
}
