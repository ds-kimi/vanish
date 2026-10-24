#include "pe_instance.hpp"
#include "pe_sections.hpp"

#include <Platform.hpp>

#if defined SYSTEM_WINDOWS

#include <cstring>

namespace Vanish
{
	namespace PeInstance
	{
		// _RTTICompleteObjectLocator: signature, offset, cdOffset, then the type
		// descriptor as an absolute pointer on x86 or an image relative offset,
		// with signature 1, on x64
#if defined ARCHITECTURE_X86_64
		const uint32_t LOCATOR_SIGNATURE = 1;
#else
		const uint32_t LOCATOR_SIGNATURE = 0;
#endif

		///@return the locator of the primary vtable for a type descriptor
		static const uint8_t *FindLocator( const std::vector<PeSections::Section> &sections,
			uintptr_t base, uintptr_t descriptor )
		{
			const uint32_t reference = static_cast<uint32_t>(
				LOCATOR_SIGNATURE == 1 ? descriptor - base : descriptor );

			const uint8_t *found = nullptr;
			for( const PeSections::Section &section : sections )
				for( size_t at = 12; !section.writable && at + 4 <= section.size; at += 4 )
				{
					uint32_t fields[4];
					std::memcpy( fields, section.start + at - 12, sizeof( fields ) );
					if( fields[3] != reference || fields[0] != LOCATOR_SIGNATURE || fields[1] != 0 )
						continue;

					if( found != nullptr )
						return nullptr;

					found = section.start + at - 12;
				}

			return found;
		}

		///@return the vtable whose slot -1 points at locator, 0 when not unique
		static uintptr_t FindVTable( const std::vector<PeSections::Section> &sections, const uint8_t *locator )
		{
			uintptr_t vtable = 0;
			for( uintptr_t *slot : PeSections::FindWord( sections, reinterpret_cast<uintptr_t>( locator ), false ) )
			{
				if( vtable != 0 )
					return 0;

				vtable = reinterpret_cast<uintptr_t>( slot + 1 );
			}

			return vtable;
		}

		void *Find( void *module, const char *decorated_name )
		{
			const std::vector<PeSections::Section> sections = PeSections::Of( module );
			const uint8_t *name = PeSections::FindBytes( sections, decorated_name, std::strlen( decorated_name ) + 1 );
			if( name == nullptr )
				return nullptr;

			// TypeDescriptor is { type_info vtable, spare, name }
			const uintptr_t descriptor = reinterpret_cast<uintptr_t>( name ) - 2 * sizeof( void * );
			const uint8_t *locator = FindLocator( sections, reinterpret_cast<uintptr_t>( module ), descriptor );
			const uintptr_t vtable = locator != nullptr ? FindVTable( sections, locator ) : 0;
			if( vtable == 0 )
				return nullptr;

			const std::vector<uintptr_t *> objects = PeSections::FindWord( sections, vtable, true );
			return objects.size( ) == 1 ? objects[0] : nullptr;
		}
	}
}

#endif
