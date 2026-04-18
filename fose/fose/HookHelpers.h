#pragma once

#include "Utilities.h"

// Modernization: Optional helper functions for Hook System
// This header provides convenience wrappers and type-safe helpers for hook operations.
// Include this header in your plugin to use these helpers - they are opt-in only.
//
// These helpers do not modify the core hook system and are designed to improve
// developer ergonomics without affecting binary compatibility or performance.

namespace fose
{
	// RAII-style temporary hook management
	// Automatically restores the original bytes when the object goes out of scope
	class TempHookGuard
	{
	public:
		TempHookGuard(UInt32 hookAddr, UInt32 jmpAddr)
			: m_hookAddr(hookAddr), m_installed(false)
		{
			CreateTempHook(hookAddr, jmpAddr);
			m_installed = true;
		}

		~TempHookGuard()
		{
			if (m_installed)
			{
				DeleteTempHook(m_hookAddr);
			}
		}

		// Disable copy
		TempHookGuard(const TempHookGuard&) = delete;
		TempHookGuard& operator=(const TempHookGuard&) = delete;

		// Enable move
		TempHookGuard(TempHookGuard&& other) noexcept
			: m_hookAddr(other.m_hookAddr), m_installed(other.m_installed)
		{
			other.m_installed = false;
		}

		TempHookGuard& operator=(TempHookGuard&& other) noexcept
		{
			if (this != &other)
			{
				if (m_installed)
				{
					DeleteTempHook(m_hookAddr);
				}
				m_hookAddr = other.m_hookAddr;
				m_installed = other.m_installed;
				other.m_installed = false;
			}
			return *this;
		}

		bool is_installed() const { return m_installed; }

	private:
		UInt32 m_hookAddr;
		bool m_installed;
	};

	// Helper for safe hook installation with error checking
	inline bool InstallHookSafe(UInt32 hookAddr, UInt32 jmpAddr)
	{
		if (!hookAddr || !jmpAddr)
			return false;

		CreateTempHook(hookAddr, jmpAddr);
		return true;
	}

	// Helper for safe hook removal with error checking
	inline bool RemoveHookSafe(UInt32 hookAddr)
	{
		if (!hookAddr)
			return false;

		return DeleteTempHook(hookAddr);
	}

	// Scope-based hook guard factory function
	inline TempHookGuard MakeTempHook(UInt32 hookAddr, UInt32 jmpAddr)
	{
		return TempHookGuard(hookAddr, jmpAddr);
	}
}
