#pragma once

#include "GameForms.h"
#include "GameAPI.h"

// Modernization: Optional helper functions for Game Data Structures
// This header provides convenience wrappers and type-safe helpers for game data structures.
// Include this header in your plugin to use these helpers - they are opt-in only.
//
// These helpers do not modify the core game data structures and are designed to improve
// developer ergonomics without affecting binary compatibility or performance.

namespace fose
{
	// Helper functions for form type checking
	inline bool IsFormType(TESForm* form, FormType type)
	{
		return form && form->typeID == type;
	}

	inline bool IsActor(TESForm* form)
	{
		return IsFormType(form, kFormType_NPC) || IsFormType(form, kFormType_Creature);
	}

	inline bool IsItem(TESForm* form)
	{
		if (!form) return false;
		FormType type = static_cast<FormType>(form->typeID);
		return type == kFormType_Weapon || type == kFormType_Armor || 
		       type == kFormType_Ammo || type == kFormType_Misc ||
		       type == kFormType_AlchemyItem || type == kFormType_Ingredient ||
		       type == kFormType_Book || type == kFormType_Key;
	}

	// Type-safe form casting helpers
	template <typename FormType>
	inline FormType* DynamicCastForm(TESForm* form)
	{
		if (!form) return nullptr;
		return (form->typeID == FormType::kTypeID) ? static_cast<FormType*>(form) : nullptr;
	}

	// Form lookup helpers
	inline TESForm* LookupFormByID(UInt32 formID)
	{
		return LookupFormByID(formID);
	}

	inline TESForm* LookupFormByEditorID(const char* editorID)
	{
		return (*g_dataHandler)->LookupFormByName(editorID);
	}

	// Form flag helpers
	inline bool IsQuestItem(TESForm* form)
	{
		return form && (form->flags & TESForm::kFormFlags_QuestItem);
	}

	inline bool IsInitialized(TESForm* form)
	{
		return form && (form->flags & TESForm::kFormFlags_Initialized);
	}

	// Form name helpers
	inline const char* GetFormName(TESForm* form)
	{
		if (!form) return nullptr;
		TESFullName* fullName = form->GetFullName();
		return fullName ? fullName->name.c_str() : nullptr;
	}

	inline std::string GetFormNameSafe(TESForm* form)
	{
		const char* name = GetFormName(form);
		return name ? std::string(name) : std::string();
	}

	// Reference helpers
	inline TESObjectREFR* GetReference(TESForm* form)
	{
		return IsFormType(form, kFormType_Reference) ? static_cast<TESObjectREFR*>(form) : nullptr;
	}

	// Cell helpers
	inline TESObjectCELL* GetContainingCell(TESObjectREFR* refr)
	{
		return refr ? refr->parentCell : nullptr;
	}

	// Base form helpers
	inline TESForm* GetBaseForm(TESObjectREFR* refr)
	{
		return refr ? refr->baseForm : nullptr;
	}
}
