#include "replies_cache.hpp"

#include <filesystem.h>

namespace Vanish
{
	namespace Query
	{
		namespace Replies
		{
			const char *DEFAULT_VERSION = "16.12.01";

			std::string ReadGameVersion( IFileSystem *filesystem )
			{
				FileHandle_t file = filesystem->Open( "steam.inf", "rb", "GAME" );
				if( file == nullptr )
					return DEFAULT_VERSION;

				// Read rather than ReadLine: Read lives on IBaseFileSystem, whose
				// layout every build shares with the header, while ReadLine sits
				// past the slots the 32-bit Linux IFileSystem adds
				char buffer[512] = { 0 };
				const int length = filesystem->Read( buffer, sizeof( buffer ) - 1, file );
				filesystem->Close( file );

				if( length <= 0 || length >= static_cast<int>( sizeof( buffer ) ) )
					return DEFAULT_VERSION;

				const std::string contents( buffer, static_cast<size_t>( length ) );
				const std::string key = "PatchVersion=";

				const size_t start = contents.find( key );
				if( start == std::string::npos )
					return DEFAULT_VERSION;

				const size_t value = start + key.size( );
				const size_t end = contents.find_first_of( "\r\n", value );
				const std::string version = contents.substr( value,
					end == std::string::npos ? std::string::npos : end - value );

				return version.empty( ) ? DEFAULT_VERSION : version;
			}
		}
	}
}
