#pragma once

#include <GarrysMod/Lua/Interface.h>

#include <cstdint>
#include <string>

namespace Vanish
{
	/// Reads Lua arguments into C++ values without undefined behaviour.
	///
	/// Converting a double outside an integer's range, or NaN, to that integer
	/// is undefined, and Lua hands over any double it likes: nan, inf, 1e300.
	namespace LuaArgs
	{
		///@return value clamped into [low, high], low for NaN
		int32_t ClampInteger( double value, int32_t low, int32_t high );

		///@return value clamped into [low, high], low for NaN
		float ClampFloat( double value, float low, float high );

		///@return table[key] read with rawget, so a metatable on the table can
		///neither raise an error nor run code; fallback when not a number
		double RawNumber( GarrysMod::Lua::ILuaBase *LUA, int table, const char *key, double fallback );

		/// Reads table[key] with rawget into out when it is a string.
		///@return whether out was written
		bool RawString( GarrysMod::Lua::ILuaBase *LUA, int table, const char *key, std::string &out );
	}
}
