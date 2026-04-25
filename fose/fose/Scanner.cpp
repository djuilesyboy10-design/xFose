#include "Scanner.h"
#include "SignatureDatabase.h"
#include <windows.h>
#include <cstring>
#include <map>
#include <vector>
#include <algorithm>

namespace Scanner
{
	bool GetSections(SectionInfo& text, SectionInfo& rdata)
	{
		text.base = NULL; text.size = 0;
		rdata.base = NULL; rdata.size = 0;

		HMODULE hExe = GetModuleHandleA(NULL);
		if (!hExe) { _MESSAGE("Scanner: GetModuleHandle failed"); return false; }

		PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hExe;
		PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((UInt8*)hExe + dos->e_lfanew);

		PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(nt);
		for (UInt32 i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++)
		{
			char name[9] = {0};
			memcpy(name, sec->Name, 8);
			if (strcmp(name, ".text") == 0) {
				text.base = (UInt8*)hExe + sec->VirtualAddress;
				text.size = sec->Misc.VirtualSize;
			}
			else if (strcmp(name, ".rdata") == 0) {
				rdata.base = (UInt8*)hExe + sec->VirtualAddress;
				rdata.size = sec->Misc.VirtualSize;
			}
		}

		if (!text.base || !rdata.base) {
			_MESSAGE("Scanner: .text or .rdata section not found");
			return false;
		}
		return true;
	}

	void RunMaskScanner(const MaskCandidate* candidates, size_t count,
		int maxResultsPerMask, UInt32 lookAheadBytes)
	{
		SectionInfo text, rdata;
		if (!GetSections(text, rdata)) return;

		_MESSAGE("MaskScanner: .text=%08X size=%08X  scanning %u masks",
			(UInt32)text.base, text.size, (UInt32)count);

		// Per-target stats: count distinct masks that call it, bitmask of which.
		struct TargetStats { UInt32 maskCount; UInt32 hitCount; UInt32 maskBits; };
		std::map<UInt32, TargetStats> targetStats;

		for (size_t ci = 0; ci < count; ci++)
		{
			UInt32 imm = candidates[ci].mask;
			const char* label = candidates[ci].name ? candidates[ci].name : "?";

			int found = 0;
			for (UInt8* q = text.base; q + 5 < text.base + text.size && found < maxResultsPerMask; q++)
			{
				if (*q != 0x68) continue;
				if (*(UInt32*)(q + 1) != imm) continue;

				UInt32 callTarget = 0;
				UInt32 callSite = 0;
				UInt8* end = q + lookAheadBytes;
				if (end > text.base + text.size) end = text.base + text.size;
				for (UInt8* r = q + 5; r + 5 < end; r++)
				{
					if (*r == 0xE8) {
						UInt32 rel = *(UInt32*)(r + 1);
						callTarget = (UInt32)r + 5 + rel;
						callSite = (UInt32)r;
						break;
					}
				}

				_MESSAGE("MaskScanner: [%s=0x%08X] push at %08X -> CALL at %08X -> target %08X",
					label, imm, (UInt32)q, callSite, callTarget);

				// Track distinct masks per target (filter out obviously bogus targets)
				if (callTarget >= 0x00401000 && callTarget < 0x00E00000)
				{
					TargetStats& ts = targetStats[callTarget];
					if ((ts.maskBits & (UInt32)(1 << ci)) == 0) {
						ts.maskBits |= (UInt32)(1 << ci);
						ts.maskCount++;
					}
					ts.hitCount++;
				}
				found++;
			}

			if (found == 0)
				_MESSAGE("MaskScanner: [%s=0x%08X] no push-imm32 sites found", label, imm);
		}

		// Rank targets by number of distinct masks that reach them.
		std::vector<std::pair<UInt32, TargetStats> > ranked(targetStats.begin(), targetStats.end());
		std::sort(ranked.begin(), ranked.end(),
			[](const std::pair<UInt32, TargetStats>& a, const std::pair<UInt32, TargetStats>& b) {
				if (a.second.maskCount != b.second.maskCount) return a.second.maskCount > b.second.maskCount;
				return a.second.hitCount > b.second.hitCount;
			});

		_MESSAGE("MaskScanner: --- top CALL targets by distinct-mask coverage ---");
		size_t topN = ranked.size() < 8 ? ranked.size() : 8;
		for (size_t i = 0; i < topN; i++)
		{
			UInt32 tgt = ranked[i].first;
			const TargetStats& ts = ranked[i].second;

			// Build mask-name list for this target
			char masks[256] = {0};
			size_t mp = 0;
			for (size_t ci = 0; ci < count && mp < sizeof(masks) - 16; ci++) {
				if (ts.maskBits & (1u << ci)) {
					int n = _snprintf_s(masks + mp, sizeof(masks) - mp, _TRUNCATE,
						"%s%s", mp ? "," : "", candidates[ci].name);
					if (n > 0) mp += n;
				}
			}

			// Dump first 16 prologue bytes so we know detour size later.
			UInt8 prologue[16] = {0};
			memcpy(prologue, (void*)tgt, sizeof(prologue));
			_MESSAGE("MaskScanner: target %08X masks=%u hits=%u [%s]",
				tgt, ts.maskCount, ts.hitCount, masks);
			_MESSAGE("MaskScanner:   prologue: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
				prologue[0], prologue[1], prologue[2], prologue[3],
				prologue[4], prologue[5], prologue[6], prologue[7],
				prologue[8], prologue[9], prologue[10], prologue[11],
				prologue[12], prologue[13], prologue[14], prologue[15]);
		}

		_MESSAGE("MaskScanner: scan complete");
	}

	// Scan for AOB signature pattern in .text section
	UInt32 FindSignature(const UInt8* pattern, UInt32 patternLength)
	{
		SectionInfo text, rdata;
		if (!GetSections(text, rdata)) return 0;

		_MESSAGE("SignatureScanner: scanning for pattern (length=%u)", patternLength);

		for (UInt8* p = text.base; p + patternLength < text.base + text.size; p++)
		{
			bool match = true;
			for (UInt32 i = 0; i < patternLength; i++)
			{
				// Support wildcard bytes (0xCC for now)
				if (pattern[i] != 0xCC && pattern[i] != p[i])
				{
					match = false;
					break;
				}
			}
			if (match)
			{
				_MESSAGE("SignatureScanner: found pattern at %08X", (UInt32)p);
				return (UInt32)p;
			}
		}

		_MESSAGE("SignatureScanner: pattern not found");
		return 0;
	}

	// Find all hook addresses using signature scanning
	void FindHookAddresses(UInt32* addresses, UInt32 count)
	{
		SectionInfo text, rdata;
		if (!GetSections(text, rdata)) return;

		_MESSAGE("SignatureScanner: finding %u hook addresses via AOB scanning", count);

		for (UInt32 i = 0; i < g_numHookSignatures && i < count; i++)
		{
			const HookSignature& sig = g_hookSignatures[i];
			UInt32 foundAddr = FindSignature(sig.pattern, sig.patternLength);

			if (foundAddr)
			{
				addresses[i] = foundAddr;
				_MESSAGE("SignatureScanner: %s found at %08X (expected %08X)",
					sig.name, foundAddr, sig.expectedAddr);

				// Verify address is close to expected (within 1MB range for version differences)
				UInt32 diff = (foundAddr > sig.expectedAddr) ?
					(foundAddr - sig.expectedAddr) : (sig.expectedAddr - foundAddr);
				if (diff > 0x100000)  // 1MB
				{
					_MESSAGE("SignatureScanner: WARNING - address %08X differs significantly from expected %08X",
						foundAddr, sig.expectedAddr);
				}
			}
			else
			{
				addresses[i] = 0;
				_MESSAGE("SignatureScanner: %s NOT FOUND - will use fallback address table",
					sig.name);
			}
		}
	}

	// Detect Fallout 3 version at runtime using executable file size
	// Simple approach: map file size to known FALLOUT_VERSION constants
	// Falls back to compile-time FALLOUT_VERSION if size doesn't match
	UInt32 DetectFalloutVersion()
	{
		// Get path to current executable (Fallout3.exe)
		char exePath[MAX_PATH];
		GetModuleFileNameA(NULL, exePath, MAX_PATH);

		// Get file size
		HANDLE hFile = CreateFileA(exePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			_MESSAGE("VersionDetector: Failed to open executable, using compile-time FALLOUT_VERSION = %08X", FALLOUT_VERSION);
			return FALLOUT_VERSION;
		}

		LARGE_INTEGER fileSize;
		if (!GetFileSizeEx(hFile, &fileSize))
		{
			CloseHandle(hFile);
			_MESSAGE("VersionDetector: Failed to get file size, using compile-time FALLOUT_VERSION = %08X", FALLOUT_VERSION);
			return FALLOUT_VERSION;
		}
		CloseHandle(hFile);

		UInt32 sizeKB = (UInt32)(fileSize.QuadPart / 1024);
		_MESSAGE("VersionDetector: Executable size = %u KB", sizeKB);

		// Map file size to known Fallout 3 versions
		// These sizes are approximate and will need to be verified with actual executables
		UInt32 detectedVersion = FALLOUT_VERSION; // Default fallback

		// Known file sizes (in KB) for different versions
		// These are placeholder values - need to collect actual sizes from different versions
		if (sizeKB >= 14000 && sizeKB <= 15000)
		{
			// Likely 1.7 (GOG/Steam)
			detectedVersion = FALLOUT_VERSION_1_7;
			_MESSAGE("VersionDetector: Detected version 1.7 based on file size %u KB", sizeKB);
		}
		else if (sizeKB >= 13000 && sizeKB <= 14000)
		{
			// Likely 1.6
			detectedVersion = FALLOUT_VERSION_1_6;
			_MESSAGE("VersionDetector: Detected version 1.6 based on file size %u KB", sizeKB);
		}
		else if (sizeKB >= 12000 && sizeKB <= 13000)
		{
			// Likely 1.5
			detectedVersion = FALLOUT_VERSION_1_5_22;
			_MESSAGE("VersionDetector: Detected version 1.5 based on file size %u KB", sizeKB);
		}
		else
		{
			// Unknown size, use compile-time version
			_MESSAGE("VersionDetector: Unknown file size %u KB, using compile-time FALLOUT_VERSION = %08X", sizeKB, FALLOUT_VERSION);
		}

		return detectedVersion;
	}

	// Events NOT seen by the existing MarkEvent hook at 0x005183C0.
	// If we locate the CALL target these push onto the stack, we'll find
	// the other event-marking function(s).
	void RunMissingEventMaskScanner()
	{
		static const MaskCandidate kMissing[] = {
			{ 0x00000001, "OnAdd"        },
			{ 0x00000004, "OnDrop"       },
			{ 0x00000008, "OnUnequip"    },
			{ 0x00000020, "OnMurder"     },
			{ 0x00000040, "OnCombatEnd"  },
			{ 0x00000080, "OnHit"        },
			{ 0x00000100, "OnHitWith"    },
			{ 0x00004000, "OnSell"       },
			{ 0x00008000, "OnStartCombat"},
			{ 0x00010000, "OnOpen"       },
			{ 0x00020000, "OnClose"      },
			{ 0x00080000, "OnGrab"       },
			{ 0x00400000, "OnFire"       },
		};
		RunMaskScanner(kMissing, sizeof(kMissing) / sizeof(kMissing[0]));
	}
}
