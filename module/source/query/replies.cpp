// Derived from gmsv_spoof by Daniel Almeida (BSD 3-Clause), see
// licenses/gmsv_spoof.txt. The reply layout is the Source A2S format; the
// signature scans, threaded socket and firewall of the original are gone,
// every interface here is resolved through garrysmod_common instead.

#include "replies.hpp"
#include "state.hpp"

#include "../vanish/game_server.hpp"
#include "../main.hpp"

#include <Platform.hpp>
#include <scanning/symbolfinder.hpp>

#include <GarrysMod/InterfacePointers.hpp>

#include <tier0/dbg.h>
#include <bitbuf.h>
#include <eiface.h>
#include <iserver.h>
#include <filesystem.h>
#include <steam/steamclientpublic.h>

#include <cstring>
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

			// the game's VERSION number, which only Lua is given
			static int32_t lua_version = 0;

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

#if defined SYSTEM_LINUX

			// The Linux filesystem has four virtual functions ahead of
			// Gamemodes( ) that the SDK header does not declare, so calling it
			// through the header lands on the wrong slot and returns NULL. The
			// engine's own tag code is read instead, for both the object and
			// the slot it calls:
			//
			//   mov  eax, [g_pFullFileSystem]
			//   mov  edx, [eax]
			//   mov  [esp], eax
			//   call [edx + Gamemodes]
			//   mov  edx, [eax]
			//   mov  [esp], eax
			//   call [edx + 0x10]            Active( )
			static const char GAMEMODES_CALL[] =
				"\xA1\x2A\x2A\x2A\x2A\x8B\x10\x89\x04\x24\xFF\x92\x2A\x2A\x2A\x2A"
				"\x8B\x10\x89\x04\x24\xFF\x52\x10";

			const size_t FILESYSTEM_OPERAND = 1;
			const size_t GAMEMODES_OPERAND = 12;

			///@return the engine's gamemode system, nullptr when not found
			static Gamemode::System *Gamemodes( )
			{
				static const uint8_t *code = nullptr;
				if( code == nullptr )
				{
					SymbolFinder finder;
					code = static_cast<const uint8_t *>( finder.Resolve( global::engine_loader.GetModule( ),
						GAMEMODES_CALL, sizeof( GAMEMODES_CALL ) - 1 ) );
				}

				if( code == nullptr )
					return nullptr;

				IFileSystem **global_filesystem = nullptr;
				int32_t offset = 0;
				std::memcpy( &global_filesystem, code + FILESYSTEM_OPERAND, sizeof( global_filesystem ) );
				std::memcpy( &offset, code + GAMEMODES_OPERAND, sizeof( offset ) );

				IFileSystem *instance = *global_filesystem;
				if( instance == nullptr )
					return nullptr;

				typedef Gamemode::System *( *Gamemodes_t )( IFileSystem * );
				void **vtable = *reinterpret_cast<void ***>( instance );
				return reinterpret_cast<Gamemodes_t>( vtable[offset / sizeof( void * )] )( instance );
			}

#else

			static Gamemode::System *Gamemodes( )
			{
				return filesystem->Gamemodes( );
			}

#endif

			///@return the gamemode name the way the engine tags it
			static std::string TagName( std::string name )
			{
				// the engine strips this suffix before tagging
				const std::string suffix = "_modded";
				for( size_t at = name.find( suffix ); at != std::string::npos; at = name.find( suffix ) )
					name.erase( at, suffix.size( ) );

				return name;
			}

			///@return the tags the engine advertises, in its own order:
			///gm, gmws, gmc, loc and ver
			static std::string ReadGameTags( )
			{
				Gamemode::System *gamemodes = Gamemodes( );
				if( gamemodes == nullptr )
					return std::string( );

				const IGamemodeSystem::Information &gamemode = gamemodes->Active( );

				std::string tags = " gm:";
				tags += TagName( gamemode.name );

				if( gamemode.workshopid != 0 )
				{
					tags += " gmws:";
					tags += std::to_string( gamemode.workshopid );
				}

				if( !gamemode.category.empty( ) )
				{
					tags += " gmc:";
					tags += gamemode.category;
				}

				// the engine shows at most the first five characters, and only
				// once more than one is set
				const std::string &location = Get( ).Settings( ).location;
				if( location.size( ) > 1 )
				{
					tags += " loc:";
					tags += location.substr( 0, 5 );
				}

				if( lua_version != 0 )
				{
					tags += " ver:";
					tags += std::to_string( lua_version );
				}

				return tags;
			}

			///@return PatchVersion from steam.inf, or a safe default
			static std::string ReadGameVersion( )
			{
				FileHandle_t file = filesystem->Open( "steam.inf", "rb", "GAME" );
				if( file == nullptr )
					return DEFAULT_VERSION;

				// Read rather than ReadLine: Read lives on IBaseFileSystem,
				// whose layout the Linux build shares with the header, while
				// ReadLine sits past the slots the Linux IFileSystem adds
				char buffer[512] = { 0 };
				const int length = filesystem->Read( buffer, sizeof( buffer ) - 1, file );
				filesystem->Close( file );

				if( length <= 0 )
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

			bool Initialize( )
			{
				server = GameServer::Get( );
				engine_server = InterfacePointers::VEngineServer( );
				gamedll = InterfacePointers::ServerGameDLL( );
				filesystem = InterfacePointers::FileSystemServer( );

				return server != nullptr && engine_server != nullptr
					&& gamedll != nullptr && filesystem != nullptr;
			}

			void SetVersion( int32_t version )
			{
				lua_version = version;
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
				const int32_t visible = Get( ).Settings( ).visible_max_players;

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
				out.WriteByte( state.VisibleBots( ) );
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
