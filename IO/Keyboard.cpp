#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <crtdbg.h>

#include "Engine/Engine.h"
#include "Engine/Party.h"
#include "Engine/Time.h"
#include "Engine/Objects/Actor.h"

#include "Engine/TurnEngine/TurnEngine.h"

#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/Indoor.h"

#include "Engine/Spells/CastSpellInfo.h"

#include "IO/Keyboard.h"

#include "GUI/GUIWindow.h"

#include <tuple>
#include <vector>
#include <string>

struct KeyboardActionMapping *pKeyActionMap;


class CKeyListElement
{
public:
  std::string m_keyName;
  unsigned char m_keyDefaultCode;
  unsigned short m_cmdId;
  KeyToggleType m_toggType;
  CKeyListElement(std::string keyName, unsigned char keyDefaultCode, unsigned short cmdId, KeyToggleType toggType):
    m_keyName(keyName),
    m_keyDefaultCode(keyDefaultCode),
    m_cmdId(cmdId),
    m_toggType(toggType)
  {

  }
};

std::array<CKeyListElement, 30>keyMappingParams = {
  CKeyListElement("KEY_FORWARD", VK_UP, INPUT_MoveForward, TOGGLE_Continuously),
  CKeyListElement("KEY_BACKWARD", VK_DOWN, INPUT_MoveBackwards, TOGGLE_Continuously),
  CKeyListElement("KEY_LEFT", VK_LEFT, INPUT_TurnLeft, TOGGLE_Continuously),
  CKeyListElement("KEY_RIGHT", VK_RIGHT, INPUT_TurnRight, TOGGLE_Continuously),
  CKeyListElement("KEY_ATTACK", 'A', INPUT_Attack, TOGGLE_OneTimePress),
  CKeyListElement("KEY_CASTREADY", 'S', INPUT_CastReady, TOGGLE_OneTimePress),
  CKeyListElement("KEY_YELL", 'Y', INPUT_Yell, TOGGLE_OneTimePress),
  CKeyListElement("KEY_JUMP", 'X', INPUT_Jump, TOGGLE_OneTimePress),
  CKeyListElement("KEY_COMBAT", VK_RETURN, INPUT_Combat, TOGGLE_OneTimePress),
  CKeyListElement("KEY_EVENTTRIGGER", VK_SPACE, INPUT_EventTrigger, TOGGLE_OneTimePress),
  CKeyListElement("KEY_CAST", 'C', INPUT_Cast, TOGGLE_OneTimePress),
  CKeyListElement("KEY_PASS", 'B', INPUT_Pass, TOGGLE_OneTimePress),
  CKeyListElement("KEY_CHARCYCLE", VK_TAB, INPUT_CharCycle, TOGGLE_OneTimePress),
  CKeyListElement("KEY_QUEST", 'Q', INPUT_Quest, TOGGLE_OneTimePress),
  CKeyListElement("KEY_QUICKREF", 'Z', INPUT_QuickRef, TOGGLE_OneTimePress),
  CKeyListElement("KEY_REST", 'R', INPUT_Rest, TOGGLE_OneTimePress),
  CKeyListElement("KEY_TIMECAL", 'T', INPUT_TimeCal, TOGGLE_OneTimePress),
  CKeyListElement("KEY_AUTONOTES", 'N', INPUT_Autonotes, TOGGLE_OneTimePress),
  CKeyListElement("KEY_MAPBOOK", 'M', INPUT_Mapbook, TOGGLE_OneTimePress),
  CKeyListElement("KEY_LOOKUP", VK_NEXT, INPUT_LookUp, TOGGLE_OneTimePress),
  CKeyListElement("KEY_LOOKDOWN", VK_DELETE, INPUT_LookDown, TOGGLE_OneTimePress),
  CKeyListElement("KEY_CENTERVIEWPT", VK_END, INPUT_CenterView, TOGGLE_OneTimePress),
  CKeyListElement("KEY_ZOOMIN", VK_ADD, INPUT_ZoomIn, TOGGLE_OneTimePress),
  CKeyListElement("KEY_ZOOMOUT", VK_SUBTRACT, INPUT_ZoomOut, TOGGLE_OneTimePress),
  CKeyListElement("KEY_FLYUP", VK_PRIOR, INPUT_FlyUp, TOGGLE_Continuously),
  CKeyListElement("KEY_FLYDOWN", VK_INSERT, INPUT_FlyDown, TOGGLE_Continuously),
  CKeyListElement("KEY_LAND", VK_HOME, INPUT_Land, TOGGLE_OneTimePress),
  CKeyListElement("KEY_ALWAYSRUN", 'U', INPUT_AlwaysRun, TOGGLE_OneTimePress),
  CKeyListElement("KEY_STEPLEFT", VK_OEM_4, INPUT_StrafeLeft, TOGGLE_Continuously),
  CKeyListElement("KEY_STEPRIGHT", VK_OEM_6, INPUT_StrafeRight, TOGGLE_Continuously)
}; 

std::array<std::tuple<const char*, const unsigned __int8>, 26> keyNameToCodeTranslationMap =
{
  std::tuple<const char*, const unsigned __int8>("UP", VK_UP),
  std::tuple<const char*, const unsigned __int8>("DOWN", VK_DOWN),
  std::tuple<const char*, const unsigned __int8>("LEFT", VK_LEFT),
  std::tuple<const char*, const unsigned __int8>("бкебн", VK_LEFT),
  std::tuple<const char*, const unsigned __int8>("RIGHT", VK_RIGHT),
  std::tuple<const char*, const unsigned __int8>("бопюбн", VK_RIGHT),
  std::tuple<const char*, const unsigned __int8>("RETURN", VK_RETURN),
  std::tuple<const char*, const unsigned __int8>("SPACE", VK_SPACE),
  std::tuple<const char*, const unsigned __int8>("PAGE_DOWN", VK_NEXT),
  std::tuple<const char*, const unsigned __int8>("PAGE_UP", VK_PRIOR),
  std::tuple<const char*, const unsigned __int8>("TAB", VK_TAB),
  std::tuple<const char*, const unsigned __int8>("SUBTRACT", VK_SUBTRACT),
  std::tuple<const char*, const unsigned __int8>("ADD", VK_ADD),
  std::tuple<const char*, const unsigned __int8>("END", VK_END),
  std::tuple<const char*, const unsigned __int8>("DELETE", VK_DELETE),
  std::tuple<const char*, const unsigned __int8>("HOME", VK_HOME),
  std::tuple<const char*, const unsigned __int8>("INSERT", VK_INSERT),
  std::tuple<const char*, const unsigned __int8>("COMMA", VK_OEM_COMMA),
  std::tuple<const char*, const unsigned __int8>("DECIMAL", VK_DECIMAL),
  std::tuple<const char*, const unsigned __int8>("SEMICOLON", VK_OEM_1),
  std::tuple<const char*, const unsigned __int8>("PERIOD", VK_OEM_PERIOD),
  std::tuple<const char*, const unsigned __int8>("SLASH", VK_OEM_2),
  std::tuple<const char*, const unsigned __int8>("SQUOTE", VK_OEM_7),
  std::tuple<const char*, const unsigned __int8>("BACKSLASH", VK_OEM_5),
  std::tuple<const char*, const unsigned __int8>("BACKSPACE", VK_BACK),
  std::tuple<const char*, const unsigned __int8>("CONTROL", VK_CONTROL),
};

//----- (00459C68) --------------------------------------------------------
void KeyboardActionMapping::SetKeyMapping(int uAction, int vKey, KeyToggleType type)
{
  pVirtualKeyCodesMapping[uAction] = vKey;
  pToggleTypes[uAction] = type;
}

//----- (00459C82) --------------------------------------------------------
unsigned int KeyboardActionMapping::GetActionVKey(enum InputAction eAction)
{
  return this->pVirtualKeyCodesMapping[eAction];
}

//----- (00459C8D) --------------------------------------------------------
KeyboardActionMapping::KeyboardActionMapping()
{
  uLastKeyPressed = 0;
  field_204 = 0;
  pWindow = nullptr;

  SetDefaultMapping();
  ReadMappings();

  ResetKeys();

  uNumKeysPressed = 0;

  uGameMenuUI_CurentlySelectedKeyIdx = -1;
}
// 506E68: using guessed type int uGameMenuUI_CurentlySelectedKeyIdx;

//----- (00459CC4) --------------------------------------------------------
void KeyboardActionMapping::SetDefaultMapping()
{
  for ( size_t i = 0; i < keyMappingParams.size(); i++)
  {
    SetKeyMapping(keyMappingParams[i].m_cmdId, keyMappingParams[i].m_keyDefaultCode, keyMappingParams[i].m_toggType);
  }
}

//----- (00459E3F) --------------------------------------------------------
void KeyboardActionMapping::ResetKeys()
{
  for (uint i = 0; i < 30; ++i)
    GetAsyncKeyState(pVirtualKeyCodesMapping[i]);
}

//----- (00459E5A) --------------------------------------------------------
void KeyboardActionMapping::EnterText(int a2, int max_string_len, GUIWindow *pWindow)
{
  memset(this->pPressedKeysBuffer, 0, 0x101u);
  this->uNumKeysPressed = 0;
  if ( a2 )
    this->field_204 = 2;
  else
    this->field_204 = 1;
  this->max_input_string_len = max_string_len;
  this->pWindow = pWindow;
  pWindow->receives_keyboard_input_2 = WINDOW_INPUT_IN_PROGRESS;
}

//----- (00459ED1) --------------------------------------------------------
void KeyboardActionMapping::SetWindowInputStatus(int a2)
{
  field_204 = 0;
  if ( pWindow )
    pWindow->receives_keyboard_input_2 = a2;
}

//----- (00459F10) --------------------------------------------------------
bool KeyboardActionMapping::ProcessTextInput(unsigned int a2)
{
  pKeyActionMap->uLastKeyPressed = a2;
  if ( uGameMenuUI_CurentlySelectedKeyIdx == -1 )
  {
    if ( pKeyActionMap->field_204 != 1 && pKeyActionMap->field_204 != 2 )
      return 0;
    if ( a2 == VK_BACK)
    {
      if (pKeyActionMap->uNumKeysPressed > 0)
      {
        --pKeyActionMap->uNumKeysPressed;
        pKeyActionMap->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] = 0;
      }
    }
    else if ( a2 == VK_RETURN )
      pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_CONFIRMED);
    else if ( a2 == VK_ESCAPE )
      pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_CANCELLED);
    else if (this->uNumKeysPressed < this->max_input_string_len)
    {
      if ( pKeyActionMap->field_204 == 1 )
      {
        if ( a2 != VK_TAB )
        {
          pKeyActionMap->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] = a2;
          ++pKeyActionMap->uNumKeysPressed;
          pKeyActionMap->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] = 0;
        }
      }
      else if (pKeyActionMap->field_204 == 2)
      {
        if ( isdigit(a2))
        {
          pKeyActionMap->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] = a2;
          ++pKeyActionMap->uNumKeysPressed;
        }
      }
    }
  }
  else
  {
    pKeyActionMap->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] = a2;
    ++pKeyActionMap->uNumKeysPressed;
    pKeyActionMap->pPressedKeysBuffer[pKeyActionMap->uNumKeysPressed] = 0;
    pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_CONFIRMED);
  }
  return 1;
}
// 506E68: using guessed type int uGameMenuUI_CurentlySelectedKeyIdx;

//----- (00459FFC) --------------------------------------------------------
void KeyboardActionMapping::ReadMappings()
{
  char str[32];

  for (size_t i = 0; i < keyMappingParams.size(); i++)
  {
    const char* keyName = keyMappingParams[i].m_keyName.c_str();
    short commandDefaultKeyCode = keyMappingParams[i].m_keyDefaultCode;
    short commandId = keyMappingParams[i].m_cmdId;
    KeyToggleType toggType = keyMappingParams[i].m_toggType;

    ReadWindowsRegistryString(keyName, str, 32, "DEFAULT");
    if ( strcmp(str, "DEFAULT") && ( TranslateKeyNameToKeyCode(str) != -1) )
      pVirtualKeyCodesMapping[commandId] = TranslateKeyNameToKeyCode(str);
    else
      pVirtualKeyCodesMapping[commandId] = commandDefaultKeyCode;
    pToggleTypes[commandId] = toggType;
  }

  bAlwaysRun = ReadWindowsRegistryInt("valAlwaysRun", 0) != 0;
  bFlipOnExit = ReadWindowsRegistryInt("FlipOnExit", 0) != 0;
}

//----- (0045A960) --------------------------------------------------------
void KeyboardActionMapping::StoreMappings()
{

  const char *v2; // eax@1

  for ( size_t i = 0; i < keyMappingParams.size(); i++)
  {
    v2 = GetVKeyDisplayName(pVirtualKeyCodesMapping[keyMappingParams[i].m_cmdId]);
    WriteWindowsRegistryString(keyMappingParams[i].m_keyName.c_str(), v2);
  }
}

//----- (0045ABCA) --------------------------------------------------------
const unsigned __int8 KeyboardActionMapping::TranslateKeyNameToKeyCode(const char *Str)
{
  if (strlen(Str) == 1)
  {
    if( Str[0] >= 65 && Str[0] <= 90 )
      return *Str;
    else
      return 0xFF;
  }

  for ( size_t i = 0; i < keyNameToCodeTranslationMap.size(); i++)
  {
    if (!strcmp(Str, std::get<0>(keyNameToCodeTranslationMap[i])))
      return std::get<1>(keyNameToCodeTranslationMap[i]);
  }
  return 0xFF;
}

//----- (0045AE2C) --------------------------------------------------------
const char * KeyboardActionMapping::GetVKeyDisplayName(unsigned char a1)
{
  static char static_sub_45AE2C_string_69ADE0_keyName[32];

  if ( a1 >= 65 && a1 <= 90 )
  {
    static_sub_45AE2C_string_69ADE0_keyName[0] = a1;
    static_sub_45AE2C_string_69ADE0_keyName[1] = '\0';
    return static_sub_45AE2C_string_69ADE0_keyName;
  }

  for ( size_t i = 0; i < keyNameToCodeTranslationMap.size(); i++)
  {
    if ( a1 == std::get<1>(keyNameToCodeTranslationMap[i]))
    {
      const char* keyName =  std::get<0>(keyNameToCodeTranslationMap[i]);
      strcpy_s(static_sub_45AE2C_string_69ADE0_keyName, keyName);
      return static_sub_45AE2C_string_69ADE0_keyName;
    }
  }

  strcpy_s(static_sub_45AE2C_string_69ADE0_keyName, "-мер -");
  return static_sub_45AE2C_string_69ADE0_keyName;
}


//----- (0045B019) --------------------------------------------------------
void Keyboard::EnterCriticalSection()
{
}

//----- (0045B06E) --------------------------------------------------------
bool Keyboard::IsShiftHeld()
{
  return (GetAsyncKeyState(VK_SHIFT) & 0x8001) != 0;
}

//----- (0045B0A9) --------------------------------------------------------
bool Keyboard::IsKeyBeingHeld(int vKey)
{
  return (GetAsyncKeyState(vKey) & 0x8001) != 0;
}

//----- (0045B0CE) --------------------------------------------------------
bool Keyboard::WasKeyPressed(int vKey)
{
  return (GetAsyncKeyState(vKey) & 1) != 0;
}
//----- (0046A14B) --------------------------------------------------------
void OnPressSpace()
{

  //if ( pRenderer->pRenderD3D )
  {
    pEngine->PickKeyboard(Keyboard::IsKeyBeingHeld(VK_CONTROL), &vis_sprite_filter_3, &vis_door_filter);
    int pid = pEngine->pVisInstance->get_picked_object_zbuf_val();
    if ( pid != -1 )
      DoInteractionWithTopmostZObject(pid & 0xFFFF, PID_ID(pid));
    return;
  }

  
  // software render stuff following
  /*
  static int dword_720660[100]; // 720660
  static int dword_7207F0[100]; // 7207F0

  v22 = 0;
  v1 = (int *)((signed int)(viewparams->uScreen_BttmR_X + viewparams->uScreen_topL_X) >> 1);//wrong pointer
  if ( (signed int)viewparams->uScreen_topL_Y < (signed int)viewparams->uScreen_BttmR_Y )
  {
	  v2 = (char *)v1 - 50;
	  v1 = (int *)((char *)v1 + 50);
	  v3 = 640 * viewparams->uScreen_topL_Y;
	  v17 = v2;
	  v20 = v1;
	  v18 = ((viewparams->uScreen_BttmR_Y - viewparams->uScreen_topL_Y - 1) >> 1) + 1;
	  do
	  {
		if ( (signed int)v2 < (signed int)v20 )
		{
			v1 = &pRenderer->pActiveZBuffer[(int)&v2[v3]];
			v21 = &pRenderer->pActiveZBuffer[(int)&v2[v3]];
			v4 = v22;
			v5 = (((char *)v20 - v2 - 1) >> 1) + 1;
			do
			{
			  v6 = 0;
			  v7 = *v1 & 0xFFFF;
			  v19 = 0;
			  if ( v4 > 0 )
			  {
				do
				{
				  if ( dword_7207F0[v6] == v7 )
					break;
				  ++v6;
				  v19 = v6;
				}
				while ( v6 < v22 );
			  }
			  if ( PID_TYPE(v7) == OBJECT_Decoration)
			  {
				v16 = (unsigned int)PID_ID(v7);
				if ( (signed int)(((unsigned int)*v21 >> 16)
								- pDecorationList->pDecorations[pLevelDecorations[(unsigned int)PID_ID(v7)].uDecorationDescID].uRadius) <= 512 )
				  if ( v19 == v22 && v4 < 100 )
				  {
					++v22;
					++v4;
					v8 = *v21;
					dword_7207F0[v4 - 1] = v7;
					dword_720660[v4 - 1] = v8;
				  }
			  }
			  else if ( (unsigned int)*v21 <= 0x2000000 )
			  {
				  if ( v19 == v22 && v4 < 100 )
				  {
					++v22;
					++v4;
					v8 = *v21;
					dword_7207F0[v4 - 1] = v7;
					dword_720660[v4 - 1] = v8;
				  }
			  }
			  v1 = v21 + 2;
			  --v5;
			  v21 += 2;
			}
			while ( v5 );
			v2 = v17;
		}
		v3 += 1280;
		--v18;
	  }
	  while ( v18 );
  }
  if ( v22 > 0 )
  {
    v9 = dword_720660;
    v10 = 1;
    do
    {
      for ( i = v10; i < v22; ++i )
      {
        v12 = *v9;
        v13 = dword_720660[i];
        if ( v13 < *v9 )
        {
          *v9 = v13;
          dword_720660[i] = v12;
        }
      }
      ++v10;
      ++v9;
      LOBYTE(v1) = v10 - 1;
    }
    while ( v10 - 1 < v22 );
  }
  for ( j = 0; j < v22; ++j )
  {
    LOBYTE(v1) = DoInteractionWithTopmostZObject(dword_720660[j] & 0xFFFF, v16);
    if ( !(char)v1 )
      break;
  }*/
}


//----- (0042FC4E) --------------------------------------------------------
void Keyboard::ProcessInputActions()
{
  char v4; // al@9
  unsigned __int16 v9; // ax@102
  int spell_price; // eax@103
  PartyAction partyAction; // [sp-14h] [bp-1Ch]@20
  InputAction inputAction; // [sp+0h] [bp-8h]@7

  pEngine->pKeyboardInstance->EnterCriticalSection();
  Keyboard* pKeyboard = pEngine->pKeyboardInstance;
  if (!bAlwaysRun)
  {
    if (pKeyboard->IsShiftHeld())
      pParty->uFlags2 |= PARTY_FLAGS_2_RUNNING;
    else
      pParty->uFlags2 &= ~PARTY_FLAGS_2_RUNNING;
   }
  else
  {
    if (pKeyboard->IsShiftHeld())
      pParty->uFlags2 &= ~PARTY_FLAGS_2_RUNNING;
    else
      pParty->uFlags2 |= PARTY_FLAGS_2_RUNNING;
  }

  //pParty->uFlags2 |= PARTY_FLAGS_2_RUNNING;


    //  WUT? double event trigger
  /*for ( uint i = 0; i < 30; ++i )
  {
    if ( pKeyActionMap->pToggleTypes[i] )
      v14 = pEngine->pKeyboardInstance->WasKeyPressed(pKeyActionMap->pVirtualKeyCodesMapping[i]);
    else
      v14 = pEngine->pKeyboardInstance->IsKeyBeingHeld(pKeyActionMap->pVirtualKeyCodesMapping[i]);
    if ( v14 )
    {
      if (current_screen_type == SCREEN_GAME)
      {
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Game_Action, 0, 0);
        continue;
      }
      if ( current_screen_type == SCREEN_NPC_DIALOGUE || current_screen_type == SCREEN_BRANCHLESS_NPC_DIALOG )
      {
        v15 = pMessageQueue_50CBD0->uNumMessages;
        if ( pMessageQueue_50CBD0->uNumMessages )
        {
          v15 = 0;
          if ( pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].field_8 )
          {
            v15 = 1;
            pMessageQueue_50CBD0->uNumMessages = 0;
            pMessageQueue_50CBD0->pMessages[v15].eType = UIMSG_Escape;
            pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].param = 0;
            *(&pMessageQueue_50CBD0->uNumMessages + 3 * pMessageQueue_50CBD0->uNumMessages + 3) = 0;
            ++pMessageQueue_50CBD0->uNumMessages;
            continue;
          }
          pMessageQueue_50CBD0->uNumMessages = 0;
        }
        //pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
      }
    }
  }*/
  if ( !pEventTimer->bPaused )
  {
    for ( uint i = 0; i < 30; ++i )
    {
      inputAction = (InputAction)i;
      if ( pKeyActionMap->pToggleTypes[inputAction] )
        v4 = pKeyboard->WasKeyPressed(pKeyActionMap->pVirtualKeyCodesMapping[inputAction]);
      else
        v4 = pKeyboard->IsKeyBeingHeld(pKeyActionMap->pVirtualKeyCodesMapping[inputAction]);
      if ( v4 )
      {
        switch ( inputAction )
        {
          case INPUT_MoveForward:
            if (current_screen_type  != SCREEN_GAME)
              break;
            if (!pParty->bTurnBasedModeOn)
            {
              if ( pParty->uFlags2 & PARTY_FLAGS_2_RUNNING)
                partyAction = PARTY_RunForward;
              else
                partyAction = PARTY_WalkForward;
              pPartyActionQueue->Add(partyAction);
              break;
            }
            if (pTurnEngine->turn_stage != TE_WAIT && pTurnEngine->turn_stage != TE_ATTACK && pTurnEngine->uActionPointsLeft > 0 )
            {
              pTurnEngine->uActionPointsLeft -= 26;
              if ( pParty->uFlags2 & PARTY_FLAGS_2_RUNNING)
                partyAction = PARTY_RunForward;
              else
                partyAction = PARTY_WalkForward;
              pPartyActionQueue->Add(partyAction);
              break;
            }
            break;
          case INPUT_MoveBackwards:
            if (current_screen_type  != SCREEN_GAME)
              break;
            if (!pParty->bTurnBasedModeOn)
            {
              if ( pParty->uFlags2 & 2 )
                partyAction = PARTY_RunBackward;
              else
                partyAction = PARTY_WalkBackward;
              pPartyActionQueue->Add(partyAction);
              break;
            }
            if ( pTurnEngine->turn_stage != TE_WAIT && pTurnEngine->turn_stage != TE_ATTACK && pTurnEngine->uActionPointsLeft > 0 )
            {
              pTurnEngine->uActionPointsLeft -= 26;
              if ( pParty->uFlags2 & 2 )
                partyAction = PARTY_RunBackward;
              else
                partyAction = PARTY_WalkBackward;
              pPartyActionQueue->Add(partyAction);
              break;
            }
            break;
          case INPUT_StrafeLeft:
            if (current_screen_type  != SCREEN_GAME)
              break;
            if (!pParty->bTurnBasedModeOn)
            {
              partyAction = PARTY_StrafeLeft;
              pPartyActionQueue->Add(partyAction);
              break;
            }
            if ( pTurnEngine->turn_stage == TE_WAIT || pTurnEngine->turn_stage == TE_ATTACK || pTurnEngine->uActionPointsLeft <= 0 )
              break;
            pTurnEngine->uActionPointsLeft -= 26;
            partyAction = PARTY_StrafeLeft;
            pPartyActionQueue->Add(partyAction);
            break;
          case INPUT_StrafeRight:
            if (current_screen_type != SCREEN_GAME)
              break;
            if (!pParty->bTurnBasedModeOn)
            {
              partyAction = PARTY_StrafeRight;
              pPartyActionQueue->Add(partyAction);
              break;
            }
            if ( pTurnEngine->turn_stage == TE_WAIT || pTurnEngine->turn_stage == TE_ATTACK || pTurnEngine->uActionPointsLeft <= 0 )
              break;
            pTurnEngine->uActionPointsLeft -= 26;
            partyAction = PARTY_StrafeRight;
            pPartyActionQueue->Add(partyAction);
            break;
          case INPUT_TurnLeft:
            if (current_screen_type != SCREEN_GAME)
              break;
            if ( GetAsyncKeyState(VK_CONTROL) ) // strafing
            {
              if (pParty->bTurnBasedModeOn)
              {
                if ( pTurnEngine->turn_stage == TE_WAIT || pTurnEngine->turn_stage == TE_ATTACK || pTurnEngine->uActionPointsLeft <= 0 )
                  break;
                pTurnEngine->uActionPointsLeft -= 26;
              }
              partyAction = PARTY_StrafeLeft;
            }
            else
            {
              if ( pParty->uFlags2 & 2 )
                partyAction = PARTY_FastTurnLeft;
              else
                partyAction = PARTY_TurnLeft;
            }
            pPartyActionQueue->Add(partyAction);
            if (uCurrentlyLoadedLevelType == LEVEL_Outdoor && pWeather->bRenderSnow)
              pWeather->OnPlayerTurn(10);
            break;
          case INPUT_TurnRight:
            if (current_screen_type != SCREEN_GAME)
              break;
            if ( GetAsyncKeyState(VK_CONTROL) )         // strafing
            {
              if (pParty->bTurnBasedModeOn)
              {
                if ( pTurnEngine->turn_stage == TE_WAIT || pTurnEngine->turn_stage == TE_ATTACK || pTurnEngine->uActionPointsLeft <= 0 )
                  break;
                pTurnEngine->uActionPointsLeft -= 26;
              }
              partyAction = PARTY_StrafeRight;
            }
            else
            {
              if ( pParty->uFlags2 & 2 )
                partyAction = PARTY_FastTurnRight;
              else
                partyAction = PARTY_TurnRight;
            }
            pPartyActionQueue->Add(partyAction);
            if (uCurrentlyLoadedLevelType == LEVEL_Outdoor && pWeather->bRenderSnow)
              pWeather->OnPlayerTurn(-10);
            break;
          case INPUT_Jump:
            if (current_screen_type != SCREEN_GAME || pParty->bTurnBasedModeOn)
              break;
            partyAction = (PartyAction)12;
            pPartyActionQueue->Add(partyAction);
            break;
          case INPUT_Yell:
            if (!current_screen_type && uActiveCharacter)
            {
              pParty->Yell();
              pPlayers[uActiveCharacter]->PlaySound(SPEECH_Yell, 0);
            }
          break;
          case INPUT_Pass:
            if ( current_screen_type )
              break;
            if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT)
            {
              pTurnEngine->field_18 |= TE_FLAG_8;
              break;
            }
            if ( uActiveCharacter )
            {
              if ( !pPlayers[uActiveCharacter]->uTimeToRecovery )
              {
                if ( !pParty->bTurnBasedModeOn )
                  pPlayers[uActiveCharacter]->SetRecoveryTime((signed __int64)(flt_6BE3A4_debug_recmod1 * (double)pPlayers[uActiveCharacter]->GetAttackRecoveryTime(false) * 2.133333333333333));
                CastSpellInfoHelpers::_427D48();
                pTurnEngine->ApplyPlayerAction();
              }
            }
            break;
          case INPUT_Combat://if press ENTER
            if (current_screen_type == SCREEN_GAME)
            {
              if (pParty->bTurnBasedModeOn)
              {
                if (pTurnEngine->turn_stage == TE_MOVEMENT || PID_TYPE(pTurnEngine->pQueue[0].uPackedID) == OBJECT_Player)
                {
                  pParty->bTurnBasedModeOn = 0;
                  pTurnEngine->End(true);
                }
              }
              else
              {
                pTurnEngine->Start();
                pParty->bTurnBasedModeOn = true;
              }
            }
            break;
          case INPUT_CastReady:
            {
              if (current_screen_type != SCREEN_GAME)
                break;
              if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT)
              {
                pTurnEngine->field_18 |= TE_FLAG_8;
                break;
              }
              if ( !uActiveCharacter )
                break;
              uchar quickSpellNumber = pPlayers[uActiveCharacter]->uQuickSpell;
              v9 = pPlayers[uActiveCharacter]->pActiveSkills[quickSpellNumber / 11 + 12];
              bool enoughMana = false;
              if ((v9 & 0x100) != 0)
              {
                enoughMana = pSpellDatas[quickSpellNumber].uMagisterLevelMana < pPlayers[uActiveCharacter]->sMana;
              }
              else if ((v9 & 0x80) != 0)
              {
                enoughMana = pSpellDatas[quickSpellNumber].uMasterLevelMana < pPlayers[uActiveCharacter]->sMana;
              }
              else if ((v9 & 0x40) != 0)
              {
                enoughMana = pSpellDatas[quickSpellNumber].uExpertLevelMana < pPlayers[uActiveCharacter]->sMana;
              }
              else
              {
                enoughMana = pSpellDatas[quickSpellNumber].uNormalLevelMana < pPlayers[uActiveCharacter]->sMana;
              }
              if ( !pPlayers[uActiveCharacter]->uQuickSpell || bUnderwater || !enoughMana)
              {
                pPartyActionQueue = pPartyActionQueue;
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Attack, 0, 0);
                break;
              }
              else
                pMessageQueue_50C9E8->AddGUIMessage(UIMSG_CastQuickSpell, 0, 0);
            }
            break;
          case INPUT_Attack:
            if (current_screen_type != SCREEN_GAME)
              break;
            if (pParty->bTurnBasedModeOn == true && pTurnEngine->turn_stage == TE_MOVEMENT)
            {
              pTurnEngine->field_18 |= TE_FLAG_8;
              break;
            }
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Attack, 0, 0);
            break;
          case INPUT_EventTrigger:
            if (current_screen_type == SCREEN_GAME)
            {
              pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Game_Action, 0, 0);
              break;
            }
            if ( current_screen_type == SCREEN_NPC_DIALOGUE )
            {
              if ( pMessageQueue_50CBD0->uNumMessages )
              {
                pMessageQueue_50CBD0->uNumMessages = 0;
                if ( pMessageQueue_50CBD0->pMessages[0].field_8 )
                {
                  pMessageQueue_50CBD0->uNumMessages = 1;
                  pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].eType = UIMSG_Escape;
                  pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].param = 0;
                  pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].field_8 = 0;
                  ++pMessageQueue_50CBD0->uNumMessages;
                  break;
                }
                break;
              }
              pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
            }
            break;
          case INPUT_CharCycle:
            if ( current_screen_type == SCREEN_SPELL_BOOK  )
              break;

            pMessageQueue_50C9E8->AddGUIMessage(UIMSG_CycleCharacters, 0, 0);
            break;
          case INPUT_LookUp:
            if ( pEventTimer->bPaused )
              break;
            partyAction = (PartyAction)7;
            pPartyActionQueue->Add(partyAction);
            break;
          case INPUT_CenterView:
            if ( pEventTimer->bPaused )
              break;
            partyAction = (PartyAction)9;
            pPartyActionQueue->Add(partyAction);
            break;
          case INPUT_LookDown:
            if ( pEventTimer->bPaused )
              break;
            partyAction = (PartyAction)8;
            pPartyActionQueue->Add(partyAction);
            break;
          case INPUT_FlyUp:
            if ( current_screen_type || pEventTimer->bPaused )
              break;
            partyAction = (PartyAction)13;
            pPartyActionQueue->Add(partyAction);
            break;
          case INPUT_Land:
            if ( current_screen_type || pEventTimer->bPaused )
              break;
            partyAction = (PartyAction)15;
            pPartyActionQueue->Add(partyAction);
            break;
          case INPUT_FlyDown:
            if ( !current_screen_type && !pEventTimer->bPaused )
            {
              partyAction = (PartyAction)14;
              pPartyActionQueue->Add(partyAction);
            }
            break;
          case INPUT_ZoomIn:
              pMessageQueue_50C9E8->AddGUIMessage(UIMSG_ClickZoomOutBtn, 0, 0);
            break;
          case INPUT_ZoomOut:
              pMessageQueue_50C9E8->AddGUIMessage(UIMSG_ClickZoomInBtn, 0, 0);
            break;
          case INPUT_AlwaysRun:
            bAlwaysRun = bAlwaysRun == 0;
            break;
          default:
            break;
        }
      }
    }
  }
}
