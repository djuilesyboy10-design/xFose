#ifndef __SIGNATUREDATABASE_H
#define __SIGNATUREDATABASE_H

#include "fose_common\fose_version.h"

// Signature database for dynamic version detection
// Contains AOB signatures for all hook targets to enable single FOSE build across versions

struct HookSignature
{
	const char* name;           // Hook name for logging
	const UInt8* pattern;       // AOB byte pattern (XX = wildcard)
	UInt32 patternLength;       // Length of pattern in bytes
	UInt32 expectedAddr;        // Expected address for current version (1.7)
	UInt32 prologueLength;      // Length of prologue to overwrite
	const char* description;    // Description of what the hook does
};

// AOB signature patterns
// XX = wildcard byte (matches any value)
static const UInt8 SIG_MarkEvent[] = { 0x57, 0x8B, 0x79, 0x08, 0x32, 0xC0 };           // push edi / mov edi,[ecx+8] / xor al,al
static const UInt8 SIG_MarkEvent2[] = { 0x8B, 0x41, 0x08, 0x85, 0xC0 };                // mov eax,[ecx+8]; test eax,eax
static const UInt8 SIG_MarkEvent3[] = { 0x53, 0x56, 0x8B, 0x74, 0x24, 0x10 };        // push ebx; push esi; mov esi,[esp+10h]
static const UInt8 SIG_OnDrop[] = { 0x56, 0x8B, 0x74, 0x24, 0x08, 0x85, 0xF6 };       // push esi; mov esi,[esp+8]; test esi,esi
static const UInt8 SIG_OnDropAlt[] = { 0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08 };          // push ebp; mov ebp,esp; sub esp,8
static const UInt8 SIG_OnHit[] = { 0x8B, 0x44, 0x24, 0x04, 0x53, 0x33, 0xDB };        // mov eax,[esp+4]; push ebx; xor ebx,ebx
static const UInt8 SIG_MultiEvent[] = { 0x83, 0xEC, 0x08 };                             // sub esp,8
static const UInt8 SIG_Activate[] = { 0x81, 0xEC, 0x14, 0x01, 0x00, 0x00 };           // sub esp,114h
static const UInt8 SIG_EquipItem[] = { 0x6A, 0xFF, 0x68, 0x28, 0x72, 0xC3, 0x00 };    // push -1; push offset; (first 7 bytes of 14-byte prologue)

// Hook signature database
static const HookSignature g_hookSignatures[] = {
	{
		"MarkEvent",
		SIG_MarkEvent,
		sizeof(SIG_MarkEvent),
		0x005183C0,  // Current version address
		6,            // Prologue length
		"ScriptEventList::MarkEvent - standard event dispatch"
	},
	{
		"MarkEvent2",
		SIG_MarkEvent2,
		sizeof(SIG_MarkEvent2),
		0x00518430,
		5,
		"Additional ScriptEventList events (OnSell, OnStartCombat, OnOpen, OnClose, OnGrab)"
	},
	{
		"MarkEvent3",
		SIG_MarkEvent3,
		sizeof(SIG_MarkEvent3),
		0x00518DF0,
		6,
		"Additional ScriptEventList events (DISABLED - causes crashes)"
	},
	{
		"OnDrop",
		SIG_OnDrop,
		sizeof(SIG_OnDrop),
		0x00826590,
		6,
		"OnDrop event handler (DISABLED - causes crashes)"
	},
	{
		"OnDropAlt",
		SIG_OnDropAlt,
		sizeof(SIG_OnDropAlt),
		0x008FE580,
		5,
		"OnDrop alternate handler (DISABLED - causes crashes)"
	},
	{
		"OnHit",
		SIG_OnHit,
		sizeof(SIG_OnHit),
		0x0040A4C0,
		6,
		"OnHit event handler"
	},
	{
		"MultiEvent",
		SIG_MultiEvent,
		sizeof(SIG_MultiEvent),
		0x0086B930,
		5,
		"Multi-event handler (OnHit, OnSell, OnGrab) - DISABLED"
	},
	{
		"Activate",
		SIG_Activate,
		sizeof(SIG_Activate),
		0x004EE000,
		6,
		"Activate function hook"
	},
	{
		"EquipItem",
		SIG_EquipItem,
		sizeof(SIG_EquipItem),
		0x0053CF40,
		14,
		"EquipItem function hook"
	}
};

static const UInt32 g_numHookSignatures = sizeof(g_hookSignatures) / sizeof(HookSignature);

// Version-specific address tables (fallback if signature scanning fails)
struct VersionAddressTable
{
	UInt32 version;           // FOSE version constant
	const char* versionName;  // Human-readable version name
	UInt32 markEventAddr;     // MarkEvent address
	UInt32 markEvent2Addr;    // MarkEvent2 address
	UInt32 activateAddr;      // Activate address
	UInt32 equipItemAddr;     // EquipItem address
	// Add more addresses as needed
};

static const VersionAddressTable g_versionTables[] = {
	{ FALLOUT_VERSION_1_0_15, "1.0.15", 0x005183C0, 0x00518430, 0x004EE000, 0x0053CF40 },
	{ FALLOUT_VERSION_1_1_35, "1.1.35", 0x005183C0, 0x00518430, 0x004EE000, 0x0053CF40 },
	{ FALLOUT_VERSION_1_4_6,  "1.4.6",  0x005183C0, 0x00518430, 0x004EE000, 0x0053CF40 },
	{ FALLOUT_VERSION_1_4_6b, "1.4.6b", 0x005183C0, 0x00518430, 0x004EE000, 0x0053CF40 },
	{ FALLOUT_VERSION_1_5_22, "1.5.22", 0x005183C0, 0x00518430, 0x004EE000, 0x0053CF40 },
	{ FALLOUT_VERSION_1_6,    "1.6",    0x005183C0, 0x00518430, 0x004EE000, 0x0053CF40 },
	{ FALLOUT_VERSION_1_7,    "1.7",    0x005183C0, 0x00518430, 0x004EE000, 0x0053CF40 },
	{ FALLOUT_VERSION_1_7ng,  "1.7ng",  0x005183C0, 0x00518430, 0x004EE000, 0x0053CF40 },
	// GOG, Steam, Anniversary versions to be added after research
};

static const UInt32 g_numVersionTables = sizeof(g_versionTables) / sizeof(VersionAddressTable);

// Get hook addresses from version table based on detected version
// Returns true if version found in table, false otherwise
inline bool GetVersionAddresses(UInt32 detectedVersion, VersionAddressTable& outTable)
{
	for (UInt32 i = 0; i < g_numVersionTables; i++)
	{
		if (g_versionTables[i].version == detectedVersion)
		{
			outTable = g_versionTables[i];
			return true;
		}
	}
	return false;
}

#endif
