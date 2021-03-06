// rSDK demonstration release - www.aquadasoft.com
// Jamie McLaughlin and Jean Villy Edberg

#include <hash_map>
#include <string>

using namespace std;
using namespace stdext;

// Send any queries or bug reports to support@aquadasoft.com

class rRundata;
typedef rRundata * LPRRDATA;

// --------------------------------
// RUNNING OBJECT DATA STRUCTURE
// --------------------------------
// If you want to store anything between actions/conditions/expressions
// you should store it here

typedef headerObject object;
typedef hash_map<string,float> float_vars;
typedef pair<string,float> float_var;
typedef hash_map<string,string> string_vars;
typedef pair<string,string> string_var;
struct variables
{
	float_vars floats;
	string_vars strings;
};

typedef hash_map<unsigned int,variables> variable_map;
typedef pair<unsigned int,variables> objvar_pair;
typedef struct tagRDATA
{
	#include "MagicRDATA.h"

	variable_map * pVariableMap;

} RUNDATA;
typedef	RUNDATA	* LPRDATA;

// --------------------------------
// EDITION OF OBJECT DATA STRUCTURE
// --------------------------------
// These values let you store data in your extension that will be saved in the MFA
// You should use these with properties

typedef struct tagEDATA_V1
{
	extHeader		eHeader;
//	short			swidth;
//	short			sheight;

} EDITDATA;
typedef EDITDATA * LPEDATA;
