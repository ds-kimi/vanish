#include "entry.hpp"

#include <GarrysMod/InterfacePointers.hpp>

#include <networkstringtabledefs.h>

namespace Vanish
{
	namespace UserInfo
	{
		namespace Entry
		{
			INetworkStringTable *Table( )
			{
				INetworkStringTableContainer *container =
					InterfacePointers::NetworkStringTableContainerServer( );

				return container != nullptr ? container->FindTable( "userinfo" ) : nullptr;
			}

			const uint8_t *Data( INetworkStringTable *table, int32_t slot, int32_t &length )
			{
				length = 0;
				if( table == nullptr || slot < 0 || slot >= table->GetNumStrings( ) )
					return nullptr;

				int size = 0;
				const void *data = table->GetStringUserData( slot, &size );
				if( data == nullptr || size <= 0 )
					return nullptr;

				length = size;
				return static_cast<const uint8_t *>( data );
			}

			bool IsBlank( const uint8_t *data, int32_t length )
			{
				for( int32_t i = 0; i < length; ++i )
					if( data[i] != 0 )
						return false;

				return true;
			}
		}
	}
}
