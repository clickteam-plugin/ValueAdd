#ifndef __DynExt_h__
#define __DynExt_h__

#pragma pack (push, 1)

// Make identifier
//#define MAKEID(a,b,c,d) ((#@a << 24)|(#@b << 16)|(#@c << 8)|(#@d)) 

#define IS_SET(flags, flag) (((flags) & (flag)) ? true : false)
#define PACK_FLOAT(f) *((long*)&(f))
#define UNPACK_FLOAT(f) *((float*)&(f))

////

template<class T>
inline T* PtrAddBytes(T *ptr, int bytes)
{
	return (T*)( (__int8*)ptr + bytes );
}

__inline double getAlterableValue(LPRO object, int ValueNum)
{
	rVal* object_rVal = &object->rov;

	if (object_rVal->rvpValues)
	{
		CValue & val = object_rVal->rvpValues[ValueNum];
		if (val.m_type == TYPE_LONG)
		{
			return (double)val.m_long;
		}
		else if (val.m_type == TYPE_DOUBLE)
		{
			return val.m_double;
		}
	}

	// unsupported version, value type or no alt values present.
	return 0;
}

/*struct AltVal
{
	long ValueType;
	long ValueFlags;
	union
	{
		long LongValue;
		double DoubleValue;
	};
};*/

/*__inline double getAlterableValue(LPRO object, int ValueNum)
{
	DWORD MMFVersion = object->roHo.hoAdRunHeader->rh4.rh4Mv->mvGetVersion();

	rVal* object_rVal = &object->rov;

	if (((MMFVersion & MMFVERSION_MASK) <= MMFVERSION_20) && ((MMFVersion & MMFBUILD_MASK) < 243))
	{
		if (object_rVal->rvValuesType[ValueNum] == TYPE_LONG)
		{
			return (double)object_rVal->rvValues[ValueNum];
		}
		else if (object_rVal->rvValuesType[ValueNum] == TYPE_FLOAT)
		{
			return (double)UNPACK_FLOAT(object_rVal->rvValues[ValueNum]);
		}
	}
	else if (((MMFVersion & MMFVERSION_MASK) >= MMFVERSION_20) && ((MMFVersion & MMFBUILD_MASK) >= 243))
	{
		if (object_rVal->rvValues[0])
		{
			AltVal & val = ((AltVal*)(object_rVal->rvValues[0]))[ValueNum];
			if (val.ValueType == TYPE_LONG)
			{
				return (double)val.LongValue;
			}
			else if (val.ValueType == TYPE_FLOAT)
			{
				return val.DoubleValue;
			}
		}
	}

	// unsupported version, value type or no alt values present.
	return 0;
}*/

////

enum Comparison
{
	PC_EQUAL            = CMPOPE_EQU,
	PC_DIFFERENT        = CMPOPE_DIF,
	PC_LOWER_OR_EQUAL   = CMPOPE_LOWEQU,
	PC_LOWER            = CMPOPE_LOW,
	PC_GREATER_OR_EQUAL = CMPOPE_GREEQU,
	PC_GREATER          = CMPOPE_GRE,
};

enum CompValueType
{
	PC_LONG = 0,
	PC_FLOAT = 23,
	PC_DOUBLE = 23,
};

struct ParamComp
{
//	Comparison compType:16;
	short compType;
	struct //CompValue
	{
		short unknown;
		short valueType;
		short valueSize;
		union
		{
			long longValue;
			struct
			{
				double doubleValue;
				float floatValue;
			};
		};
	}; //compValue;

	inline double GetDoubleValue()
	{
		if (valueType == PC_LONG)
		{
			return (double)longValue;
		}
		else if (valueType == PC_DOUBLE) // || PC_FLOAT, same thing
		{
			return doubleValue;
		}
		return 0.0;
	};
};

ParamComp* GetComparisonParameter(LPRDATA rdPtr)
{
	eventParam * CurrentParam = rdPtr->rHo.hoCurrentParam;
//	EVPNEXT(rdPtr->rHo.hoCurrentParam);
	switch (CurrentParam->evpCode)
	{
		case PARAM_COMPARAISON:
		{
			ParamComp* pData = (ParamComp*)(&(CurrentParam->evp));
			return pData;
		}
		break;
	}
	return NULL;
}

inline long Param_Comparison_true(Comparison comparison, long value) // Returns a value such that (return comparison value) is true
{
	switch(comparison)
	{
	case PC_EQUAL:
	case PC_LOWER_OR_EQUAL:
	case PC_GREATER_OR_EQUAL:
		return value;
	case PC_DIFFERENT:
	case PC_LOWER:
		return value - 1;
	case PC_GREATER:
		return value + 1;
	default:
		return 0; // ERROR!!!
	}
}

inline long Param_Comparison_false(Comparison comparison, long value) // Returns a value such that (return comparison value) is false
{
	switch(comparison)
	{
	case PC_DIFFERENT:
	case PC_LOWER:
	case PC_GREATER:
		return value;
	case PC_EQUAL:
	case PC_GREATER_OR_EQUAL:
		return value - 1;
	case PC_LOWER_OR_EQUAL:
		return value + 1;
	default:
		return 0; // ERROR!!!
	}
}

inline bool Param_Comparison_Test(Comparison comparison, double left, double right)
{
	switch(comparison)
	{
	case PC_EQUAL:
		return left == right;
	case PC_DIFFERENT:
		return left != right;
	case PC_LOWER_OR_EQUAL:
		return left <= right;
	case PC_LOWER:
		return left < right;
	case PC_GREATER_OR_EQUAL:
		return left >= right;
	case PC_GREATER:
		return left > right;
	default:
		return 0; // ERROR!!!
	}
}

////

/*double GetDoubleParameter(LPRDATA rdPtr)
{
	eventParam * CurrentParam = rdPtr->rHo.hoCurrentParam;
	EVPNEXT(rdPtr->rHo.hoCurrentParam);
	switch (CurrentParam->evpCode)
	{
		case PARAM_EXPRESSION:
		case PARAM_COMPARAISON:
		{
			ParamComp* pData = (ParamComp*)(&(CurrentParam->evp));
			return pData->GetDoubleValue();
		}
		break;
	}
	return 0.0;
}*/

////

__inline rCom* getrCom(LPRO object)
{
	DWORD OEFlags = object->roHo.hoOEFlags;
	if(!IS_SET(OEFlags, OEFLAG_MOVEMENTS) && !IS_SET(OEFlags, OEFLAG_ANIMATIONS))
		return 0;

	return (rCom*)((__int8*)object + sizeof(headerObject));
}

__inline rMvt* getrMvt(LPRO object)
{
	DWORD OEFlags = object->roHo.hoOEFlags;
	if(!IS_SET(OEFlags, OEFLAG_MOVEMENTS))
		return 0;

	return (rMvt*)((__int8*)object + sizeof(headerObject) + sizeof(rCom));
}

__inline rAni* getrAni(LPRO object)
{
	DWORD OEFlags = object->roHo.hoOEFlags;
	if(!IS_SET(OEFlags, OEFLAG_ANIMATIONS))
		return 0;

	return (rAni*)((__int8*)object + sizeof(headerObject) + sizeof(rCom) +
		(IS_SET(OEFlags, OEFLAG_MOVEMENTS) ? sizeof(rMvt) : 0));
}

__inline rSpr* getrSpr(LPRO object)
{
	DWORD OEFlags = object->roHo.hoOEFlags;
	if(!IS_SET(OEFlags, OEFLAG_SPRITES))
		return 0;

	return (rSpr*)((__int8*)object + sizeof(headerObject) +
		((IS_SET(OEFlags, OEFLAG_MOVEMENTS) ||
		 IS_SET(OEFlags, OEFLAG_ANIMATIONS)) ? sizeof(rCom) : 0) +
		(IS_SET(OEFlags, OEFLAG_MOVEMENTS) ? sizeof(rMvt) : 0) +
		(IS_SET(OEFlags, OEFLAG_ANIMATIONS) ? sizeof(rAni) : 0));
}

__inline rVal* getrVal(LPRO object)
{
	DWORD OEFlags = object->roHo.hoOEFlags;
	if(!IS_SET(OEFlags, OEFLAG_VALUES))
		return 0;

	return (rVal*)((__int8*)object + sizeof(headerObject) +
		((IS_SET(OEFlags, OEFLAG_MOVEMENTS) ||
		 IS_SET(OEFlags, OEFLAG_ANIMATIONS)) ? sizeof(rCom) : 0) +
		(IS_SET(OEFlags, OEFLAG_MOVEMENTS) ? sizeof(rMvt) : 0) +
		(IS_SET(OEFlags, OEFLAG_ANIMATIONS) ? sizeof(rAni) : 0) +
		(IS_SET(OEFlags, OEFLAG_SPRITES) ? sizeof(rSpr) : 0));
}

////
__inline rCom* getrCom(headerObject* object)
{
	DWORD OEFlags = object->hoOEFlags;
	if(!IS_SET(OEFlags, OEFLAG_MOVEMENTS) && !IS_SET(OEFlags, OEFLAG_ANIMATIONS))
		return 0;

	return (rCom*)((__int8*)object + sizeof(headerObject));
}

__inline rMvt* getrMvt(headerObject* object)
{
	DWORD OEFlags = object->hoOEFlags;
	if(!IS_SET(OEFlags, OEFLAG_MOVEMENTS))
		return 0;

	return (rMvt*)((__int8*)object + sizeof(headerObject) + sizeof(rCom));
}

__inline rAni* getrAni(headerObject* object)
{
	DWORD OEFlags = object->hoOEFlags;
	if(!IS_SET(OEFlags, OEFLAG_ANIMATIONS))
		return 0;

	return (rAni*)((__int8*)object + sizeof(headerObject) + sizeof(rCom) +
		(IS_SET(OEFlags, OEFLAG_MOVEMENTS) ? sizeof(rMvt) : 0));
}

__inline rSpr* getrSpr(headerObject* object)
{
	DWORD OEFlags = object->hoOEFlags;
	if(!IS_SET(OEFlags, OEFLAG_SPRITES))
		return 0;

	return (rSpr*)((__int8*)object + sizeof(headerObject) +
		((IS_SET(OEFlags, OEFLAG_MOVEMENTS) ||
		 IS_SET(OEFlags, OEFLAG_ANIMATIONS)) ? sizeof(rCom) : 0) +
		(IS_SET(OEFlags, OEFLAG_MOVEMENTS) ? sizeof(rMvt) : 0) +
		(IS_SET(OEFlags, OEFLAG_ANIMATIONS) ? sizeof(rAni) : 0));
}

__inline rVal* getrVal(headerObject* object)
{
	DWORD OEFlags = object->hoOEFlags;
	if(!IS_SET(OEFlags, OEFLAG_VALUES))
		return 0;

	return (rVal*)((__int8*)object + sizeof(headerObject) +
		((IS_SET(OEFlags, OEFLAG_MOVEMENTS) ||
		 IS_SET(OEFlags, OEFLAG_ANIMATIONS)) ? sizeof(rCom) : 0) +
		(IS_SET(OEFlags, OEFLAG_MOVEMENTS) ? sizeof(rMvt) : 0) +
		(IS_SET(OEFlags, OEFLAG_ANIMATIONS) ? sizeof(rAni) : 0) +
		(IS_SET(OEFlags, OEFLAG_SPRITES) ? sizeof(rSpr) : 0));
}

#pragma pack (pop)

#else
#ifdef WARNING_MULTI_INCLUDE
#pragma message(__FILE__ " included multiple times")
#endif
#endif
