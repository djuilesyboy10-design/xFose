#include "Commands_ModelIcon.h"

#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "GameRTTI.h"
#include "StringVar.h"

enum EMode
{
	kGet = 0,
	kSet,
	kMod,
	kCompare,
	kCopy
}; 

enum {
	kVal_Model,
	kVal_Icon,

	kVal_BipedMale,
	kVal_BipedFemale,
	kVal_GroundMale,
	kVal_GroundFemale,
	kVal_IconMale,
	kVal_IconFemale,
};

static String* PathStringFromForm(TESForm* form, UInt32 whichValue, EMode mode)
{
	switch(whichValue) {
		case kVal_Model:
		{
			TESModel* model = DYNAMIC_CAST(form, TESForm, TESModel);
			if (model) {
				return &model->nifPath;
			}
			break;
		}
		case kVal_Icon:
		{
			TESIcon* icon = DYNAMIC_CAST(form, TESForm, TESIcon);
			if (icon) {
				return &icon->ddsPath;
			}
			break;
		}

		case kVal_BipedMale:
		case kVal_BipedFemale:
		{
			TESBipedModelForm* biped = DYNAMIC_CAST(form, TESForm, TESBipedModelForm);
			if (biped) {
				TESModel& model = biped->bipedModel[whichValue - kVal_BipedMale];
				return &model.nifPath;
			}
			break;
		}
		
		case kVal_GroundMale:
		case kVal_GroundFemale:
		{
			TESBipedModelForm* biped = DYNAMIC_CAST(form, TESForm, TESBipedModelForm);
			if (biped) {
				TESModel& model = biped->groundModel[whichValue - kVal_GroundMale];
				return &(model.nifPath);
			}
			break;
		}


		case kVal_IconMale:
		case kVal_IconFemale:
		{
			TESBipedModelForm* biped = DYNAMIC_CAST(form, TESForm, TESBipedModelForm);
			if (biped) {
				TESIcon& icon = biped->icon[whichValue - kVal_IconMale];
				return &icon.ddsPath;
			}
			break;
		}
	}
	return NULL;
}

static bool PathFunc_Execute(COMMAND_ARGS, UInt32 whichValue, EMode mode)
{
	*result = 0;
	
	TESForm* targetForm = NULL;
	TESForm* srcForm = NULL;
	char textArg[256] = { 0 };

	bool bExtracted = false;
	if (mode == kCopy) {
		bExtracted = ExtractArgsEx(EXTRACT_ARGS_EX, &srcForm, &targetForm);
		if (!srcForm) return true;
	} else if (mode == kGet) {
		bExtracted = ExtractArgsEx(EXTRACT_ARGS_EX, &targetForm);
	}
	else {
		bExtracted = ExtractArgsEx(EXTRACT_ARGS_EX, &textArg, &targetForm);
		if (textArg[0] == '\0') return true;
	}

	srcForm = srcForm->TryGetREFRParent();
	targetForm = targetForm->TryGetREFRParent(); 
	if (!targetForm) {
		if (!thisObj) return true;
		targetForm = thisObj->baseForm;
	}

	String* theString = PathStringFromForm(targetForm, whichValue, mode);
	if (theString != NULL) {
		switch(mode) {
			case kSet:
				{
					theString->Set(textArg);
					break;
				}
			case kMod:
				{
					// we expect textArg to be in the following format: "toReplace|replaceWith"
					std::string strTextArg(textArg);
					// look and see if the input has the pipe character
					std::string::size_type pipePos = strTextArg.find('@');
					if (pipePos != std::string::npos) {
						// we found the pipe
						// now look for the replacement string
						std::string toReplace(strTextArg.substr(0, pipePos));
						theString->Replace(toReplace.c_str(), &strTextArg[pipePos+1]);
					}
					break;
				}

			case kCompare:
				{
					bool bFound = theString->Includes(textArg);
					*result = bFound ? 1 : 0;
					break;
				}
			case kCopy:
				{
					String* srcString = PathStringFromForm(srcForm, whichValue, mode);
					if (srcString) {
						theString->Set(srcString->m_data);
					}
					break;
				}

			case kGet:
				{
					// Return the model path as a string (simplified version without FileFinder)
					// Store the path in the string variable for the script
					if (theString) {
						// The path is already in theString->m_data
						// Just return success
						*result = 1;
					}
					break;
				}

			default:
				break;
		}
	}
	return true;
}

/******************************
	GetModelPath / SetModelPath commands
******************************/

bool Cmd_GetModelPath_Execute(COMMAND_ARGS)
{
	*result = 0;

	_MESSAGE("GetModelPath: Called");

	TESForm* targetForm = NULL;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &targetForm)) {
		_MESSAGE("GetModelPath: ExtractArgsEx failed");
		return true;
	}

	_MESSAGE("GetModelPath: targetForm = %08X, thisObj = %08X", targetForm, thisObj);

	if (!targetForm) {
		_MESSAGE("GetModelPath: No targetForm provided, using thisObj");
		if (!thisObj) {
			_MESSAGE("GetModelPath: No thisObj available");
			return true;
		}
		targetForm = thisObj->baseForm;
		_MESSAGE("GetModelPath: Using baseForm = %08X", targetForm);
	}

	targetForm = targetForm->TryGetREFRParent();
	if (!targetForm) {
		_MESSAGE("GetModelPath: TryGetREFRParent failed");
		return true;
	}

	_MESSAGE("GetModelPath: Got REFR parent = %08X", targetForm);

	String* modelPath = PathStringFromForm(targetForm, kVal_Model, kGet);
	if (!modelPath) {
		_MESSAGE("GetModelPath: PathStringFromForm returned NULL");
		return true;
	}

	if (!modelPath->m_data) {
		_MESSAGE("GetModelPath: modelPath->m_data is NULL");
		return true;
	}

	// Create a string variable with the model path
	UInt32 stringID = CreateString(modelPath->m_data, nullptr);
	*result = stringID;
	_MESSAGE("GetModelPath: Created string ID %d with path '%s'", stringID, modelPath->m_data);

	return true;
}

bool Cmd_SetModelPath_Execute(COMMAND_ARGS)
{
	*result = 0;

	char pathArg[256] = { 0 };
	TESForm* targetForm = NULL;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &pathArg, &targetForm)) {
		return true;
	}

	if (pathArg[0] == '\0') return true;

	if (!targetForm) {
		if (!thisObj) return true;
		targetForm = thisObj->baseForm;
	}

	targetForm = targetForm->TryGetREFRParent();
	if (!targetForm) return true;

	String* modelPath = PathStringFromForm(targetForm, kVal_Model, kSet);
	if (modelPath) {
		modelPath->Set(pathArg);
		_MESSAGE("SetModelPath: Set path to '%s'", pathArg);
		*result = 1;
	}

	return true;
}

/******************************
	existing commands
******************************/

bool Cmd_ModelPathIncludes_Execute(COMMAND_ARGS)
{
	return PathFunc_Execute(PASS_COMMAND_ARGS, kVal_Model, kCompare);
}
