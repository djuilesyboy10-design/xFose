#pragma once

#include "CommandTable.h"
#include "ParamInfos.h"

static ParamInfo kParams_Faction_OptionalRef[] =
{
	{	"faction",	kParamType_Faction,	0 },
	{	"reference",	kParamType_Actor,	1 },
};

static ParamInfo kParams_FactionRank[] =
{
	{	"faction",	kParamType_Faction,	0 },
	{	"rank",	kParamType_Integer,	0 },
	{	"reference",	kParamType_Actor,	1 },
};

static ParamInfo kParams_TwoFactions[] =
{
	{	"factionA",	kParamType_Faction,	0 },
	{	"factionB",	kParamType_Faction,	0 },
};

static ParamInfo kParams_TwoFactions_Modifier[] =
{
	{	"factionA",	kParamType_Faction,	0 },
	{	"factionB",	kParamType_Faction,	0 },
	{	"modifier",	kParamType_Integer,	0 },
};

DEFINE_CMD_ALT(GetFactionRank, , Returns the rank of a reference in a faction, 0, 2, kParams_FactionRank);
DEFINE_CMD_ALT(SetFactionRank, , Sets the rank of a reference in a faction, 0, 3, kParams_FactionRank);
DEFINE_CMD_ALT(GetFactionReaction, , Returns the reaction modifier between two factions, 0, 2, kParams_TwoFactions);
DEFINE_CMD_ALT(SetFactionReaction, , Sets the reaction modifier between two factions, 0, 3, kParams_TwoFactions_Modifier);
DEFINE_CMD_ALT(AddFaction, , Adds a reference to a faction with a specified rank, 0, 3, kParams_FactionRank);
DEFINE_CMD_ALT(RemoveFaction, , Removes a reference from a faction, 0, 2, kParams_Faction_OptionalRef);
