// rSDK demonstration release - www.aquadasoft.com
// Jamie McLaughlin and Jean Villy Edberg

// Send any queries or bug reports to support@aquadasoft.com

// ============================================================================
//
// This file contains the actions, conditions and expressions your object uses
// 
// ============================================================================

#include "common.h"
#include "DynExt.h"

#define TYPE_OBJECT 0x0010

inline long Float2Long(float value) {
	return *(long *)&value;
}

inline float Long2Float(long value) {
	return *(float *)&value;
}

inline int Object2Fixed(object* o) {
	return ((o->hoCreationId<<16) + o->hoNumber);
}

inline object* Fixed2Object(LPRDATA rdPtr, int fixed) {
	LPOBL objList = rdPtr->rHo.hoAdRunHeader->rhObjectList;
	return objList[0x0000FFFF&fixed].oblOffset;
}

// ============================================================================
//
// CONDITIONS
// 
// ============================================================================

CONDITION(
	/* ID */			0,
	/* Name */			"%o: Ext. Alt. Value %1 of %0 %2",
	/* Flags */			EVFLAGS_ALWAYS,
	/* Params */			(3,PARAM_OBJECT,"Object",PARAM_STRING,"Extended Alt. Value Name",PARAM_COMPARISON,"To Compare Against")
) {
	short p1 = ((eventParam*)Param(TYPE_OBJECT))->evp.evpW.evpW0;
	char* p2 = (char*)Param(TYPE_STRING);
	ParamComp* p3 = GetComparisonParameter(rdPtr);
	double p4 = Long2Float(CNC_GetFloatParameter(rdPtr));

	LPRH rhPtr = rdPtr->rHo.hoAdRunHeader;      //get a pointer to the mmf runtime header
	LPOBL ObjectList = rhPtr->rhObjectList;     //get a pointer to the mmf object list
	LPOIL OiList = rhPtr->rhOiList;             //get a pointer to the mmf object info list
	LPQOI QualToOiList = rhPtr->rhQualToOiList; //get a pointer to the mmf qualifier to Oi list

	bool bSelected = false;

	if ( p1 & 0x8000 ) // dealing with a qualifier...
	{
		// For qualifiers evpW0( & 0x7FFF ) is the offset in the qualToOi array
		// And evpW1( & 0x7FFF ) is the qualifier number.
		LPQOI CurrentQualToOiStart = PtrAddBytes(QualToOiList, p1 & 0x7FFF);
		LPQOI CurrentQualToOi = CurrentQualToOiStart;

		// Loop through all objects associated with this qualifier
		for (CurrentQualToOi; CurrentQualToOi->qoiOiList >= 0;
			CurrentQualToOi = PtrAddBytes(CurrentQualToOi, 4) )
		{
			LPOIL CurrentOi = OiList + CurrentQualToOi->qoiOiList; //get a pointer to the objectInfo for this object in the qualifier
			if ( CurrentOi->oilNObjects <= 0 ) //skip if there are none of the object
				continue;

			bool prevSelected = (CurrentOi->oilEventCount == rhPtr->rh2.rh2EventCount); //find out if conditions have selected any objects yet
			if ( prevSelected && CurrentOi->oilNumOfSelected <= 0 ) //if "0" have been selected (blame qualifiers) then skip
				continue;

			object* CurrentObject = NULL;
			object* PrevSelected = NULL;
			int iCount = 0;
			int numSelected = 0;
			if ( prevSelected )
			{
				CurrentObject = ObjectList[CurrentOi->oilListSelected].oblOffset;
				iCount = CurrentOi->oilNumOfSelected;
			}
			else
			{
				CurrentObject = ObjectList[CurrentOi->oilObject].oblOffset;
				iCount = CurrentOi->oilNObjects;
				CurrentOi->oilEventCount = rhPtr->rh2.rh2EventCount; //tell mmf that the object selection is relevant to this event
			}
			for(int i = 0; i < iCount; i++)
			{
				variable_map::iterator it = rdPtr->pVariableMap->find( CurrentObject );
				if ( it != rdPtr->pVariableMap->end() )
				{
					variables::iterator j = it->second.find( p2 );
					if ( j != it->second.end() &&
						Param_Comparison_Test( (Comparison)p3->compType, j->second, p4 ) )
					{
						if ( numSelected++ == 0 )
						{
							CurrentOi->oilListSelected = CurrentObject->hoNumber; //select the first object
						}
						else
						{
							PrevSelected->hoNextSelected = CurrentObject->hoNumber;
						}

						PrevSelected = CurrentObject;
					}

					if ( prevSelected )
					{
						if ( CurrentObject->hoNextSelected >= 0 )
						{
							CurrentObject = ObjectList[CurrentObject->hoNextSelected].oblOffset;
						}
						else
						{
							break;
						}
					}
					else
					{
						if ( CurrentObject->hoNumNext >= 0 )
						{
							CurrentObject = ObjectList[CurrentObject->hoNumNext].oblOffset;
						}
						else
						{
							break;
						}
					}
				}
			}

			CurrentOi->oilNumOfSelected = numSelected;
			if ( numSelected > 0 )
			{
				PrevSelected->hoNextSelected = -1;
				bSelected = true;
			}
		}

		return bSelected;
	}
	else
	{
		LPOIL CurrentOi = OiList + p1; //get a pointer to the objectInfo for this object
		if ( CurrentOi->oilNObjects <= 0 ) //return if there are none of the object
			return false;

		bool prevSelected = (CurrentOi->oilEventCount == rhPtr->rh2.rh2EventCount); //find out if conditions have selected any objects yet
		if ( prevSelected && CurrentOi->oilNumOfSelected <= 0 ) //if "0" have been selected (blame qualifiers) then return
			return false;

		object* CurrentObject = NULL;
		object* PrevSelected = NULL;
		int iCount = 0;
		int numSelected = 0;
		if ( prevSelected )
		{
			CurrentObject = ObjectList[CurrentOi->oilListSelected].oblOffset;
			iCount = CurrentOi->oilNumOfSelected;
		}
		else
		{
			CurrentObject = ObjectList[CurrentOi->oilObject].oblOffset;
			iCount = CurrentOi->oilNObjects;
			CurrentOi->oilEventCount = rhPtr->rh2.rh2EventCount; //tell mmf that the object selection is relevant to this event
		}
		for(int i = 0; i < iCount; i++)
		{
			variable_map::iterator it = rdPtr->pVariableMap->find( CurrentObject );
			if ( it != rdPtr->pVariableMap->end() )
			{
				variables::iterator j = it->second.find( p2 );
				if ( Param_Comparison_Test( (Comparison)p3->compType, (j != it->second.end()) ? (j->second) : (0.0), p4 ) )
				{
					if ( numSelected++ == 0 )
					{
						CurrentOi->oilListSelected = CurrentObject->hoNumber; //select the first object
					}
					else
					{
						PrevSelected->hoNextSelected = CurrentObject->hoNumber;
					}

					PrevSelected = CurrentObject;
				}

				if ( prevSelected )
				{
					if ( CurrentObject->hoNextSelected < 0 )
					{
						break;
					}
					else
					{
						CurrentObject = ObjectList[CurrentObject->hoNextSelected].oblOffset;
					}
				}
				else
				{
					if ( CurrentObject->hoNumNext < 0 )
					{
						break;
					}
					else
					{
						CurrentObject = ObjectList[CurrentObject->hoNumNext].oblOffset;
					}
				}
			}
		}

		CurrentOi->oilNumOfSelected = numSelected;
		if ( numSelected > 0 )
		{
			PrevSelected->hoNextSelected = -1;
		}

		return (numSelected > 0);
	}
}

// ============================================================================
//
// ACTIONS
// 
// ============================================================================

ACTION(
	/* ID */			0,
	/* Name */			"%o: Set Ext. Alt. Value %1 of %0 to %2",
	/* Flags */			0,
	/* Params */			(3,PARAM_OBJECT,"Object",PARAM_STRING,"Extended Alt. Value's Name",PARAM_NUMBER,"New Value")
) {
	object* p1 = (object*)Param(TYPE_OBJECT);
	char* p2 = (char*)Param(TYPE_STRING);
	float p3 = Long2Float(CNC_GetFloatParameter(rdPtr));

	if ( p1 && !(p1->hoFlags & HOF_DESTROYED) )
	{
		variable_map::iterator i = rdPtr->pVariableMap->find(p1);
		if ( i == rdPtr->pVariableMap->end() )
		{
			i = rdPtr->pVariableMap->insert( pair<object*,variables>(p1,variables()) ).first;
		}

		variables::iterator j = i->second.find(p2);
		if ( j == i->second.end() )
		{
			j = i->second.insert( pair<string,float>(p2,p3) ).first;
		}
		else
		{
			j->second = p3;
		}
	}
}

ACTION(
	/* ID */			1,
	/* Name */			"%o: Spread Value %2 (+%3) in Ext. Alt. Value %1 of %0",
	/* Flags */			0,
	/* Params */			(4,PARAM_OBJECT,"Object",PARAM_STRING,"Extended Alt. Value's Name",PARAM_NUMBER,"Start Value",PARAM_NUMBER,"Step Value")
) {
	object* p1 = (object*)Param(TYPE_OBJECT);
	char* p2 = (char*)Param(TYPE_STRING);
	float p3 = Long2Float(CNC_GetFloatParameter(rdPtr));
	float p4 = Long2Float(CNC_GetFloatParameter(rdPtr));
	p3 += p4 * rdPtr->rHo.hoAdRunHeader->rh2.rh2ActionLoopCount;

	if ( p1 && !(p1->hoFlags & HOF_DESTROYED) )
	{
		variable_map::iterator i = rdPtr->pVariableMap->find(p1);
		if ( i == rdPtr->pVariableMap->end() )
		{
			i = rdPtr->pVariableMap->insert( pair<object*,variables>(p1,variables()) ).first;
		}

		variables::iterator j = i->second.find(p2);
		if ( j == i->second.end() )
		{
			j = i->second.insert( pair<string,float>(p2,p3) ).first;
		}
		else
		{
			j->second = p3;
		}
	}
}

// ============================================================================
//
// EXPRESSIONS
// 
// ============================================================================

EXPRESSION(
	/* ID */			0,
	/* Name */			"alt(",
	/* Flags */			0,
	/* Params */			(2,EXPPARAM_NUMBER,"Fixed Value of Object",EXPPARAM_STRING,"Extended Alt. Value's Name")
) {
	int p1 = ExParam(TYPE_INT);
	char* p2 = (char*)ExParam(TYPE_STRING);
	
	variable_map::iterator i = rdPtr->pVariableMap->find( Fixed2Object(rdPtr,p1) );
	if ( i != rdPtr->pVariableMap->end() )
	{
		variables::iterator j = i->second.find(p2);
		if ( j != i->second.end() )
		{
			ReturnFloat( (float)(j->second) );
		}
	}

	ReturnFloat( 0.0f );
}
