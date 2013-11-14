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
	// Hack around obscure crash by getting the param data myself instead of trusting Param(TYPE_OBJECT)
	//short p1 = ((eventParam*)Param(TYPE_OBJECT))->evp.evpW.evpW0;
	short p1 = rdPtr->rHo.hoCurrentParam->evp.evpW.evpW0; Param(TYPE_OBJECT);

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
			LPOIL CurrentOi = GetOILPtr(OiList, CurrentQualToOi->qoiOiList); //get a pointer to the objectInfo for this object in the qualifier
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
				float value = 0;
				variable_map::iterator it = rdPtr->pVariableMap->find( Object2Fixed(CurrentObject) );
				if ( it != rdPtr->pVariableMap->end() )
				{
					float_vars::iterator j = it->second.floats.find( p2 );
					if ( j != it->second.floats.end() )
					{
						value = j->second;
					}
				}
				if ( Param_Comparison_Test( (Comparison)p3->compType, value, p4 ) )
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

			CurrentOi->oilNumOfSelected = numSelected;
			if ( numSelected > 0 )
			{
				PrevSelected->hoNextSelected = -1;
				bSelected = true;
			}
			else
			{
				CurrentOi->oilListSelected = -1;
			}
		}

		return bSelected;
	}
	else
	{
		LPOIL CurrentOi = GetOILPtr(OiList, p1); //get a pointer to the objectInfo for this object
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
			float value = 0;
			variable_map::iterator it = rdPtr->pVariableMap->find( Object2Fixed(CurrentObject) );
			if ( it != rdPtr->pVariableMap->end() )
			{
				float_vars::iterator j = it->second.floats.find( p2 );
				if ( j != it->second.floats.end() )
				{
					value = j->second;
				}
			}
			if ( Param_Comparison_Test( (Comparison)p3->compType, value, p4 ) )
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

		CurrentOi->oilNumOfSelected = numSelected;
		if ( numSelected > 0 )
		{
			PrevSelected->hoNextSelected = -1;
		}

		return (numSelected > 0);
	}
}

CONDITION(
	/* ID */			1,
	/* Name */			"%o: Ext. Alt. String %1 of %0 %2",
	/* Flags */			EVFLAGS_ALWAYS,
	/* Params */			(3,PARAM_OBJECT,"Object",PARAM_STRING,"Extended Alt. Sring Name",PARAM_CMPSTRING,"To Compare Against")
) {
	// Hack around obscure crash by getting the param data myself instead of trusting Param(TYPE_OBJECT)
	//short p1 = ((eventParam*)Param(TYPE_OBJECT))->evp.evpW.evpW0;
	short p1 = rdPtr->rHo.hoCurrentParam->evp.evpW.evpW0; Param(TYPE_OBJECT);

	char* p2 = (char*)Param(TYPE_STRING);
	ParamComp* p3 = GetComparisonParameter(rdPtr);
	char* p4 = (char*)Param(TYPE_STRING);

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
			LPOIL CurrentOi = GetOILPtr(OiList, CurrentQualToOi->qoiOiList); //get a pointer to the objectInfo for this object in the qualifier
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
				const char* value = 0;
				variable_map::iterator it = rdPtr->pVariableMap->find( Object2Fixed(CurrentObject) );
				if ( it != rdPtr->pVariableMap->end() )
				{
					string_vars::iterator j = it->second.strings.find( p2 );
					if ( j != it->second.strings.end() )
					{
						value = j->second.c_str();
					}
				}
				if ( Param_Comparison_Test( (Comparison)p3->compType, value, p4 ) )
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

			CurrentOi->oilNumOfSelected = numSelected;
			if ( numSelected > 0 )
			{
				PrevSelected->hoNextSelected = -1;
				bSelected = true;
			}
			else
			{
				CurrentOi->oilListSelected = -1;
			}
		}

		return bSelected;
	}
	else
	{
		LPOIL CurrentOi = GetOILPtr(OiList, p1); //get a pointer to the objectInfo for this object
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
			const char* value = 0;
			variable_map::iterator it = rdPtr->pVariableMap->find( Object2Fixed(CurrentObject) );
			if ( it != rdPtr->pVariableMap->end() )
			{
				string_vars::iterator j = it->second.strings.find( p2 );
				if ( j != it->second.strings.end() )
				{
					value = j->second.c_str();
				}
			}
			if ( Param_Comparison_Test( (Comparison)p3->compType, value, p4 ) )
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
		variable_map::iterator i = rdPtr->pVariableMap->find( Object2Fixed(p1) );
		if ( i == rdPtr->pVariableMap->end() )
		{
			i = rdPtr->pVariableMap->insert( objvar_pair(Object2Fixed(p1), variables()) ).first;
		}

		float_vars::iterator j = i->second.floats.find(p2);
		if ( j == i->second.floats.end() )
		{
			j = i->second.floats.insert( float_var(p2,p3) ).first;
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
		variable_map::iterator i = rdPtr->pVariableMap->find(Object2Fixed(p1));
		if ( i == rdPtr->pVariableMap->end() )
		{
			i = rdPtr->pVariableMap->insert( objvar_pair(Object2Fixed(p1), variables()) ).first;
		}

		float_vars::iterator j = i->second.floats.find(p2);
		if ( j == i->second.floats.end() )
		{
			j = i->second.floats.insert( float_var(p2,p3) ).first;
		}
		else
		{
			j->second = p3;
		}
	}
}

ACTION(
	/* ID */			2,
	/* Name */			"%o: Set Ext. Alt. String %1 of %0 to %2",
	/* Flags */			0,
	/* Params */			(3,PARAM_OBJECT,"Object",PARAM_STRING,"Extended Alt. String's Name",PARAM_STRING,"New Value")
) {
	object* p1 = (object*)Param(TYPE_OBJECT);
	char* p2 = (char*)Param(TYPE_STRING);
	char* p3 = (char*)Param(TYPE_STRING);

	if ( p1 && !(p1->hoFlags & HOF_DESTROYED) )
	{
		variable_map::iterator i = rdPtr->pVariableMap->find( Object2Fixed(p1) );
		if ( i == rdPtr->pVariableMap->end() )
		{
			i = rdPtr->pVariableMap->insert( objvar_pair(Object2Fixed(p1), variables()) ).first;
		}

		string_vars::iterator j = i->second.strings.find(p2);
		if ( j == i->second.strings.end() )
		{
			j = i->second.strings.insert( string_var(p2,p3) ).first;
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
	unsigned int p1 = ExParam(TYPE_INT);
	char* p2 = (char*)ExParam(TYPE_STRING);
	
	variable_map::iterator i = rdPtr->pVariableMap->find(p1);
	if ( i != rdPtr->pVariableMap->end() )
	{
		float_vars::iterator j = i->second.floats.find(p2);
		if ( j != i->second.floats.end() )
		{
			ReturnFloat( j->second );
		}
	}

	ReturnFloat( 0.0f );
}

EXPRESSION(
	/* ID */			1,
	/* Name */			"alt$(",
	/* Flags */			EXPFLAG_STRING,
	/* Params */			(2,EXPPARAM_NUMBER,"Fixed Value of Object",EXPPARAM_STRING,"Extended Alt. String's Name")
) {
	unsigned int p1 = ExParam(TYPE_INT);
	char* p2 = (char*)ExParam(TYPE_STRING);
	
	variable_map::iterator i = rdPtr->pVariableMap->find(p1);
	if ( i != rdPtr->pVariableMap->end() )
	{
		string_vars::iterator j = i->second.strings.find(p2);
		if ( j != i->second.strings.end() )
		{
			ReturnString( j->second.c_str() );
		}
	}

	ReturnString( "" );
}
