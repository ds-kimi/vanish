#include "table.hpp"

#include <GarrysMod/InterfacePointers.hpp>

#include <networkstringtabledefs.h>

#include <map>
#include <vector>

namespace Vanish
{
	namespace UserInfo
	{
		/// original user data of every slot that is currently blanked
		static std::map<int32_t, std::vector<uint8_t>> saved;

		///@return the userinfo string table, nullptr when unavailable
		static INetworkStringTable *Table( )
		{
			static INetworkStringTable *table = nullptr;

			if( table == nullptr )
			{
				INetworkStringTableContainer *container =
					InterfacePointers::NetworkStringTableContainerServer( );

				if( container != nullptr )
					table = container->FindTable( "userinfo" );
			}

			return table;
		}

		bool Hide( int32_t slot )
		{
			INetworkStringTable *table = Table( );
			if( table == nullptr || slot < 0 || slot >= table->GetNumStrings( ) )
				return false;

			if( saved.find( slot ) != saved.end( ) )
				return true;

			int length = 0;
			const void *data = table->GetStringUserData( slot, &length );
			if( data == nullptr || length <= 0 )
				return false;

			const uint8_t *bytes = static_cast<const uint8_t *>( data );
			saved[slot].assign( bytes, bytes + length );

			// the whole entry is zeroed rather than a named field cleared: the
			// layout of player_info_t differs between builds, and a zeroed
			// entry carries no name, no SteamID and no friends ID on any of them
			const std::vector<uint8_t> blank( static_cast<size_t>( length ), 0 );
			table->SetStringUserData( slot, length, blank.data( ) );

			return true;
		}

		bool Show( int32_t slot )
		{
			const auto entry = saved.find( slot );
			if( entry == saved.end( ) )
				return false;

			INetworkStringTable *table = Table( );
			if( table != nullptr )
				table->SetStringUserData( slot,
					static_cast<int>( entry->second.size( ) ), entry->second.data( ) );

			saved.erase( entry );
			return true;
		}

		void Reset( )
		{
			while( !saved.empty( ) )
				Show( saved.begin( )->first );
		}
	}
}
