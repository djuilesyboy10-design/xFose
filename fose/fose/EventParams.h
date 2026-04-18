#pragma once

#include "GameForms.h"

// Event parameter type definitions for FOSE Event Manager
// These define the types of parameters that events can accept

namespace EventManager
{
	// Parameter types for event handlers
	enum ParamType : UInt8
	{
		kParamType_Invalid = 0,
		kParamType_Int,          // Integer value
		kParamType_Float,        // Floating-point value
		kParamType_Form,         // Any form (reference or base form)
		kParamType_Reference,    // Reference (TESObjectREFR)
		kParamType_BaseForm,     // Base form (non-reference)
		kParamType_String,       // String (for future use)
		kParamType_Anything,     // Any type (for user-defined events)
	};

	// Helper functions for parameter type checking
	inline bool IsFormParam(ParamType type)
	{
		return type == kParamType_Form || type == kParamType_Reference || type == kParamType_BaseForm;
	}

	inline bool IsPtrParam(ParamType type)
	{
		return type == kParamType_Reference || type == kParamType_BaseForm;
	}

	inline ParamType GetNonPtrParamType(ParamType type)
	{
		if (IsPtrParam(type))
			return kParamType_Form;
		return type;
	}

	// Common event parameter combinations
	static ParamType kEventParams_GameEvent[2] =
	{
		kParamType_Form, kParamType_Form
	};

	static ParamType kEventParams_OneRef[1] =
	{
		kParamType_Reference,
	};

	static ParamType kEventParams_TwoRefs[2] =
	{
		kParamType_Reference, kParamType_Reference
	};

	static ParamType kEventParams_OneInt[1] =
	{
		kParamType_Int,
	};

	static ParamType kEventParams_TwoInts[2] =
	{
		kParamType_Int, kParamType_Int
	};

	static ParamType kEventParams_OneForm[1] =
	{
		kParamType_Form,
	};

	// OnActivate: (activatorRef, targetRef)
	static ParamType kEventParams_OnActivate[2] =
	{
		kParamType_Reference, kParamType_Reference
	};

	// OnEquip: (actorRef, itemForm)
	static ParamType kEventParams_OnEquip[2] =
	{
		kParamType_Reference, kParamType_Form
	};
}
