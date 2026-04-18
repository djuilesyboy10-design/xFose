#pragma once

#include "PluginAPI.h"

// Modernization: Optional helper functions for Plugin API
// This header provides convenience wrappers and type-safe helpers for the FOSE Plugin API.
// Include this header in your plugin to use these helpers - they are opt-in only.
//
// These helpers do not modify the core PluginAPI.h and are designed to improve
// developer ergonomics without affecting binary compatibility or performance.

namespace fose
{
	// Helper functions for version checking
	inline bool IsFOSEVersionCompatible(const FOSEInterface* fose, UInt32 requiredMajor, UInt32 requiredMinor = 0)
	{
		if (!fose) return false;
		UInt32 foseVersion = fose->foseVersion;
		UInt32 major = (foseVersion >> 16) & 0xFFFF;
		UInt32 minor = foseVersion & 0xFFFF;
		return (major > requiredMajor) || (major == requiredMajor && minor >= requiredMinor);
	}

	inline bool IsRuntimeVersionCompatible(const FOSEInterface* fose, UInt32 requiredVersion)
	{
		if (!fose) return false;
		return fose->runtimeVersion >= requiredVersion;
	}

	// Type-safe interface query helper
	template <typename InterfaceType>
	inline InterfaceType* QueryInterface(const FOSEInterface* fose, UInt32 interfaceID)
	{
		if (!fose) return nullptr;
		return static_cast<InterfaceType*>(fose->QueryInterface(interfaceID));
	}

	// Convenience wrappers for getting common interfaces
	inline FOSEConsoleInterface* GetConsoleInterface(const FOSEInterface* fose)
	{
		return QueryInterface<FOSEConsoleInterface>(fose, kInterface_Console);
	}

	inline FOSESerializationInterface* GetSerializationInterface(const FOSEInterface* fose)
	{
		return QueryInterface<FOSESerializationInterface>(fose, kInterface_Serialization);
	}

	inline FOSEMessagingInterface* GetMessagingInterface(const FOSEInterface* fose)
	{
		return QueryInterface<FOSEMessagingInterface>(fose, kInterface_Messaging);
	}

	inline FOSEStringVarInterface* GetStringVarInterface(const FOSEInterface* fose)
	{
		return QueryInterface<FOSEStringVarInterface>(fose, kInterface_StringVar);
	}

	inline FOSEIOInterface* GetIOInterface(const FOSEInterface* fose)
	{
		return QueryInterface<FOSEIOInterface>(fose, kInterface_IO);
	}

	inline FOSECommandTableInterface* GetCommandTableInterface(const FOSEInterface* fose)
	{
		return QueryInterface<FOSECommandTableInterface>(fose, kInterface_CommandTable);
	}
}
