#pragma once

#include <cstddef>
#include <cstdint>

namespace Vanish
{
	namespace DeltaHeader
	{
		/// Where the leave-PVS branch of the Linux engine's
		/// CBaseServer::WriteDeltaEntities is patched, per architecture.
		///
		/// SV_WriteDeltaHeader is inlined there, so the hook is spliced in at
		/// the point where both leave paths have settled the flags: the patched
		/// bytes move into a stub that calls Promote and writes the result back
		/// into the flags register.
		struct Site
		{
			const char *signature;
			size_t signature_length;

			// where the patched instructions start within the match, and how
			// many bytes of whole instructions are moved into the stub
			size_t offset;
			size_t length;

			// position of the m_pBuf frame displacement within the moved bytes,
			// which locates CEntityWriteInfo in the engine's frame
			size_t buffer_operand;
		};

		const Site &PatchSite( );

		/// Builds the stub the patched site jumps to.
		///@param stub writable block of at least STUB_SIZE bytes
		///@param site address of the patched instructions in the engine
		///@param original the instructions the patch displaces
		///@param info_displacement CEntityWriteInfo's offset from the frame pointer
		void BuildStub( uint8_t *stub, const uint8_t *site, const uint8_t *original, int32_t info_displacement );

		/// Builds the jump written over the site, padded to the site's length.
		void BuildPatch( uint8_t *patch, const uint8_t *site, const uint8_t *stub );

		const size_t STUB_SIZE = 256;
		const size_t MAX_SITE_LENGTH = 32;
	}
}
