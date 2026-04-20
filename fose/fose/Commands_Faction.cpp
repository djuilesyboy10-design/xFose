#include "Commands_Faction.h"
#include "GameForms.h"
#include "GameObjects.h"

bool Cmd_GetFactionRank_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESFaction* faction = NULL;
	TESObjectREFR* refr = NULL;
	
	if (ExtractArgs(EXTRACT_ARGS, &faction, &refr))
	{
		if (!refr) refr = thisObj;
		if (refr && faction)
		{
			TESActorBase* actorBase = (TESActorBase*)refr->baseForm;
			if (actorBase && actorBase->typeID == kFormType_NPC)
			{
				SInt8 rank = actorBase->baseData.GetFactionRank(faction);
				*result = rank;
				if (IsConsoleMode())
					Console_Print("%s rank in %s: %d", GetFullName(refr), GetFullName(faction), rank);
			}
		}
	}

	return true;
}

bool Cmd_SetFactionRank_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESFaction* faction = NULL;
	SInt32 rank = 0;
	TESObjectREFR* refr = NULL;
	
	if (ExtractArgs(EXTRACT_ARGS, &faction, &rank, &refr))
	{
		if (!refr) refr = thisObj;
		if (refr && faction)
		{
			TESActorBase* actorBase = (TESActorBase*)refr->baseForm;
			if (actorBase && actorBase->typeID == kFormType_NPC)
			{
				// Find existing faction entry
				bool found = false;
				SInt32 index = 0;
				tList<TESActorBaseData::FactionListData>::Iterator it;
				for (it = actorBase->baseData.factionList.Begin(); !it.End(); ++it, ++index)
				{
					TESActorBaseData::FactionListData* data = it.Get();
					if (data && data->faction == faction)
					{
						data->rank = rank;
						found = true;
						break;
					}
				}
				
				// If not found, add new entry
				if (!found)
				{
					TESActorBaseData::FactionListData* data = new TESActorBaseData::FactionListData();
					data->faction = faction;
					data->rank = rank;
					actorBase->baseData.factionList.push_back(data);
				}
				
				actorBase->MarkAsModified(0x00000001); // Mark as modified
				*result = 1;
				
				if (IsConsoleMode())
					Console_Print("Set %s rank in %s to %d", GetFullName(refr), GetFullName(faction), rank);
			}
		}
	}

	return true;
}

bool Cmd_GetFactionReaction_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESFaction* factionA = NULL;
	TESFaction* factionB = NULL;
	
	if (ExtractArgs(EXTRACT_ARGS, &factionA, &factionB))
	{
		if (factionA && factionB)
		{
			// Search for reaction
			tList<TESReactionForm::Reaction>::Iterator it;
			for (it = factionA->reaction.reactions.Begin(); !it.End(); ++it)
			{
				TESReactionForm::Reaction* reaction = it.Get();
				if (reaction && reaction->faction == factionB)
				{
					*result = reaction->modifier;
					if (IsConsoleMode())
						Console_Print("Reaction from %s to %s: %d", GetFullName(factionA), GetFullName(factionB), reaction->modifier);
					return true;
				}
			}
			
			// Default reaction if not found
			*result = 0;
			if (IsConsoleMode())
				Console_Print("No reaction found from %s to %s", GetFullName(factionA), GetFullName(factionB));
		}
	}

	return true;
}

bool Cmd_SetFactionReaction_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESFaction* factionA = NULL;
	TESFaction* factionB = NULL;
	SInt32 modifier = 0;
	
	if (ExtractArgs(EXTRACT_ARGS, &factionA, &factionB, &modifier))
	{
		if (factionA && factionB)
		{
			// Find existing reaction
			bool found = false;
			tList<TESReactionForm::Reaction>::Iterator it;
			for (it = factionA->reaction.reactions.Begin(); !it.End(); ++it)
			{
				TESReactionForm::Reaction* reaction = it.Get();
				if (reaction && reaction->faction == factionB)
				{
					reaction->modifier = modifier;
					found = true;
					break;
				}
			}
			
			// If not found, add new reaction
			if (!found)
			{
				TESReactionForm::Reaction* reaction = new TESReactionForm::Reaction();
				reaction->faction = factionB;
				reaction->modifier = modifier;
				reaction->reaction = TESReactionForm::Reaction::kNeutral;
				factionA->reaction.reactions.push_back(reaction);
			}
			
			factionA->MarkAsModified(0x00000001); // Mark as modified
			*result = 1;
			
			if (IsConsoleMode())
				Console_Print("Set reaction from %s to %s to %d", GetFullName(factionA), GetFullName(factionB), modifier);
		}
	}

	return true;
}

bool Cmd_AddFaction_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESFaction* faction = NULL;
	SInt32 rank = 0;
	TESObjectREFR* refr = NULL;
	
	if (ExtractArgs(EXTRACT_ARGS, &faction, &rank, &refr))
	{
		if (!refr) refr = thisObj;
		if (refr && faction)
		{
			TESActorBase* actorBase = (TESActorBase*)refr->baseForm;
			if (actorBase && actorBase->typeID == kFormType_NPC)
			{
				// Check if already in faction
				tList<TESActorBaseData::FactionListData>::Iterator it;
				for (it = actorBase->baseData.factionList.Begin(); !it.End(); ++it)
				{
					TESActorBaseData::FactionListData* data = it.Get();
					if (data && data->faction == faction)
					{
						// Already in faction, just update rank
						data->rank = rank;
						actorBase->MarkAsModified(0x00000001);
						*result = 1;
						
						if (IsConsoleMode())
							Console_Print("%s already in %s, updated rank to %d", GetFullName(refr), GetFullName(faction), rank);
						return true;
					}
				}
				
				// Add new faction entry
				TESActorBaseData::FactionListData* data = new TESActorBaseData::FactionListData();
				data->faction = faction;
				data->rank = rank;
				actorBase->baseData.factionList.push_back(data);
				
				actorBase->MarkAsModified(0x00000001);
				*result = 1;
				
				if (IsConsoleMode())
					Console_Print("Added %s to %s with rank %d", GetFullName(refr), GetFullName(faction), rank);
			}
		}
	}

	return true;
}

bool Cmd_RemoveFaction_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESFaction* faction = NULL;
	TESObjectREFR* refr = NULL;
	
	if (ExtractArgs(EXTRACT_ARGS, &faction, &refr))
	{
		if (!refr) refr = thisObj;
		if (refr && faction)
		{
			TESActorBase* actorBase = (TESActorBase*)refr->baseForm;
			if (actorBase && actorBase->typeID == kFormType_NPC)
			{
				// Find and remove faction entry
				SInt32 index = 0;
				tList<TESActorBaseData::FactionListData>::Iterator it;
				for (it = actorBase->baseData.factionList.Begin(); !it.End(); ++it, ++index)
				{
					TESActorBaseData::FactionListData* data = it.Get();
					if (data && data->faction == faction)
					{
						actorBase->baseData.factionList.RemoveNth(index);
						actorBase->MarkAsModified(0x00000001);
						*result = 1;
						
						if (IsConsoleMode())
							Console_Print("Removed %s from %s", GetFullName(refr), GetFullName(faction));
						return true;
					}
				}
				
				if (IsConsoleMode())
					Console_Print("%s is not in %s", GetFullName(refr), GetFullName(faction));
			}
		}
	}

	return true;
}
