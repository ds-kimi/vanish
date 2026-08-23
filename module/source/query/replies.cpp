// Derived from gmsv_spoof by Daniel Almeida (BSD 3-Clause), see
// licenses/gmsv_spoof.txt. The reply layout is the Source A2S format; the
// signature scans, threaded socket and firewall of the original are gone,
// every interface here is resolved through garrysmod_common instead.

#include "replies.hpp"
#include "state.hpp"

#include <GarrysMod/InterfacePointers.hpp>

#include <tier0/dbg.h>
#include <bitbuf.h>
#include <eiface.h>
#include <iserver.h>
#include <filesystem.h>
#include <icvar.h>
#include <convar.h>
#include <steam/steamclientpublic.h>

#include <string>

namespace Vanish
{
	namespace Query
	{
		namespace Replies
		{
			// protocol constants of the A2S reply, unchanged from the engine
			const uint8_t PROTOCOL_VERSION = 17;
			const char SERVER_TYPE_DEDICATED = 'd';
			const char *DEFAULT_VERSION = "16.12.01";

#if defined SYSTEM_WINDOWS
			const char OPERATING_SYSTEM = 'w';
#elif defined SYSTEM_LINUX
			const char OPERATING_SYSTEM = 'l';
#else
			const char OPERATING_SYSTEM = 'm';
#endif

			static IServer *server = nullptr;
			static IVEngineServer *engine_server = nullptr;
			static IServerGameDLL *gamedll = nullptr;
			static IFileSystem *filesystem = nullptr;
			static ConVar *visible_max_players = nullptr;

			// read once per Refresh, since none of it changes between queries
			static std::string game_description;
			static std::string game_directory;
			static std::string game_version;
			static std::string game_tags;
			static int32_t max_clients = 0;
			static int32_t udp_port = 0;

			///@return the game directory name without its path
			static std::string ReadGameDirectory( )
			{
				char path[256] = { 0 };
				engine_server->GetGameDir( path, sizeof( path ) );

				std::string directory( path );
				const size_t slash = directory.find_last_of( "\\/" );
				if( slash != std::string::npos )
					directory.erase( 0, slash + 1 );

				return directory;
			}

			///@return the gamemode tags the server browser filters on
			static std::string ReadGameTags( )
			{
				const IGamemodeSystem::Information &gamemode =
					filesystem->Gamemodes( )->Active( );

				std::string tags = " gm:";
				tags += gamemode.name;

				if( gamemode.workshopid != 0 )
				{
					tags += " gmws:";
					tags += std::to_string( gamemode.workshopid );
				}

				return tags;
			}

			///@return PatchVersion from steam.inf, or a safe default
			static std::string ReadGameVersion( )
			{
				FileHandle_t file = filesystem->Open( "steam.inf", "r", "GAME" );
				if( file == nullptr )
					return DEFAULT_VERSION;

				char line[256] = { 0 };
				const bool failed = filesystem->ReadLine( line, sizeof( line ), file ) == nullptr;
				filesystem->Close( file );

				// the first line is PatchVersion=x.y.z, so the value starts
				// after the key and its separator
				if( failed || strlen( line ) <= 13 )
					return DEFAULT_VERSION;

				std::string version( &line[13] );

				const size_t end = version.find_first_of( "\r\n" );
				if( end != std::string::npos )
					version.erase( end );

				return version;
			}

			bool Initialize( )
			{
				server = InterfacePointers::Server( );
				engine_server = InterfacePointers::VEngineServer( );
				gamedll = InterfacePointers::ServerGameDLL( );
				filesystem = InterfacePointers::FileSystemServer( );

				ICvar *cvar = InterfacePointers::Cvar( );
				if( cvar != nullptr )
					visible_max_players = cvar->FindVar( "sv_visiblemaxplayers" );

				return server != nullptr && engine_server != nullptr
					&& gamedll != nullptr && filesystem != nullptr;
			}

			void Refresh( )
			{
				if( server == nullptr )
					return;

				game_description = gamedll->GetGameDescription( );
				game_directory = ReadGameDirectory( );
				game_tags = ReadGameTags( );
				game_version = ReadGameVersion( );
				max_clients = server->GetMaxClients( );
				udp_port = server->GetUDPPort( );
			}

			///@return slot count to advertise, honouring sv_visiblemaxplayers
			static int32_t VisibleMaxPlayers( )
			{
				const int32_t visible =
					visible_max_players != nullptr ? visible_max_players->GetInt( ) : -1;

				return visible <= 0 || visible > max_clients ? max_clients : visible;
			}

			void BuildInfo( bf_write &out )
			{
				const State &state = Get( );
				const int32_t appid = engine_server->GetAppID( );

				out.WriteLong( -1 );
				out.WriteByte( 'I' );
				out.WriteByte( PROTOCOL_VERSION );
				out.WriteString( server->GetName( ) );
				out.WriteString( server->GetMapName( ) );
				out.WriteString( game_directory.c_str( ) );
				out.WriteString( game_description.c_str( ) );
				out.WriteShort( appid );
				out.WriteByte( state.VisibleCount( ) );
				out.WriteByte( VisibleMaxPlayers( ) );
				out.WriteByte( server->GetNumFakeClients( ) );
				out.WriteByte( SERVER_TYPE_DEDICATED );
				out.WriteByte( OPERATING_SYSTEM );
				out.WriteByte( server->GetPassword( ) != nullptr ? 1 : 0 );
				out.WriteByte( state.IsSecure( ) ? 1 : 0 );
				out.WriteString( game_version.c_str( ) );

				const CSteamID *steamid = engine_server->GetGameServerSteamID( );
				const bool tagged = !game_tags.empty( );

				// 0x80 port, 0x10 server steamid, 0x20 tags, 0x01 long appid
				out.WriteByte( 0x80 | 0x10 | ( tagged ? 0x20 : 0x00 ) | 0x01 );
				out.WriteShort( udp_port );
				out.WriteLongLong( steamid != nullptr ? steamid->ConvertToUint64( ) : 0 );

				if( tagged )
					out.WriteString( game_tags.c_str( ) );

				out.WriteLongLong( appid );
			}

			void BuildPlayers( bf_write &out )
			{
				const std::vector<PlayerEntry> &players = Get( ).Players( );

				out.WriteLong( -1 );
				out.WriteByte( 'D' );
				out.WriteByte( static_cast<int>( players.size( ) ) );

				for( size_t i = 0; i < players.size( ); ++i )
				{
					out.WriteByte( static_cast<int>( i ) );
					out.WriteString( players[i].name.c_str( ) );
					out.WriteLong( players[i].score );
					out.WriteFloat( players[i].time );
				}
			}
		}
	}
}
