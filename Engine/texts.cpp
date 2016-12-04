#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include <string.h>

#include "Engine/Engine.h"

#include "LOD.h"
#include "texts.h"


std::vector<char*> Tokenize( char* input, const char separator )
{
  std::vector<char*> retVect;
  retVect.push_back(input);
  while (*input)
  {
    if (*input == separator)
    {
        *input = '\0';
        retVect.push_back(input + 1);
    }
    ++input;
  }
  return retVect;
}



static char *pGlobalTXT_Raw=nullptr;

std::array<const char *, MAX_LOC_STRINGS> pGlobalTXT_LocalizationStrings;



int  sprintfex(char *buf, const char *format, ...)
{
  va_list args_ptr;
  va_start(args_ptr, format);
  {
    vsprintf(buf, format, args_ptr);
  }
  va_end(args_ptr);

  extern int  sprintfex_internal(char *buf);
  return sprintfex_internal(buf);
}

//----- (00452B95) --------------------------------------------------------
void  SetSomeItemsNames()
{
	pSomeItemsNames[0] = pGlobalTXT_LocalizationStrings[568];
	pSomeItemsNames[1] = pGlobalTXT_LocalizationStrings[271];
	pSomeItemsNames[2] = pGlobalTXT_LocalizationStrings[272];
	pSomeItemsNames[3] = pGlobalTXT_LocalizationStrings[273];
	pSomeItemsNames[4] = pGlobalTXT_LocalizationStrings[274];
	pSomeItemsNames[5] = pGlobalTXT_LocalizationStrings[275];
	pSomeItemsNames[6] = pGlobalTXT_LocalizationStrings[276];
	pSomeItemsNames[7] = pGlobalTXT_LocalizationStrings[277];
	pSomeItemsNames[8] = pGlobalTXT_LocalizationStrings[278];
	pSomeItemsNames[9] = pGlobalTXT_LocalizationStrings[279];
	pSomeItemsNames[10] = pGlobalTXT_LocalizationStrings[280];
	pSomeItemsNames[11] = pGlobalTXT_LocalizationStrings[281];
	pSomeItemsNames[12] = pGlobalTXT_LocalizationStrings[282];
	pSomeItemsNames[13] = pGlobalTXT_LocalizationStrings[143];
}

//----- (00452C30) --------------------------------------------------------
char *RemoveQuotes(char *pStr)
	{
	if ( *pStr == '"' )
		{
		pStr[strlen(pStr) - 1] = 0;
		return pStr + 1;
		}
	return pStr;
	}

//----- (00452C49) --------------------------------------------------------
void InitializeGameText()
	{
	//char *v0; // ebx@1
	/*char *v1; // eax@3
	int v2; // ebp@4
	char v3; // dl@5
	char *v4; // ecx@5
	int v5; // esi@9
	//char **v6; // [sp+10h] [bp-8h]@3
	signed int v7; // [sp+14h] [bp-4h]@4*/
	char *test_string; // eax@3
	char *tmp_pos; // eax@3
	int step; // ebp@4
	unsigned char c; // dl@5
	int temp_str_len; // ecx@5
	bool string_end; // [sp+14h] [bp-4h]@4
	//v0 = 0;
	free(pGlobalTXT_Raw);
	pGlobalTXT_Raw = (char *)pEvents_LOD->LoadRaw("global.txt", 0);

/*	strtok(pGlobalTXT_Raw, "\r");
	strtok(nullptr, "\r");
	v1 = strtok(nullptr, "\r") + 1;

	for (uint i = 0; i < 677; ++i)
		{
		char *p = v1;
		while (*++p != '\t' && *p);

		pGlobalTXT_LocalizationStrings[i] = RemoveQuotes(p + 1);

		v1 = strtok(nullptr, "\r") + 1;
		}*/
	strtok((char *)pGlobalTXT_Raw, "\r");	   //Carriage Return 0D 13
	strtok(NULL, "\r");					   	  //Carriage Return 0D 13

	for (int i = 0; i < MAX_LOC_STRINGS; ++i)
  {
    test_string = strtok(NULL, "\r") + 1;	 //Carriage Return 0D 13
		step = 0;
		string_end = false;
		do
			{
			c = *(unsigned char*)test_string;
			temp_str_len = 0;
			if (c!= '\t' )	// Horizontal tab 09 
				{
				do
					{
					if ( !c )
						break; 
					c = *(test_string+temp_str_len+1);
					temp_str_len++;    
					}
				while ( c != '\t' );	// Horizontal tab 09 
				}
			tmp_pos=test_string+temp_str_len;
			if (*tmp_pos == 0)
				string_end = true;
			*tmp_pos = 0;
			if ( temp_str_len == 0)
				string_end = true;
			else
				{
				if ( step == 1 )
					pGlobalTXT_LocalizationStrings[i] = RemoveQuotes(test_string);
				}
			++step;
			test_string=tmp_pos+1;
			}
		while ( step <= 2 && !string_end);  
		}	

  SetSomeItemsNames();
	aMonthNames[0] = pGlobalTXT_LocalizationStrings[415];  aMonthNames[1] = pGlobalTXT_LocalizationStrings[416];
	aMonthNames[2] = pGlobalTXT_LocalizationStrings[417];  aMonthNames[3] = pGlobalTXT_LocalizationStrings[418];
	aMonthNames[4] = pGlobalTXT_LocalizationStrings[419];  aMonthNames[5] = pGlobalTXT_LocalizationStrings[420];
	aMonthNames[6] = pGlobalTXT_LocalizationStrings[421];  aMonthNames[7] = pGlobalTXT_LocalizationStrings[422];
	aMonthNames[8] = pGlobalTXT_LocalizationStrings[423];  aMonthNames[9] = pGlobalTXT_LocalizationStrings[424];
	aMonthNames[10] = pGlobalTXT_LocalizationStrings[425]; aMonthNames[11] = pGlobalTXT_LocalizationStrings[426];

	aDayNames[0] = pGlobalTXT_LocalizationStrings[145];  aDayNames[1] = pGlobalTXT_LocalizationStrings[230];
	aDayNames[2] = pGlobalTXT_LocalizationStrings[243];  aDayNames[3] = pGlobalTXT_LocalizationStrings[227];
	aDayNames[4] = pGlobalTXT_LocalizationStrings[91];   aDayNames[5] = pGlobalTXT_LocalizationStrings[188];
	aDayNames[6] = pGlobalTXT_LocalizationStrings[222];

	
	pClassNames [0] = pGlobalTXT_LocalizationStrings[LOCSTR_KNIGHT];
	pClassNames [1] = pGlobalTXT_LocalizationStrings[LOCSTR_CAVALIER];
	pClassNames [2] = pGlobalTXT_LocalizationStrings[LOCSTR_CHAMPION];
	pClassNames [3] = pGlobalTXT_LocalizationStrings[LOCSTR_BLK_KNIGHT];

	pClassNames [4] = pGlobalTXT_LocalizationStrings[LOCSTR_THIEF];
	pClassNames [5] = pGlobalTXT_LocalizationStrings[LOCSTR_ROGUE];
	pClassNames [6] = pGlobalTXT_LocalizationStrings[LOCSTR_SPY];
	pClassNames [7] = pGlobalTXT_LocalizationStrings[LOCSTR_ASSASIN];

	pClassNames [8] = pGlobalTXT_LocalizationStrings[LOCSTR_MONK];
	pClassNames [9] = pGlobalTXT_LocalizationStrings[26];
	pClassNames [10] = pGlobalTXT_LocalizationStrings[432];
	pClassNames [11] = pGlobalTXT_LocalizationStrings[27];

	pClassNames [12] = pGlobalTXT_LocalizationStrings[LOCSTR_PALADIN];
	pClassNames [13] = pGlobalTXT_LocalizationStrings[263];
	pClassNames [14] = pGlobalTXT_LocalizationStrings[264];
	pClassNames [15] = pGlobalTXT_LocalizationStrings[28];

	pClassNames [16] = pGlobalTXT_LocalizationStrings[LOCSTR_ARCHER];
	pClassNames [17] = pGlobalTXT_LocalizationStrings[267];
	pClassNames [18] = pGlobalTXT_LocalizationStrings[119];
	pClassNames [19] = pGlobalTXT_LocalizationStrings[124];

	pClassNames [20] = pGlobalTXT_LocalizationStrings[LOCSTR_RANGER];
	pClassNames [21] = pGlobalTXT_LocalizationStrings[370];
	pClassNames [22] = pGlobalTXT_LocalizationStrings[33];
	pClassNames [23] = pGlobalTXT_LocalizationStrings[40];

	pClassNames [24] = pGlobalTXT_LocalizationStrings[LOCSTR_CLERIC];
	pClassNames [25] = pGlobalTXT_LocalizationStrings[257];
	pClassNames [26] = pGlobalTXT_LocalizationStrings[44];
	pClassNames [27] = pGlobalTXT_LocalizationStrings[46];

	pClassNames [28] = pGlobalTXT_LocalizationStrings[LOCSTR_DRUID];
	pClassNames [29] = pGlobalTXT_LocalizationStrings[269];
	pClassNames [30] = pGlobalTXT_LocalizationStrings[270];
	pClassNames [31] = pGlobalTXT_LocalizationStrings[48];

	pClassNames [32] = pGlobalTXT_LocalizationStrings[LOCSTR_SORCERER];
	pClassNames [33] = pGlobalTXT_LocalizationStrings[260];
	pClassNames [34] = pGlobalTXT_LocalizationStrings[261];
	pClassNames [35] = pGlobalTXT_LocalizationStrings[49];

	aCharacterConditionNames[0] = pGlobalTXT_LocalizationStrings[52];
	aCharacterConditionNames[1] = pGlobalTXT_LocalizationStrings[241];
	aCharacterConditionNames[2] = pGlobalTXT_LocalizationStrings[14];
	aCharacterConditionNames[3] = pGlobalTXT_LocalizationStrings[4];
	aCharacterConditionNames[4] = pGlobalTXT_LocalizationStrings[69];
	aCharacterConditionNames[5] = pGlobalTXT_LocalizationStrings[117];
	aCharacterConditionNames[6] = pGlobalTXT_LocalizationStrings[166];
	aCharacterConditionNames[7] = pGlobalTXT_LocalizationStrings[65];
	aCharacterConditionNames[8] = pGlobalTXT_LocalizationStrings[166];
	aCharacterConditionNames[9] = pGlobalTXT_LocalizationStrings[65];
	aCharacterConditionNames[10] = pGlobalTXT_LocalizationStrings[166];
	aCharacterConditionNames[11] = pGlobalTXT_LocalizationStrings[65];
	aCharacterConditionNames[12] = pGlobalTXT_LocalizationStrings[162];
	aCharacterConditionNames[13] = pGlobalTXT_LocalizationStrings[231];
	aCharacterConditionNames[14] = pGlobalTXT_LocalizationStrings[58];
	aCharacterConditionNames[15] = pGlobalTXT_LocalizationStrings[220];
	aCharacterConditionNames[16] = pGlobalTXT_LocalizationStrings[76];
	aCharacterConditionNames[17] = pGlobalTXT_LocalizationStrings[601];
	aCharacterConditionNames[18] = pGlobalTXT_LocalizationStrings[98];

	pSkillNames[0] = pGlobalTXT_LocalizationStrings[271];
	pSkillNames[1] = pGlobalTXT_LocalizationStrings[272];
	pSkillNames[2] = pGlobalTXT_LocalizationStrings[273];
	pSkillNames[3] = pGlobalTXT_LocalizationStrings[274];
	pSkillNames[4] = pGlobalTXT_LocalizationStrings[275];
	pSkillNames[5] = pGlobalTXT_LocalizationStrings[276];
	pSkillNames[6] = pGlobalTXT_LocalizationStrings[277];
	pSkillNames[7] = pGlobalTXT_LocalizationStrings[278];
	pSkillNames[8] = pGlobalTXT_LocalizationStrings[279];
	pSkillNames[9] = pGlobalTXT_LocalizationStrings[280];
	pSkillNames[10] = pGlobalTXT_LocalizationStrings[281];
	pSkillNames[11] = pGlobalTXT_LocalizationStrings[282];
	pSkillNames[12] = pGlobalTXT_LocalizationStrings[283];
	pSkillNames[13] = pGlobalTXT_LocalizationStrings[284];
	pSkillNames[14] = pGlobalTXT_LocalizationStrings[285];
	pSkillNames[15] = pGlobalTXT_LocalizationStrings[286];
	pSkillNames[16] = pGlobalTXT_LocalizationStrings[289];
	pSkillNames[17] = pGlobalTXT_LocalizationStrings[290];
	pSkillNames[18] = pGlobalTXT_LocalizationStrings[291];
	pSkillNames[19] = pGlobalTXT_LocalizationStrings[287];
	pSkillNames[20] = pGlobalTXT_LocalizationStrings[288];
	pSkillNames[21] = pGlobalTXT_LocalizationStrings[292];
	pSkillNames[22] = pGlobalTXT_LocalizationStrings[293];
	pSkillNames[23] = pGlobalTXT_LocalizationStrings[294];
	pSkillNames[24] = pGlobalTXT_LocalizationStrings[295];
	pSkillNames[25] = pGlobalTXT_LocalizationStrings[296];
	pSkillNames[26] = pGlobalTXT_LocalizationStrings[297];
	pSkillNames[27] = pGlobalTXT_LocalizationStrings[298];
	pSkillNames[28] = pGlobalTXT_LocalizationStrings[299];
	pSkillNames[29] = pGlobalTXT_LocalizationStrings[300];
	pSkillNames[30] = pGlobalTXT_LocalizationStrings[50];
	pSkillNames[31] = pGlobalTXT_LocalizationStrings[77];
	pSkillNames[32] = pGlobalTXT_LocalizationStrings[88];
	pSkillNames[33] = pGlobalTXT_LocalizationStrings[89];
	pSkillNames[34] = pGlobalTXT_LocalizationStrings[90];
	pSkillNames[35] = pGlobalTXT_LocalizationStrings[95];
	pSkillNames[36] = pGlobalTXT_LocalizationStrings[301];
	pSkillNames[37] = pGlobalTXT_LocalizationStrings[153];

	aSpellSchoolNames[0] = pGlobalTXT_LocalizationStrings[87];
	aSpellSchoolNames[1] = pGlobalTXT_LocalizationStrings[6];
	aSpellSchoolNames[2] = pGlobalTXT_LocalizationStrings[240];
	aSpellSchoolNames[3] = pGlobalTXT_LocalizationStrings[70];
	aSpellSchoolNames[4] = pGlobalTXT_LocalizationStrings[214];
	aSpellSchoolNames[5] = pGlobalTXT_LocalizationStrings[142];
	aSpellSchoolNames[6] = pGlobalTXT_LocalizationStrings[29];
	aSpellSchoolNames[7] = pGlobalTXT_LocalizationStrings[133];
	aSpellSchoolNames[8] = pGlobalTXT_LocalizationStrings[54];

	aAttributeNames[0] = pGlobalTXT_LocalizationStrings[144];
	aAttributeNames[1] = pGlobalTXT_LocalizationStrings[116];
	aAttributeNames[2] = pGlobalTXT_LocalizationStrings[163];
	aAttributeNames[3] = pGlobalTXT_LocalizationStrings[75];
	aAttributeNames[4] = pGlobalTXT_LocalizationStrings[1];
	aAttributeNames[5] = pGlobalTXT_LocalizationStrings[211];
	aAttributeNames[6] = pGlobalTXT_LocalizationStrings[136];

	aAMPMNames[0] = pGlobalTXT_LocalizationStrings[472];
	aAMPMNames[1] = pGlobalTXT_LocalizationStrings[473];

	aNPCProfessionNames[0] = pGlobalTXT_LocalizationStrings[153];
	aNPCProfessionNames[1] = pGlobalTXT_LocalizationStrings[308];
	aNPCProfessionNames[2] = pGlobalTXT_LocalizationStrings[309];
	aNPCProfessionNames[3] = pGlobalTXT_LocalizationStrings[7];
	aNPCProfessionNames[4] = pGlobalTXT_LocalizationStrings[306];
	aNPCProfessionNames[5] = pGlobalTXT_LocalizationStrings[310];
	aNPCProfessionNames[6] = pGlobalTXT_LocalizationStrings[311];
	aNPCProfessionNames[7] = pGlobalTXT_LocalizationStrings[312];
	aNPCProfessionNames[8] = pGlobalTXT_LocalizationStrings[313];
	aNPCProfessionNames[9] = pGlobalTXT_LocalizationStrings[314];
	aNPCProfessionNames[10] = pGlobalTXT_LocalizationStrings[105];
	aNPCProfessionNames[11] = pGlobalTXT_LocalizationStrings[315];
	aNPCProfessionNames[12] = pGlobalTXT_LocalizationStrings[316];
	aNPCProfessionNames[13] = pGlobalTXT_LocalizationStrings[317];
	aNPCProfessionNames[14] = pGlobalTXT_LocalizationStrings[115];
	aNPCProfessionNames[15] = pGlobalTXT_LocalizationStrings[318];
	aNPCProfessionNames[16] = pGlobalTXT_LocalizationStrings[319];
	aNPCProfessionNames[17] = pGlobalTXT_LocalizationStrings[320];
	aNPCProfessionNames[18] = pGlobalTXT_LocalizationStrings[321];
	aNPCProfessionNames[19] = pGlobalTXT_LocalizationStrings[322];
	aNPCProfessionNames[20] = pGlobalTXT_LocalizationStrings[323];
	aNPCProfessionNames[21] = pGlobalTXT_LocalizationStrings[293];
	aNPCProfessionNames[22] = pGlobalTXT_LocalizationStrings[324];
	aNPCProfessionNames[23] = pGlobalTXT_LocalizationStrings[498];
	aNPCProfessionNames[24] = pGlobalTXT_LocalizationStrings[525];
	aNPCProfessionNames[25] = pGlobalTXT_LocalizationStrings[327];
	aNPCProfessionNames[26] = pGlobalTXT_LocalizationStrings[328];
	aNPCProfessionNames[27] = pGlobalTXT_LocalizationStrings[329];
	aNPCProfessionNames[28] = pGlobalTXT_LocalizationStrings[330];
	aNPCProfessionNames[29] = pGlobalTXT_LocalizationStrings[331];
	aNPCProfessionNames[30] = pGlobalTXT_LocalizationStrings[332];
	aNPCProfessionNames[31] = pGlobalTXT_LocalizationStrings[333];
	aNPCProfessionNames[32] = pGlobalTXT_LocalizationStrings[334];
	aNPCProfessionNames[33] = pGlobalTXT_LocalizationStrings[335];
	aNPCProfessionNames[34] = pGlobalTXT_LocalizationStrings[336];
	aNPCProfessionNames[35] = pGlobalTXT_LocalizationStrings[337];
	aNPCProfessionNames[36] = pGlobalTXT_LocalizationStrings[338];
	aNPCProfessionNames[37] = pGlobalTXT_LocalizationStrings[339];
	aNPCProfessionNames[38] = pGlobalTXT_LocalizationStrings[340];
	aNPCProfessionNames[39] = pGlobalTXT_LocalizationStrings[341];
	aNPCProfessionNames[40] = pGlobalTXT_LocalizationStrings[342];
	aNPCProfessionNames[41] = pGlobalTXT_LocalizationStrings[343];
	aNPCProfessionNames[42] = pGlobalTXT_LocalizationStrings[596];
	aNPCProfessionNames[43] = pGlobalTXT_LocalizationStrings[345];
	aNPCProfessionNames[44] = pGlobalTXT_LocalizationStrings[346];
	aNPCProfessionNames[45] = pGlobalTXT_LocalizationStrings[347];
	aNPCProfessionNames[46] = pGlobalTXT_LocalizationStrings[348];
	aNPCProfessionNames[47] = pGlobalTXT_LocalizationStrings[349];  
	aNPCProfessionNames[48] = pGlobalTXT_LocalizationStrings[350];
	aNPCProfessionNames[49] = pGlobalTXT_LocalizationStrings[597];
	aNPCProfessionNames[50] = pGlobalTXT_LocalizationStrings[352];
	aNPCProfessionNames[51] = pGlobalTXT_LocalizationStrings[353];
	aNPCProfessionNames[52] = pGlobalTXT_LocalizationStrings[598];
	aNPCProfessionNames[53] = pGlobalTXT_LocalizationStrings[344];
	aNPCProfessionNames[54] = pGlobalTXT_LocalizationStrings[26];
	aNPCProfessionNames[55] = pGlobalTXT_LocalizationStrings[599];
	aNPCProfessionNames[56] = pGlobalTXT_LocalizationStrings[21];
	aNPCProfessionNames[57] = pGlobalTXT_LocalizationStrings[600];
	aNPCProfessionNames[58] = pGlobalTXT_LocalizationStrings[370];

	aMoonPhaseNames[0] = pGlobalTXT_LocalizationStrings[150];
	aMoonPhaseNames[1] = pGlobalTXT_LocalizationStrings[171];
	aMoonPhaseNames[2] = pGlobalTXT_LocalizationStrings[102];
	aMoonPhaseNames[3] = pGlobalTXT_LocalizationStrings[169];
	aMoonPhaseNames[4] = pGlobalTXT_LocalizationStrings[92];

	aSpellNames[0] = pGlobalTXT_LocalizationStrings[202];
	aSpellNames[1] = pGlobalTXT_LocalizationStrings[204];
	aSpellNames[2] = pGlobalTXT_LocalizationStrings[219];
	aSpellNames[3] = pGlobalTXT_LocalizationStrings[215];
	aSpellNames[4] = pGlobalTXT_LocalizationStrings[208];
	aSpellNames[5] = pGlobalTXT_LocalizationStrings[454];
	aSpellNames[6] = pGlobalTXT_LocalizationStrings[24];
	aSpellNames[7] = pGlobalTXT_LocalizationStrings[455];
	aSpellNames[8] = pGlobalTXT_LocalizationStrings[441];
	aSpellNames[9] = pGlobalTXT_LocalizationStrings[440];
	aSpellNames[10] = pGlobalTXT_LocalizationStrings[218];
	aSpellNames[11] = pGlobalTXT_LocalizationStrings[217];
	aSpellNames[12] = pGlobalTXT_LocalizationStrings[213];
	aSpellNames[13] = pGlobalTXT_LocalizationStrings[462];
	aSpellNames[14] = pGlobalTXT_LocalizationStrings[279];
	aSpellNames[15] = pGlobalTXT_LocalizationStrings[442];
	aSpellNames[16] = pGlobalTXT_LocalizationStrings[452];
	aSpellNames[17] = pGlobalTXT_LocalizationStrings[194];
	aSpellNames[18] = pGlobalTXT_LocalizationStrings[456];
	aSpellNames[19] = pGlobalTXT_LocalizationStrings[453];
	aSpellNames[20] = pGlobalTXT_LocalizationStrings[202];
	aSpellNames[21] = pGlobalTXT_LocalizationStrings[443];
	aSpellNames[22] = pGlobalTXT_LocalizationStrings[204];
	aSpellNames[23] = pGlobalTXT_LocalizationStrings[208];
	aSpellNames[24] = pGlobalTXT_LocalizationStrings[221];
	aSpellNames[25] = pGlobalTXT_LocalizationStrings[24];
	aSpellNames[26] = pGlobalTXT_LocalizationStrings[228];
	aSpellNames[27] = pGlobalTXT_LocalizationStrings[441];
	aSpellNames[28] = pGlobalTXT_LocalizationStrings[440];
	aSpellNames[29] = pGlobalTXT_LocalizationStrings[213];
	aSpellNames[30] = pGlobalTXT_LocalizationStrings[229];
	aSpellNames[31] = pGlobalTXT_LocalizationStrings[233];
	aSpellNames[32] = pGlobalTXT_LocalizationStrings[234];
	aSpellNames[33] = pGlobalTXT_LocalizationStrings[279];
	aSpellNames[34] = pGlobalTXT_LocalizationStrings[442];
	aSpellNames[35] = pGlobalTXT_LocalizationStrings[235];
	aSpellNames[36] = pGlobalTXT_LocalizationStrings[246];
	aSpellNames[37] = pGlobalTXT_LocalizationStrings[247];
	aSpellNames[38] = pGlobalTXT_LocalizationStrings[248];
	aSpellNames[39] = pGlobalTXT_LocalizationStrings[674];
	aSpellNames[40] = pGlobalTXT_LocalizationStrings[249];
	aSpellNames[41] = pGlobalTXT_LocalizationStrings[258];
	aSpellNames[42] = pGlobalTXT_LocalizationStrings[194];
	aSpellNames[43] = pGlobalTXT_LocalizationStrings[657];
	}
//----- (00413FF1) --------------------------------------------------------
void SetMonthNames()
{
  aMonthNames[0] = pGlobalTXT_LocalizationStrings[415];
  aMonthNames[1] = pGlobalTXT_LocalizationStrings[416];
  aMonthNames[2] = pGlobalTXT_LocalizationStrings[417];
  aMonthNames[3] = pGlobalTXT_LocalizationStrings[418];
  aMonthNames[4] = pGlobalTXT_LocalizationStrings[419];
  aMonthNames[5] = pGlobalTXT_LocalizationStrings[420];
  aMonthNames[6] = pGlobalTXT_LocalizationStrings[421];
  aMonthNames[7] = pGlobalTXT_LocalizationStrings[422];
  aMonthNames[8] = pGlobalTXT_LocalizationStrings[423];
  aMonthNames[9] = pGlobalTXT_LocalizationStrings[424];
  aMonthNames[10] = pGlobalTXT_LocalizationStrings[425];
  aMonthNames[11] = pGlobalTXT_LocalizationStrings[426];
}

//----- (0041406F) --------------------------------------------------------
void SetDayNames()
{
  aDayNames[0] = pGlobalTXT_LocalizationStrings[145];
  aDayNames[1] = pGlobalTXT_LocalizationStrings[230];
  aDayNames[2] = pGlobalTXT_LocalizationStrings[243];
  aDayNames[3] = pGlobalTXT_LocalizationStrings[227];
  aDayNames[4] = pGlobalTXT_LocalizationStrings[91];
  aDayNames[5] = pGlobalTXT_LocalizationStrings[188];
  aDayNames[6] = pGlobalTXT_LocalizationStrings[222];
}

//----- (004140BB) --------------------------------------------------------
void SetSpellSchoolNames()
{
  aSpellSchoolNames[0] = pGlobalTXT_LocalizationStrings[87];
  aSpellSchoolNames[1] = pGlobalTXT_LocalizationStrings[6];
  aSpellSchoolNames[2] = pGlobalTXT_LocalizationStrings[240];
  aSpellSchoolNames[3] = pGlobalTXT_LocalizationStrings[70];
  aSpellSchoolNames[4] = pGlobalTXT_LocalizationStrings[214];
  aSpellSchoolNames[5] = pGlobalTXT_LocalizationStrings[142];
  aSpellSchoolNames[6] = pGlobalTXT_LocalizationStrings[29];
  aSpellSchoolNames[7] = pGlobalTXT_LocalizationStrings[133];
  aSpellSchoolNames[8] = pGlobalTXT_LocalizationStrings[54];
}

//----- (0041411B) --------------------------------------------------------
void SetAttributeNames()
{
  aAttributeNames[0] = pGlobalTXT_LocalizationStrings[144];
  aAttributeNames[1] = pGlobalTXT_LocalizationStrings[116];
  aAttributeNames[2] = pGlobalTXT_LocalizationStrings[163];
  aAttributeNames[3] = pGlobalTXT_LocalizationStrings[75];
  aAttributeNames[4] = pGlobalTXT_LocalizationStrings[1];
  aAttributeNames[5] = pGlobalTXT_LocalizationStrings[211];
  aAttributeNames[6] = pGlobalTXT_LocalizationStrings[136];
}
//----- (00410AF5) --------------------------------------------------------
void SetMoonPhaseNames()
{
  aMoonPhaseNames[0] = pGlobalTXT_LocalizationStrings[150];
  aMoonPhaseNames[1] = pGlobalTXT_LocalizationStrings[171];
  aMoonPhaseNames[2] = pGlobalTXT_LocalizationStrings[102];
  aMoonPhaseNames[3] = pGlobalTXT_LocalizationStrings[169];
  aMoonPhaseNames[4] = pGlobalTXT_LocalizationStrings[92];
}


