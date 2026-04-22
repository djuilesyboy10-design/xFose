#pragma once

#include "CommandTable.h"
#include "ParamInfos.h"

DEFINE_COMMAND(GetModelPath,
			   returns the model path of the specified object,
			   0, 1, kParams_OneOptionalObject);

DEFINE_COMMAND(SetModelPath,
			   sets the model path of the specified object,
			   0, 2, kParams_OneString_OneOptionalObject);

DEFINE_COMMAND(ModelPathIncludes, 
			   returns 1 if the specified substring exists within the objects model path, 
			   0, 2, kParams_OneString_OneOptionalInventoryObject);
