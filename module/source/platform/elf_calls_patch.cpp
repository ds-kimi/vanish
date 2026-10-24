#include "elf_calls.hpp"
#include "code_patch.hpp"

#include <Platform.hpp>

#if defined SYSTEM_LINUX && defined ARCHITECTURE_X86

namespace Vanish
{
	namespace ElfCalls
	{
		///@return where a call rel32 whose operand is at operand lands
		static uintptr_t CallTarget( const int32_t *operand )
		{
			return reinterpret_cast<uintptr_t>( operand + 1 ) + *operand;
		}

		void *Redirect( void *handle, const char *symbol, void *replacement, std::vector<CallSite> &patched )
		{
			const std::vector<int32_t *> operands = FindOperands( handle, symbol );
			if( operands.empty( ) )
				return nullptr;

			// every site was relocated against the same import, so a site that
			// disagrees means this is not the code the relocations describe
			const uintptr_t original = CallTarget( operands[0] );
			for( int32_t *operand : operands )
				if( CallTarget( operand ) != original )
					return nullptr;

			for( int32_t *operand : operands )
			{
				const int32_t value = static_cast<int32_t>(
					reinterpret_cast<uintptr_t>( replacement ) -
					reinterpret_cast<uintptr_t>( operand + 1 ) );

				const CallSite site = { operand, *operand };
				if( !CodePatch::Write( operand, &value, sizeof( value ) ) )
				{
					Restore( patched );
					return nullptr;
				}

				patched.push_back( site );
			}

			return reinterpret_cast<void *>( original );
		}

		void Restore( std::vector<CallSite> &patched )
		{
			for( const CallSite &site : patched )
				CodePatch::Write( site.operand, &site.original, sizeof( site.original ) );

			patched.clear( );
		}
	}
}

#endif
