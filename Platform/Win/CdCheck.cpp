#include <stdio.h>

#include "Engine/Log.h"

#include "Platform/Api.h"
#include "Platform/Win/Win.h"








HWND hInsertCDWindow; // idb


                      //----- (00464E17) --------------------------------------------------------
bool CheckMM7CD(char c)
{
    char DstBuf[256] = { 0 };
    char strCommand[256] = { 0 }; // [sp+10Ch] [bp-118h]@1
    char Filename[20] = { 0 }; // [sp+20Ch] [bp-18h]@1

    wchar_t pMagicPath[1024];
    swprintf(pMagicPath, wcslen(L"%C:\\anims\\magic7.vid"), L"%C:\\anims\\magic7.vid", c);
    if (GetFileAttributesW(pMagicPath) == -1)
        return false;

    //Open CD audio
    wsprintfA(strCommand, "open %c: type cdaudio alias CD", c);
    if (!mciSendStringA(strCommand, DstBuf, 255, 0))
    {
        wsprintfA(strCommand, "info CD UPC wait");
        mciSendStringA(strCommand, DstBuf, 255, 0);
        wsprintfA(strCommand, "close CD");
        mciSendStringA(strCommand, DstBuf, 255, 0);
    }

    memcpy(Filename, "X:\\anims\\magic7.vid", sizeof(Filename));
    *Filename = c;

    FILE* f = fopen(Filename, "rb");
    if (!f)
        return false;

    if (!fseek(f, 0, SEEK_END))
    {
        if (!fseek(f, -100, SEEK_CUR))
            fread(DstBuf, 1, 0x64u, f);

        fclose(f);
        return true;
    }
    fclose(f);
    return false;
}




//----- (00464F1B) --------------------------------------------------------
int __stdcall InsertMM7CDDialogFunc(HWND hDlg, int a2, __int16 a3, int a4)
{
    char v4; // zf@3
    int v6; // eax@10
    int v7; // eax@11
    int v8; // eax@12
    int v9; // eax@13
    BOOL(__stdcall *v10)(HWND, int, LPCSTR); // edi@15
    const CHAR *v11; // [sp-Ch] [bp-Ch]@15
    INT_PTR v12; // [sp-4h] [bp-4h]@5

    if (a2 == 272)
    {
        hInsertCDWindow = hDlg;
        v6 = (GetUserDefaultLangID() & 0x3FF) - 7;
        if (v6)
        {
            v7 = v6 - 3;
            if (v7)
            {
                v8 = v7 - 2;
                if (v8)
                {
                    v9 = v8 - 4;
                    if (v9)
                    {
                        if (v9 != 5)
                            return 0;
                        SetWindowTextA(hDlg, "Wloz CD-ROM numer 2");
                        v10 = SetDlgItemTextA;
                        SetDlgItemTextA(hDlg, 1010, "Wloz CD-ROM numer 2 Might and Magic® VII.");
                        v11 = "Odwolaj";
                    }
                    else
                    {
                        SetWindowTextA(hDlg, "Inserire il secondo CD");
                        v10 = SetDlgItemTextA;
                        SetDlgItemTextA(hDlg, 1010, "Inserire il secondo CD di Might and Magic® VII.");
                        v11 = "Annulla";
                    }
                }
                else
                {
                    SetWindowTextA(hDlg, "Insérez le CD 2");
                    v10 = SetDlgItemTextA;
                    SetDlgItemTextA(hDlg, 1010, "Insérez Might & Magic® VII CD 2.");
                    v11 = "Supprimer";
                }
            }
            else
            {
                SetWindowTextA(hDlg, "Por favor, inserte disco 2");
                v10 = SetDlgItemTextA;
                SetDlgItemTextA(hDlg, 1010, "Por favor, inserte disco 2 de Might & Magic® VII.");
                v11 = "Cancelar";
            }
        }
        else
        {
            SetWindowTextA(hDlg, "Bitte CD 2 einlegen");
            v10 = SetDlgItemTextA;
            SetDlgItemTextA(hDlg, 1010, "Bitte CD 2 von Might and Magic® VII einlegen.");
            v11 = "Abbrechen";
        }
        v10(hDlg, 2, v11);
        return 0;
    }
    if (a2 == 273)
    {
        if (a3 == 2)
        {
            v12 = 0;
            EndDialog(hDlg, v12);
            return 1;
        }
        v4 = a3 == 1;
    }
    else
    {
        v4 = a2 == 1025;
    }
    if (v4)
    {
        v12 = 1;
        EndDialog(hDlg, v12);
        return 1;
    }
    return 0;
}



//----- (00465061) --------------------------------------------------------
bool OS_FindMM7CD(char *pCDDrive)
{
    char drive[4] = { 'X', ':', '\\', 0 };

    bool bGotCDFromRegistry = false;

    HKEY hSoftware = nullptr,
        hNWC = nullptr,
        hMM7 = nullptr,
        hVersion = nullptr;
    if (!RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, KEY_READ | KEY_WOW64_32KEY, &hSoftware))
    {
        if (!RegOpenKeyExA(hSoftware, "New World Computing", 0, KEY_READ | KEY_WOW64_32KEY, &hNWC))
        {
            if (!RegOpenKeyExA(hNWC, "Might and Magic VII", 0, KEY_READ | KEY_WOW64_32KEY, &hMM7))
            {
                if (!RegOpenKeyExA(hMM7, "1.0", 0, KEY_READ | KEY_WOW64_32KEY, &hVersion))
                {
                    DWORD cbData = 3;
                    if (!RegQueryValueExA(hVersion, "CDDrive", 0, 0, (BYTE *)drive, &cbData))
                        bGotCDFromRegistry = true;
                }
                RegCloseKey(hVersion);
            }
            RegCloseKey(hMM7);
        }
        RegCloseKey(hNWC);
    }
    RegCloseKey(hSoftware);

    if (bGotCDFromRegistry)
        if (CheckMM7CD(*drive))
        {
            *pCDDrive = *drive;
            return true;
        }

    while (true)
    {
        for (unsigned int i = 0; i < 26; ++i)
        {
            drive[0] = 'A' + i;

            if (GetDriveTypeA(drive) == DRIVE_CDROM)
                if (CheckMM7CD(*drive))
                {
                    *pCDDrive = *drive;
                    OS_SetAppString("CDDrive", drive);
                    return true;
                }
        }

        if (DialogBoxParamA(GetModuleHandleW(nullptr), "InsertCD", nullptr, (DLGPROC)InsertMM7CDDialogFunc, 0))
            continue;
        return false;
    }
}