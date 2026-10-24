#include "table.hpp"
#include "entry.hpp"

#include <networkstringtabledefs.h>

#include <map>
#include <vector>

namespace Vanish
{
	namespace UserInfo
	{
		/// original user data of every slot that is currently blanked
		static std::map<int32_t, std::vector<uint8_t>> saved;

		bool Hide( int32_t slot )
		{
			INetworkStringTable *table = Entry::Table( );
			int32_t length = 0;
			const uint8_t *data = Entry::Data( table, slot, length );
			if( data == nullptr )
				return false;

			// already blank: nothing new to save. Otherwise the engine wrote the
			// entry since it was blanked, after a name or setting change, and
			// that newer copy is what must come back on Show
			if( saved.find( slot ) != saved.end( ) && Entry::IsBlank( data, length ) )
				return true;

			saved[slot].assign( data, data + length );

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

			// only an entry still blank is put back: a filled one was rewritten by
			// the engine, for a new player in the slot or a changed setting, and
			// the saved copy would overwrite it with someone else's identity
			INetworkStringTable *table = Entry::Table( );
			int32_t length = 0;
			const uint8_t *data = Entry::Data( table, slot, length );
			if( data != nullptr && Entry::IsBlank( data, length ) )
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
