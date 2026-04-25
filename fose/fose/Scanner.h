#pragma once

// =========================================================================
// Scanner - Universal memory scanning utilities for FOSE
// =========================================================================
// A collection of reusable scanners for locating game functions by
// scanning .text / .rdata sections at runtime. Version-agnostic.
//
// Scanners currently provided:
//   - MaskScanner  : find push <imm32> + CALL <target> patterns to locate
//                    event-marking functions (ScriptEventList::MarkEvent and
//                    friends). Used to discover additional MarkEvent variants.
//   - ErrorScanner : (TODO migrate from Hooks_SaveLoad.cpp) find error
//                    format strings in .rdata and trace their call sites in
//                    .text to locate RuntimeScriptError-style handlers.
// =========================================================================

#include "Utilities.h"

namespace Scanner
{
	// Section descriptors resolved at runtime from the PE image.
	struct SectionInfo
	{
		UInt8*	base;
		UInt32	size;
	};

	// Populate .text / .rdata section pointers from the main executable.
	// Returns true on success. Logs to fose log on failure.
	bool GetSections(SectionInfo& text, SectionInfo& rdata);

	// ---------------------------------------------------------------
	// MaskScanner
	// ---------------------------------------------------------------
	// For each given imm32 value, scan .text for `push <imm32>` (0x68 XX XX XX XX)
	// and record the next CALL target within `lookAheadBytes` bytes.
	// Logs results to fose log. Used to find event-marking functions by
	// the event mask value pushed as an argument.
	struct MaskCandidate
	{
		UInt32		mask;			// imm32 value to search for
		const char*	name;			// label used in log output
	};

	void RunMaskScanner(const MaskCandidate* candidates, size_t count,
		int maxResultsPerMask = 8, UInt32 lookAheadBytes = 64);

	// Convenience: scan for the 13 unknown ScriptEventList masks that aren't
	// dispatched through the currently-hooked MarkEvent at 0x005183C0.
	void RunMissingEventMaskScanner();
}
