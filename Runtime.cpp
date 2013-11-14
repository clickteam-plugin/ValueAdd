// rSDK demonstration release - www.aquadasoft.com
// Jamie McLaughlin and Jean Villy Edberg

// Send any queries or bug reports to support@aquadasoft.com

// ============================================================================
//
// This file contains routines that are handled during the Runtime
// 
// ============================================================================

// Common Include
#include	"common.h"
#include "DynExt.h"

// --------------------
// GetRunObjectDataSize
// --------------------
// Returns the size of the runtime datazone of the object
// 
short WINAPI DLLExport GetRunObjectDataSize(fprh rhPtr, LPEDATA edPtr)
{
	return(sizeof(RUNDATA));
}


// ---------------
// CreateRunObject
// ---------------
// The routine where the object is actually created
// 
short WINAPI DLLExport CreateRunObject(LPRDATA rdPtr, LPEDATA edPtr, fpcob cobPtr)
{
	// Do some rSDK stuff
	#include "rCreateRunObject.h"
	
	/*
	   This routine runs when your object is created, as you might have guessed.
	   It is here that you must transfer any data you need in rdPtr from edPtr,
	   because after this has finished you cannot access it again!
	   Also, if you have anything to initialise (e.g. dynamic arrays, surface objects)
	   you should do it here, and free your resources in DestroyRunObject.
	   See Graphic_Object_Ex.txt for an example of what you may put here.
	*/

	rdPtr->pVariableMap = new variable_map;

	// No errors
	return 0;
}


// ----------------
// DestroyRunObject
// ----------------
// Destroys the run-time object
// 
short WINAPI DLLExport DestroyRunObject(LPRDATA rdPtr, long fast)
{
/*
   When your object is destroyed (either with a Destroy action or at the end of
   the frame) this routine is called. You must free any resources you have allocated!
   See Graphic_Object_Ex.txt for an example of what you may put here.
*/
	// No errors
	delete rdPtr->rRd;

	delete rdPtr->pVariableMap;

	return 0;
}


// ----------------
// HandleRunObject
// ----------------
// Called (if you want) each loop, this routine makes the object live
// 
short WINAPI DLLExport HandleRunObject(LPRDATA rdPtr)
{
/*
   If your extension will draw to the MMF window you should first 
   check if anything about its display has changed :

       if (rdPtr->roc.rcChanged)
          return REFLAG_DISPLAY;
       else
          return 0;

   You will also need to make sure you change this flag yourself 
   to 1 whenever you want to redraw your object
 
   If your extension won't draw to the window, but it still needs 
   to do something every MMF loop use :

        return 0;

   If you don't need to do something every loop, use :

        return REFLAG_ONESHOT;

   This doesn't mean this function can never run again. If you want MMF
   to handle your object again (causing this code to run) use this function:

        rdPtr->rRd->Rehandle();

   At the end of the loop this code will run
*/

	variable_map::iterator i = rdPtr->pVariableMap->begin();

	for ( i; i != rdPtr->pVariableMap->end(); )
	{
		headerObject* pObject = Fixed2Object(rdPtr, i->first);
		if (!pObject || pObject->hoFlags & HOF_DESTROYED)
		{
			i = rdPtr->pVariableMap->erase(i);
		}
		else
		{
			i++;
		}
	}

	// Will be called next loop	
	return 0;
}

// ----------------
// DisplayRunObject
// ----------------
// Draw the object in the application screen.
// 
short WINAPI DLLExport DisplayRunObject(LPRDATA rdPtr)
{
/*
   If you return REFLAG_DISPLAY in HandleRunObject this routine will run.
   See Graphic_Object_Ex.txt for an example of what you may put here.
*/
	// Ok
	return 0;
}

// -------------------
// GetRunObjectSurface
// -------------------
// Implement this function instead of DisplayRunObject if your extension
// supports ink effects and transitions. Note: you can support ink effects
// in DisplayRunObject too, but this is automatically done if you implement
// GetRunObjectSurface (MMF applies the ink effect to the transition).
//
// Note: do not forget to enable the function in the .def file 
// if you remove the comments below.
/*
cSurface* WINAPI DLLExport GetRunObjectSurface(LPRDATA rdPtr)
{
	return NULL;
}
*/

// -------------------------
// GetRunObjectCollisionMask
// -------------------------
// Implement this function if your extension supports fine collision mode (OEPREFS_FINECOLLISIONS),
// Or if it's a background object and you want Obstacle properties for this object.
//
// Should return NULL if the object is not transparent.
//
// Note: do not forget to enable the function in the .def file 
// if you remove the comments below.
//
/*
cSurface* WINAPI DLLExport GetRunObjectCollisionMask(LPRDATA rdPtr, LPARAM lParam)
{
	// Typical example for active objects
	// ----------------------------------
	// Opaque? collide with box
	if ( (rdPtr->rs.rsEffect & EFFECTFLAG_TRANSPARENT) == 0 )	// Note: only if your object has the OEPREFS_INKEFFECTS option
		return NULL;

	// Transparent? Create mask
	LPSMASK pMask = rdPtr->m_pColMask;
	if ( pMask == NULL )
	{
		if ( rdPtr->m_pSurface != NULL )
		{
			DWORD dwMaskSize = rdPtr->m_pSurface->CreateMask(NULL, lParam);
			if ( dwMaskSize != 0 )
			{
				pMask = (LPSMASK)calloc(dwMaskSize, 1);
				if ( pMask != NULL )
				{
					rdPtr->m_pSurface->CreateMask(pMask, lParam);
					rdPtr->m_pColMask = pMask;
				}
			}
		}
	}

	// Note: for active objects, lParam is always the same.
	// For background objects (OEFLAG_BACKGROUND), lParam maybe be different if the user uses your object
	// as obstacle and as platform. In this case, you should store 2 collision masks
	// in your data: one if lParam is 0 and another one if lParam is different from 0.

	return pMask;
}
*/

// ----------------
// PauseRunObject
// ----------------
// Enters the pause mode
// 
short WINAPI DLLExport PauseRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}


// -----------------
// ContinueRunObject
// -----------------
// Quits the pause mode
//
short WINAPI DLLExport ContinueRunObject(LPRDATA rdPtr)
{
	// Ok
	return 0;
}

// -----------------
// SaveRunObject
// -----------------
// Saves the object to disk
// 
BOOL WINAPI SaveRunObject(LPRDATA rdPtr, HANDLE hf)
{
	BOOL bOK = FALSE;

#ifndef VITALIZE

	do
	{
		DWORD dwBytesWritten;
		BOOL WriteSuccess;

		WORD SaveVersion = SAVEDATAVERSION;
		WriteSuccess = WriteFile(hf, &SaveVersion, sizeof(SaveVersion), &dwBytesWritten, NULL);
		if (!WriteSuccess) break;

		WORD SavedObjects = rdPtr->pVariableMap->size();
		WriteSuccess = WriteFile(hf, &SavedObjects, sizeof(SavedObjects), &dwBytesWritten, NULL);
		if (!WriteSuccess) break;

		variable_map::iterator i = rdPtr->pVariableMap->begin();
		for (i; i != rdPtr->pVariableMap->end(); i++)
		{
			unsigned int ObjectID = i->first;
			WriteSuccess = WriteFile(hf, &ObjectID, sizeof(ObjectID), &dwBytesWritten, NULL);
			if (!WriteSuccess) break;

			WORD SavedValues = i->second.floats.size();
			WriteSuccess = WriteFile(hf, &SavedValues, sizeof(SavedValues), &dwBytesWritten, NULL);
			if (!WriteSuccess) break;

			float_vars::iterator j = i->second.floats.begin();
			for (j; j != i->second.floats.end(); j++)
			{
				WORD ValueNameLength = j->first.size();
				WriteSuccess = WriteFile(hf, &ValueNameLength, sizeof(ValueNameLength), &dwBytesWritten, NULL);
				if (!WriteSuccess) break;

				const char* ValueName = j->first.c_str();
				WriteSuccess = WriteFile(hf, ValueName, ValueNameLength, &dwBytesWritten, NULL);
				if (!WriteSuccess) break;

				float Value = j->second;
				WriteSuccess = WriteFile(hf, &Value, sizeof(Value), &dwBytesWritten, NULL);
				if (!WriteSuccess) break;
			}

			WORD SavedStrings = i->second.strings.size();
			WriteSuccess = WriteFile(hf, &SavedStrings, sizeof(SavedStrings), &dwBytesWritten, NULL);
			if (!WriteSuccess) break;

			string_vars::iterator js = i->second.strings.begin();
			for (js; js != i->second.strings.end(); js++)
			{
				WORD ValueNameLength = js->first.size();
				WriteSuccess = WriteFile(hf, &ValueNameLength, sizeof(ValueNameLength), &dwBytesWritten, NULL);
				if (!WriteSuccess) break;

				const char* ValueName = js->first.c_str();
				WriteSuccess = WriteFile(hf, ValueName, ValueNameLength, &dwBytesWritten, NULL);
				if (!WriteSuccess) break;

				DWORD StringValueLength = js->second.size();
				WriteSuccess = WriteFile(hf, &StringValueLength, sizeof(StringValueLength), &dwBytesWritten, NULL);
				if (!WriteSuccess) break;

				const char* StringValue = js->second.c_str();
				WriteSuccess = WriteFile(hf, StringValue, StringValueLength, &dwBytesWritten, NULL);
				if (!WriteSuccess) break;
			}
		}
		if (!WriteSuccess) break;

		bOK = TRUE;
	}
	while(false);

#endif // VITALIZE

	return bOK;
}

// -----------------
// LoadRunObject
// -----------------
// Loads the object from disk
// 
BOOL WINAPI LoadRunObject(LPRDATA rdPtr, HANDLE hf)
{            
	BOOL bOK=FALSE;

#ifndef VITALIZE

	do
	{
		DWORD dwBytesRead;
		BOOL ReadSuccess;

		WORD SaveVersion;
		ReadSuccess = ReadFile(hf, &SaveVersion, sizeof(SaveVersion), &dwBytesRead, NULL);
		if (!ReadSuccess) break;

		if (SaveVersion < SAVEDATAVERSION_MINSUPPORTED ||
			SaveVersion > SAVEDATAVERSION)
		{
			ReadSuccess = false;
			break;
		}

		WORD SavedObjects;
		ReadSuccess = ReadFile(hf, &SavedObjects, sizeof(SavedObjects), &dwBytesRead, NULL);
		if (!ReadSuccess) break;

		rdPtr->pVariableMap->clear();

		variable_map::iterator it = rdPtr->pVariableMap->begin();
		for (int i = 0; i < SavedObjects; i++)
		{
			unsigned int ObjectID;
			ReadSuccess = ReadFile(hf, &ObjectID, sizeof(ObjectID), &dwBytesRead, NULL);
			if (!ReadSuccess) break;

			it = rdPtr->pVariableMap->insert( it, objvar_pair(ObjectID, variables()) );

			WORD SavedValues;
			ReadSuccess = ReadFile(hf, &SavedValues, sizeof(SavedValues), &dwBytesRead, NULL);
			if (!ReadSuccess) break;

			float_vars::iterator jt = it->second.floats.begin();
			for (int j = 0; j < SavedValues; j++)
			{
				WORD ValueNameLength;
				ReadSuccess = ReadFile(hf, &ValueNameLength, sizeof(ValueNameLength), &dwBytesRead, NULL);
				if (!ReadSuccess) break;

				string ValueName;
				ValueName.resize(ValueNameLength);
				ReadSuccess = ReadFile(hf, &ValueName[0], ValueNameLength, &dwBytesRead, NULL);
				if (!ReadSuccess) break;

				float Value;
				ReadSuccess = ReadFile(hf, &Value, sizeof(Value), &dwBytesRead, NULL);
				if (!ReadSuccess) break;

				jt = it->second.floats.insert( jt, float_var(ValueName, Value) );
			}

			if (SaveVersion >= SAVEDATAVERSION_STRINGSADDED)
			{
				WORD SavedStrings;
				ReadSuccess = ReadFile(hf, &SavedStrings, sizeof(SavedStrings), &dwBytesRead, NULL);
				if (!ReadSuccess) break;

				string_vars::iterator jt = it->second.strings.begin();
				for (int j = 0; j < SavedStrings; j++)
				{
					WORD ValueNameLength;
					ReadSuccess = ReadFile(hf, &ValueNameLength, sizeof(ValueNameLength), &dwBytesRead, NULL);
					if (!ReadSuccess) break;

					string ValueName;
					ValueName.resize(ValueNameLength);
					ReadSuccess = ReadFile(hf, &ValueName[0], ValueNameLength, &dwBytesRead, NULL);
					if (!ReadSuccess) break;

					DWORD StringValueLength;
					ReadSuccess = ReadFile(hf, &StringValueLength, sizeof(StringValueLength), &dwBytesRead, NULL);
					if (!ReadSuccess) break;

					string StringValue;
					StringValue.resize(StringValueLength);
					ReadSuccess = ReadFile(hf, &StringValue[0], StringValueLength, &dwBytesRead, NULL);
					if (!ReadSuccess) break;

					jt = it->second.strings.insert( jt, string_var(ValueName, StringValue) );
				}
			}
		}
		if (!ReadSuccess) break;

		bOK = TRUE;
	}
	while(false);

#endif // VITALIZE

	return bOK; 
}




// ============================================================================
//
// START APP / END APP / START FRAME / END FRAME routines
// 
// ============================================================================

// -------------------
// StartApp
// -------------------
// Called when the application starts or restarts.
// Useful for storing global data
// 
void WINAPI DLLExport StartApp(mv _far *mV, CRunApp* pApp)
{
	// Example
	// -------
	// Delete global data (if restarts application)
//	CMyData* pData = (CMyData*)mV->mvGetExtUserData(pApp, hInstLib);
//	if ( pData != NULL )
//	{
//		delete pData;
//		mV->mvSetExtUserData(pApp, hInstLib, NULL);
//	}
	InitOiListItemSize(mV);
}

// -------------------
// EndApp
// -------------------
// Called when the application ends.
// 
void WINAPI DLLExport EndApp(mv _far *mV, CRunApp* pApp)
{
	// Example
	// -------
	// Delete global data
//	CMyData* pData = (CMyData*)mV->mvGetExtUserData(pApp, hInstLib);
//	if ( pData != NULL )
//	{
//		delete pData;
//		mV->mvSetExtUserData(pApp, hInstLib, NULL);
//	}
}

// -------------------
// StartFrame
// -------------------
// Called when the frame starts or restarts.
// 
void WINAPI DLLExport StartFrame(mv _far *mV, DWORD dwReserved, int nFrameIndex)
{

}

// -------------------
// EndFrame
// -------------------
// Called when the frame ends.
// 
void WINAPI DLLExport EndFrame(mv _far *mV, DWORD dwReserved, int nFrameIndex)
{

}

// ============================================================================
//
// TEXT ROUTINES (if OEFLAG_TEXT)
// 
// ============================================================================

// -------------------
// GetRunObjectFont
// -------------------
// Return the font used by the object.
// 
/*

  // Note: do not forget to enable the functions in the .def file 
  // if you remove the comments below.

void WINAPI GetRunObjectFont(LPRDATA rdPtr, LOGFONT* pLf)
{
	// Example
	// -------
	// GetObject(rdPtr->m_hFont, sizeof(LOGFONT), pLf);
}

// -------------------
// SetRunObjectFont
// -------------------
// Change the font used by the object.
// 
void WINAPI SetRunObjectFont(LPRDATA rdPtr, LOGFONT* pLf, RECT* pRc)
{
	// Example
	// -------
//	HFONT hFont = CreateFontIndirect(pLf);
//	if ( hFont != NULL )
//	{
//		if (rdPtr->m_hFont!=0)
//			DeleteObject(rdPtr->m_hFont);
//		rdPtr->m_hFont = hFont;
//		SendMessage(rdPtr->m_hWnd, WM_SETFONT, (WPARAM)rdPtr->m_hFont, FALSE);
//	}

}

// ---------------------
// GetRunObjectTextColor
// ---------------------
// Return the text color of the object.
// 
COLORREF WINAPI GetRunObjectTextColor(LPRDATA rdPtr)
{
	// Example
	// -------
	return 0;	// rdPtr->m_dwColor;
}

// ---------------------
// SetRunObjectTextColor
// ---------------------
// Change the text color of the object.
// 
void WINAPI SetRunObjectTextColor(LPRDATA rdPtr, COLORREF rgb)
{
	// Example
	// -------
	rdPtr->m_dwColor = rgb;
	InvalidateRect(rdPtr->m_hWnd, NULL, TRUE);
}
*/


// ============================================================================
//
// DEBUGGER ROUTINES
// 
// ============================================================================

// -----------------
// GetDebugTree
// -----------------
// This routine returns the address of the debugger tree
//
LPWORD WINAPI DLLExport GetDebugTree(LPRDATA rdPtr)
{
#if !defined(RUN_ONLY)
	return DebugTree;
#else
	return NULL;
#endif // !defined(RUN_ONLY)
}

// -----------------
// GetDebugItem
// -----------------
// This routine returns the text of a given item.
//
void WINAPI DLLExport GetDebugItem(LPSTR pBuffer, LPRDATA rdPtr, int id)
{
#if !defined(RUN_ONLY)

	// Example
	// -------
/*
	char temp[DB_BUFFERSIZE];

	switch (id)
	{
	case DB_CURRENTSTRING:
		LoadString(hInstLib, IDS_CURRENTSTRING, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->text);
		break;
	case DB_CURRENTVALUE:
		LoadString(hInstLib, IDS_CURRENTVALUE, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->value);
		break;
	case DB_CURRENTCHECK:
		LoadString(hInstLib, IDS_CURRENTCHECK, temp, DB_BUFFERSIZE);
		if (rdPtr->check)
			wsprintf(pBuffer, temp, "TRUE");
		else
			wsprintf(pBuffer, temp, "FALSE");
		break;
	case DB_CURRENTCOMBO:
		LoadString(hInstLib, IDS_CURRENTCOMBO, temp, DB_BUFFERSIZE);
		wsprintf(pBuffer, temp, rdPtr->combo);
		break;
	}
*/

#endif // !defined(RUN_ONLY)
}

// -----------------
// EditDebugItem
// -----------------
// This routine allows to edit editable items.
//
void WINAPI DLLExport EditDebugItem(LPRDATA rdPtr, int id)
{
#if !defined(RUN_ONLY)

	// Example
	// -------
/*
	switch (id)
	{
	case DB_CURRENTSTRING:
		{
			EditDebugInfo dbi;
			char buffer[256];

			dbi.pText=buffer;
			dbi.lText=TEXT_MAX;
			dbi.pTitle=NULL;

			strcpy(buffer, rdPtr->text);
			long ret=callRunTimeFunction(rdPtr, RFUNCTION_EDITTEXT, 0, (LPARAM)&dbi);
			if (ret)
				strcpy(rdPtr->text, dbi.pText);
		}
		break;
	case DB_CURRENTVALUE:
		{
			EditDebugInfo dbi;

			dbi.value=rdPtr->value;
			dbi.pTitle=NULL;

			long ret=callRunTimeFunction(rdPtr, RFUNCTION_EDITINT, 0, (LPARAM)&dbi);
			if (ret)
				rdPtr->value=dbi.value;
		}
		break;
	}
*/
#endif // !defined(RUN_ONLY)
}


