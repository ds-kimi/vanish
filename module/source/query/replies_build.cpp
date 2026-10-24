#include "replies.hpp"
#include "replies_cache.hpp"
#include "state.hpp"

#include <Platform.hpp>

#include <bitbuf.h>
#include <eiface.h>
#include <iserver.h>
#include <steam/steamclientpublic.h>

namespace Vanish
{
	namespace Query
	{
		namespace Replies
		{
			// protocol constants of the A2S reply, unchanged from the engine
			const uint8_t PROTOCOL_VERSION = 17;
			const char SERVER_TYPE_DEDICATED = 'd';

#if defined SYSTEM_WINDOWS
			const char OPERATING_SYSTEM = 'w';
#elif defined SYSTEM_LINUX
			const char OPERATING_SYSTEM = 'l';
#else
			const char OPERATING_SYSTEM = 'm';
#endif

			///@return slot count to advertise, honouring sv_visiblemaxplayers
			static int32_t VisibleMaxPlayers( )
			{
				const int32_t visible = Get( ).settings.visible_max_players;
				const int32_t max_clients = Data( ).max_clients;

				return visible <= 0 || visible > max_clients ? max_clients : visible;
			}

			/// The extra data block: port, server SteamID, tags and long appid.
			static void WriteExtraData( bf_write &out, const Interfaces &engine, int32_t appid )
			{
				const CSteamID *steamid = engine.engine->GetGameServerSteamID( );
				const std::string &tags = Data( ).tags;

				// 0x80 port, 0x10 server steamid, 0x20 tags, 0x01 long appid
				out.WriteByte( 0x80 | 0x10 | ( tags.empty( ) ? 0x00 : 0x20 ) | 0x01 );
				out.WriteShort( engine.server->GetUDPPort( ) );
				out.WriteLongLong( steamid != nullptr ? steamid->ConvertToUint64( ) : 0 );

				if( !tags.empty( ) )
					out.WriteString( tags.c_str( ) );

				out.WriteLongLong( appid );
			}

			void BuildInfo( bf_write &out )
			{
				const Interfaces &engine = Engine( );
				const State &state = Get( );
				const Cache &cache = Data( );
				const int32_t appid = engine.engine->GetAppID( );
				const char *name = engine.server->GetName( );
				const char *map = engine.server->GetMapName( );

				// read live: the gamemode sets it after the module has loaded
				const char *description = engine.gamedll->GetGameDescription( );

				out.WriteLong( -1 );
				out.WriteByte( 'I' );
				out.WriteByte( PROTOCOL_VERSION );
				out.WriteString( name != nullptr ? name : "" );
				out.WriteString( map != nullptr ? map : "" );
				out.WriteString( cache.directory.c_str( ) );
				out.WriteString( description != nullptr ? description : "" );
				out.WriteShort( appid );
				out.WriteByte( state.visible_count );
				out.WriteByte( VisibleMaxPlayers( ) );
				out.WriteByte( state.visible_bots );
				out.WriteByte( SERVER_TYPE_DEDICATED );
				out.WriteByte( OPERATING_SYSTEM );
				out.WriteByte( engine.server->GetPassword( ) != nullptr ? 1 : 0 );
				out.WriteByte( state.secure ? 1 : 0 );
				out.WriteString( cache.version.c_str( ) );
				WriteExtraData( out, engine, appid );
			}

			void BuildPlayers( bf_write &out )
			{
				const std::vector<PlayerEntry> &players = Get( ).players;

				out.WriteLong( -1 );
				out.WriteByte( 'D' );
				out.WriteByte( static_cast<int>( players.size( ) ) );

				// the engine writes 0 as every row's index, which query tools
				// ignore, so a counting index would set these replies apart
				for( const PlayerEntry &player : players )
				{
					out.WriteByte( 0 );
					out.WriteString( player.name.c_str( ) );
					out.WriteLong( player.score );
					out.WriteFloat( player.time );
				}
			}
		}
	}
}
