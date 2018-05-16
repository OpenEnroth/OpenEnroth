#include "Engine/Graphics/Direct3D/RenderD3D.h"

#include <cstdio>

#include "Platform/OSWindow.h"

#include "Engine/LOD.h"

HRESULT __stdcall DDrawDisplayModesEnumerator(DDSURFACEDESC2 *pSurfaceDesc,
                                              __int16 *found_compatible_mode) {
    if (pSurfaceDesc->ddsCaps.dwCaps | DDSCAPS_3DDEVICE) {
        *found_compatible_mode = 1;
        return S_OK;
    }
    return 1;
}

HRESULT __stdcall D3DDeviceEnumerator(
    const GUID *lpGUID, const char *lpDeviceDesc, const char *lpDeviceName,
    D3DDEVICEDESC *pHWDesc, D3DDEVICEDESC *pSWDesc, RenderD3D_aux *a6) {
    int v7 = -1;
    if (pHWDesc->dwFlags) {
        if (!a6->ptr_4->pGUID) v7 = 0;
        if (pHWDesc->dwFlags && a6->ptr_4->pGUID) v7 = 1;
    }
    if (!strcmp(lpDeviceName, "RGB Emulation") && !a6->ptr_4->pGUID) v7 = 2;
    if (!strcmp(lpDeviceName, "Reference Rasterizer") && !a6->ptr_4->pGUID)
        v7 = 3;
    if (v7 != -1) {
        a6->pInfo[v7].bIsDeviceCompatible = 1;
        a6->pInfo[v7].uCaps = 0;
        if (!(pHWDesc->dpcTriCaps.dwSrcBlendCaps & 0x10))
            a6->pInfo[v7].uCaps |= 2;
        if (!(pHWDesc->dpcTriCaps.dwSrcBlendCaps & 2)) a6->pInfo[v7].uCaps |= 4;
        if (!(pHWDesc->dpcTriCaps.dwSrcBlendCaps & 1)) a6->pInfo[v7].uCaps |= 8;
        if (!(pHWDesc->dpcTriCaps.dwDestBlendCaps & 0x20))
            a6->pInfo[v7].uCaps |= 16;
        if (!(pHWDesc->dpcTriCaps.dwDestBlendCaps & 2))
            a6->pInfo[v7].uCaps |= 32;
        if (!(pHWDesc->dpcTriCaps.dwDestBlendCaps & 4))
            a6->pInfo[v7].uCaps |= 64;
        if (!(pHWDesc->dwDevCaps & 0x1000)) a6->pInfo[v7].uCaps |= 0x100;
        if (pHWDesc->dpcTriCaps.dwTextureCaps & 0x20)
            a6->pInfo[v7].uCaps |= 0x80;

        a6->pInfo[v7].pName = new char[strlen(lpDeviceName) + 1];
        strcpy(a6->pInfo[v7].pName, lpDeviceName);

        a6->pInfo[v7].pDescription = new char[strlen(lpDeviceDesc) + 1];
        strcpy(a6->pInfo[v7].pDescription, lpDeviceDesc);

        a6->pInfo[v7].pGUID = new GUID;
        memcpy(a6->pInfo[v7].pGUID, lpGUID, 0x10);

        a6->pInfo[v7].pDriverName = strdup(a6->ptr_4->pDriverName);
        a6->pInfo[v7].pDeviceDesc = strdup(a6->ptr_4->pDeviceDesc);
        a6->pInfo[v7].pDDraw4DevDesc = strdup(a6->ptr_4->pDDraw4DevDesc);

        if (a6->ptr_4->pGUID) {
            a6->pInfo[v7].pDirectDrawGUID = new GUID;
            memcpy(a6->pInfo[v7].pDirectDrawGUID, a6->ptr_4->pGUID, 0x10);
        } else {
            a6->pInfo[v7].pDirectDrawGUID = 0;
        }
        a6->pInfo[v7].uVideoMem = a6->ptr_4->uVideoMem;
    }
    return 1;
}

int __stdcall RenderD3D__DeviceEnumerator(GUID *lpGUID, const char *lpDevDesc,
                                          const char *lpDriverName,
                                          RenderD3D__DevInfo *pOut) {
    size_t v5;                   // eax@1
    size_t v7;                   // eax@13
    DDDEVICEIDENTIFIER ddDevId;  // [sp+4h] [bp-4F8h]@11
    DDSURFACEDESC2 v10;          // [sp+42Ch] [bp-D0h]@16
    DDSCAPS2 ddsCaps;            // [sp+4A8h] [bp-54h]@14
    unsigned int uFreeVideoMem;  // [sp+4B8h] [bp-44h]@14
    RenderD3D_aux aux;           // [sp+4BCh] [bp-40h]@19
    IDirect3D3 *pDirect3D3;      // [sp+4C4h] [bp-38h]@18
    int is_there_a_compatible_screen_mode;  // [sp+4C8h] [bp-34h]@16
    IDirectDraw4 *pDirectDraw4;             // [sp+4F8h] [bp-4h]@7

    RenderD3D_D3DDevDesc v20 = {0};
    size_t v4 = strlen(lpDriverName);
    v20.pDriverName = new char[v4 + 1];
    v5 = strlen(lpDevDesc);
    v20.pDeviceDesc = new char[v5 + 1];
    strcpy(v20.pDriverName, lpDriverName);
    strcpy(v20.pDeviceDesc, lpDevDesc);
    if (lpGUID) {
        v20.pGUID = new GUID;
        memcpy(v20.pGUID, lpGUID, 0x10);
    } else {
        v20.pGUID = 0;
    }

    LPDIRECTDRAW pDirectDraw = nullptr;
    if (FAILED(DirectDrawCreate(v20.pGUID, &pDirectDraw, 0))) {
        delete[] v20.pDriverName;
        delete[] v20.pDeviceDesc;
        delete v20.pGUID;
    } else {
        if (FAILED(pDirectDraw->QueryInterface(IID_IDirectDraw4,
                                               (LPVOID *)&pDirectDraw4))) {
            delete[] v20.pDriverName;
            delete[] v20.pDeviceDesc;
            delete v20.pGUID;
            pDirectDraw->Release();
        } else {
            pDirectDraw->Release();
            if (FAILED(pDirectDraw4->GetDeviceIdentifier(&ddDevId, 1))) {
                v20.pDDraw4DevDesc = 0;
            } else {
                v7 = strlen(ddDevId.szDescription);
                v20.pDDraw4DevDesc = new char[v7 + 1];
                strcpy(v20.pDDraw4DevDesc, ddDevId.szDescription);
            }
            memset(&ddsCaps, 0, sizeof(ddsCaps));
            if (FAILED(pDirectDraw4->GetAvailableVidMem(
                    &ddsCaps, (LPDWORD)&v20.uVideoMem,
                    (LPDWORD)&uFreeVideoMem)))
                v20.uVideoMem = 0;
            memset(&v10, 0, sizeof(v10));
            v10.dwSize = 124;
            v10.dwFlags = 6;
            v10.dwHeight = window->GetWidth();
            v10.dwWidth = window->GetHeight();
            v10.ddpfPixelFormat.dwSize = 32;

            is_there_a_compatible_screen_mode = false;
            if (FAILED(pDirectDraw4->EnumDisplayModes(
                    0, 0, &is_there_a_compatible_screen_mode,
                    (LPDDENUMMODESCALLBACK2)DDrawDisplayModesEnumerator)) ||
                !is_there_a_compatible_screen_mode ||
                FAILED(pDirectDraw4->QueryInterface(IID_IDirect3D3,
                                                    (LPVOID *)&pDirect3D3))) {
                delete[] v20.pDriverName;
                delete[] v20.pDeviceDesc;
                // free(v20.pDDraw4DevDesc);
                delete[] v20.pDDraw4DevDesc;
                delete v20.pGUID;
                pDirectDraw4->Release();
            } else {
                aux.pInfo = pOut;
                aux.ptr_4 = &v20;
                pDirect3D3->EnumDevices(
                    (LPD3DENUMDEVICESCALLBACK)D3DDeviceEnumerator, &aux);
                delete[] v20.pDriverName;
                delete[] v20.pDeviceDesc;
                delete[] v20.pDDraw4DevDesc;
                delete v20.pGUID;
                pDirectDraw4->Release();
                pDirectDraw4 = 0;
                pDirect3D3->Release();
            }
        }
    }
    return 1;
}

HRESULT __stdcall D3DZBufferFormatEnumerator(DDPIXELFORMAT *Src,
                                             DDPIXELFORMAT *Dst) {
    if (Src->dwFlags & (0x400 | 0x2000)) {
        if (Src->dwRGBBitCount == 16 && !Src->dwRBitMask) {
            memcpy(Dst, Src, sizeof(DDPIXELFORMAT));
            return 0;
        }
        if (!Dst->dwSize) {
            memcpy(Dst, Src, sizeof(DDPIXELFORMAT));
            return 1;
        }
    }
    return 1;
}

void RenderD3D::GetAvailableDevices(RenderD3D__DevInfo **pOutDevices) {
    RenderD3D__DevInfo *v2;  // eax@1

    v2 = new RenderD3D__DevInfo[4];  // 4 items
    *pOutDevices = v2;
    memset(v2, 0, sizeof(v2));
    DirectDrawEnumerateExA((LPDDENUMCALLBACKEXA)RenderD3D__DeviceEnumerator,
                           *pOutDevices, DDENUM_ATTACHEDSECONDARYDEVICES);
}

RenderD3D::RenderD3D() {
    this->pHost = nullptr;
    this->pDirect3D = nullptr;
    this->pUnk = nullptr;
    this->pBackBuffer = nullptr;
    this->pFrontBuffer = nullptr;
    this->pZBuffer = nullptr;
    this->pDevice = nullptr;
    this->pViewport = nullptr;
    this->field_40 = 1;
    this->field_44 = 10;
    GetAvailableDevices(&this->pAvailableDevices);
}

void RenderD3D::Release() {
    if (!this->bWindowed) {
        if (this->pHost) {
            this->pHost->RestoreDisplayMode();
            this->pHost->SetCooperativeLevel(this->hWindow, DDSCL_NORMAL);
            this->pHost->FlipToGDISurface();
        }
    }

    for (int i = 0; i < 4; i++) {
        if (this->pAvailableDevices[i].pDriverName) {
            delete[] this->pAvailableDevices[i].pDriverName;
            this->pAvailableDevices[i].pDriverName = nullptr;
        }

        if (this->pAvailableDevices[i].pDeviceDesc) {
            delete[] this->pAvailableDevices[i].pDeviceDesc;
            this->pAvailableDevices[i].pDeviceDesc = nullptr;
        }

        if (this->pAvailableDevices[i].pDDraw4DevDesc) {
            delete[] this->pAvailableDevices[i].pDDraw4DevDesc;
            this->pAvailableDevices[i].pDDraw4DevDesc = nullptr;
        }

        if (this->pAvailableDevices[i].pDirectDrawGUID) {
            delete this->pAvailableDevices[i].pDirectDrawGUID;
            this->pAvailableDevices[i].pDirectDrawGUID = nullptr;
        }

        if (this->pAvailableDevices[i].pName) {
            delete[] this->pAvailableDevices[i].pName;
            this->pAvailableDevices[i].pName = nullptr;
        }

        if (this->pAvailableDevices[i].pDescription) {
            delete[] this->pAvailableDevices[i].pDescription;
            this->pAvailableDevices[i].pDescription = nullptr;
        }

        if (this->pAvailableDevices[i].pGUID) {
            delete this->pAvailableDevices[i].pGUID;
            this->pAvailableDevices[i].pGUID = nullptr;
        }
    }

    delete[] this->pAvailableDevices;
    this->pAvailableDevices = NULL;

    if (this->pViewport) {
        this->pViewport->Release();
        this->pViewport = NULL;
    }

    if (this->pUnk) {
        this->pUnk->Release();
        this->pUnk = NULL;
    }

    if (this->pZBuffer) {
        this->pZBuffer->Release();
        this->pZBuffer = NULL;
    }

    if (this->pDevice) {
        this->pDevice->Release();
        this->pDevice = NULL;
    }

    if (this->pDirect3D) {
        this->pDirect3D->Release();
        this->pDirect3D = NULL;
    }

    if (this->pBackBuffer) {
        this->pBackBuffer->Release();
        this->pBackBuffer = NULL;
    }

    if (this->pFrontBuffer) {
        this->pFrontBuffer->Release();
        this->pFrontBuffer = NULL;
    }

    if (this->pHost) {
        this->pHost->Release();
        this->pHost = NULL;
    }
}

bool RenderD3D::CreateDevice(unsigned int uDeviceID, int bWindowed,
                             OSWindow *window) {
    DWORD v26;                        // [sp-4h] [bp-DCh]@30
    DDSCAPS2 v27;                     // [sp+Ch] [bp-CCh]@37
    DDSURFACEDESC2 ddsd2;             // [sp+1Ch] [bp-BCh]@11
    D3DVIEWPORT2 d3dvp2;              // [sp+98h] [bp-40h]@28
    IDirectDrawClipper *lpddclipper;  // [sp+C4h] [bp-14h]@18
    LPDIRECTDRAW lpDD;                // [sp+C8h] [bp-10h]@1

    auto hWnd = (HWND)window->GetWinApiHandle();
    int game_width = window->GetWidth();
    int game_height = window->GetHeight();

    this->bWindowed = bWindowed;
    this->hWindow = hWnd;

    //Создание объекта DirectDraw
    if (FAILED(DirectDrawCreate(pAvailableDevices[uDeviceID].pDirectDrawGUID,
                                &lpDD, NULL))) {
        sprintf(pErrorMessage,
                "Init - Failed to create DirectDraw interface.\n");
        return 0;
    }

    //Запрос интерфейса IDirectDraw4
    if (FAILED(lpDD->QueryInterface(IID_IDirectDraw4, (LPVOID *)&pHost))) {
        sprintf(pErrorMessage,
                "Init - Failed to create DirectDraw4 interface.\n");
        if (lpDD) lpDD->Release();
        return 0;
    }
    lpDD->Release();
    lpDD = NULL;

    //Задаём уровень совместного доступа для приложения DirectDraw в оконном
    //режиме
    if (bWindowed && !pAvailableDevices[uDeviceID].pDirectDrawGUID) {
        if (FAILED(pHost->SetCooperativeLevel(
                hWnd, DDSCL_MULTITHREADED | DDSCL_NORMAL))) {
            sprintf(pErrorMessage, "Init - Failed to set cooperative level.\n");
            if (pHost) {
                pHost->Release();
                pHost = NULL;
            }
            return 0;
        }

        //
        memset(&ddsd2, 0, sizeof(DDSURFACEDESC2));
        ddsd2.dwSize = sizeof(DDSURFACEDESC2);
        ddsd2.dwFlags = DDSD_CAPS;
        ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        //Создаём первичную поверхность
        if (FAILED(pHost->CreateSurface(&ddsd2, &pFrontBuffer, NULL))) {
            sprintf(pErrorMessage, "Init - Failed to create front buffer.\n");
            if (pHost) {
                pHost->Release();
                pHost = NULL;
            }
            return 0;
        }
        ddsd2.dwSize = sizeof(DDSURFACEDESC2);
        pHost->GetDisplayMode(&ddsd2);

        ddsd2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
        ddsd2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
        ddsd2.dwWidth = game_width;
        ddsd2.dwHeight = game_height;
        if (pHost->CreateSurface(&ddsd2, &pBackBuffer, NULL)) {
            sprintf(pErrorMessage, "Init - Failed to create back buffer.\n");
            if (pFrontBuffer) {
                pFrontBuffer->Release();
                pFrontBuffer = NULL;
            }
            if (pHost) {
                pHost->Release();
                pHost = NULL;
            }
            return 0;
        }
        //Создание отсекателя DirectDraw
        if (pHost->CreateClipper(0, &lpddclipper, NULL)) {
            sprintf(pErrorMessage, "Init - Failed to create clipper.\n");
            if (pBackBuffer) {
                pBackBuffer->Release();
                pBackBuffer = NULL;
            }
            if (pFrontBuffer) {
                pFrontBuffer->Release();
                pFrontBuffer = NULL;
            }
            if (pHost) {
                pHost->Release();
                pHost = NULL;
            }
            return 0;
        }
        lpddclipper->SetHWnd(0, hWnd);
        pFrontBuffer->SetClipper(lpddclipper);

        lpddclipper->Release();
        lpddclipper = NULL;
        //

        pHost->QueryInterface(IID_IDirect3D3, (LPVOID *)&pDirect3D);

        ddsd2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
        ddsd2.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
        ddsd2.dwWidth = game_width;
        ddsd2.dwHeight = game_height;

        if (pDirect3D->EnumZBufferFormats(
                *pAvailableDevices[uDeviceID].pGUID,
                (HRESULT(__stdcall *)(DDPIXELFORMAT *,
                                      void *))D3DZBufferFormatEnumerator,
                &ddsd2.ddpfPixelFormat)) {
            sprintf(pErrorMessage,
                    "Init - Failed to enumerate Z buffer formats.\n");
            if (pBackBuffer) {
                pBackBuffer->Release();
                pBackBuffer = NULL;
            }
            if (pFrontBuffer) {
                pFrontBuffer->Release();
                pFrontBuffer = NULL;
            }
            if (pHost) {
                pHost->Release();
                pHost = NULL;
            }
            return 0;
        }
        if (uDeviceID == 2 || uDeviceID == 3)
            ddsd2.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

        if (!pHost->CreateSurface(&ddsd2, &pZBuffer, NULL)) {
            if (!pBackBuffer->AddAttachedSurface(pZBuffer)) {
                if (!pDirect3D->CreateDevice(
                        *pAvailableDevices[uDeviceID].pGUID, pBackBuffer,
                        &pDevice, 0)) {
                    memset(&d3dvp2, 0, sizeof(D3DVIEWPORT2));
                    d3dvp2.dvClipWidth = 2.0;
                    d3dvp2.dvClipY = 1.0;
                    d3dvp2.dvClipHeight = 2.0;
                    d3dvp2.dvMaxZ = 1.0;
                    d3dvp2.dvMinZ = 0.0;
                    goto LABEL_54;
                }
                sprintf(pErrorMessage, "Init - Failed to create D3D device.\n");
                if (pDirect3D) {
                    pDirect3D->Release();
                    pDirect3D = NULL;
                }
                if (pZBuffer) {
                    pZBuffer->Release();
                    pZBuffer = NULL;
                }
                if (pBackBuffer) {
                    pBackBuffer->Release();
                    pBackBuffer = NULL;
                }
                if (pFrontBuffer) {
                    pFrontBuffer->Release();
                    pFrontBuffer = NULL;
                }
                if (pHost) {
                    pHost->Release();
                    pHost = NULL;
                }
                return 0;
            }
            sprintf(pErrorMessage,
                    "Init - Failed to attach z-buffer to back buffer.\n");
            if (pZBuffer) {
                pZBuffer->Release();
                pZBuffer = NULL;
            }
            if (pBackBuffer) {
                pBackBuffer->Release();
                pBackBuffer = NULL;
            }
            if (pFrontBuffer) {
                pFrontBuffer->Release();
                pFrontBuffer = NULL;
            }
            if (pHost) {
                pHost->Release();
                pHost = NULL;
            }
            return 0;
        }
        sprintf(pErrorMessage, "Init - Failed to create z-buffer.\n");
        if (pBackBuffer) {
            pBackBuffer->Release();
            pBackBuffer = NULL;
        }
        if (pFrontBuffer) {
            pFrontBuffer->Release();
            pFrontBuffer = NULL;
        }
        if (pHost) {
            pHost->Release();
            pHost = NULL;
        }
        return 0;
    }
    if (uDeviceID == 1)
        v26 = 1045;
    else
        v26 = 1041;
    if (pHost->SetCooperativeLevel(hWnd, v26)) {
        sprintf(pErrorMessage, "Init - Failed to set cooperative level.\n");
        if (pHost) {
            pHost->Release();
            pHost = NULL;
        }
        return 0;
    }
    if (pHost->SetDisplayMode(window->GetWidth(), window->GetHeight(), 16, 0,
                              0)) {
        sprintf(pErrorMessage, "Init - Failed to set display mode.\n");
        if (pHost) {
            pHost->Release();
            pHost = NULL;
        }
        return 0;
    }

    memset(&ddsd2, 0, sizeof(DDSURFACEDESC2));
    ddsd2.dwSize = sizeof(DDSURFACEDESC2);
    //Подключение полей с достоверными данными
    ddsd2.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    //Запрос сложной структуры с возможностью переключения
    ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE |
                           DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    //Присвоение полю счётчика задних буферов значения 1
    ddsd2.dwBackBufferCount = 1;
    if (pHost->CreateSurface(&ddsd2, &pFrontBuffer, NULL)) {
        sprintf(pErrorMessage, "Init - Failed to create front buffer.\n");
        if (pHost) {
            pHost->Release();
            pHost = NULL;
        }
        return 0;
    }
    // a3a = &pBackBuffer;
    // v14 = *v34;
    memset(&v27, 0, sizeof(DDSCAPS2));

    v27.dwCaps = DDSCAPS_BACKBUFFER;
    // v33 = (IDirect3DDevice3 **)v14->GetAttachedSurface(&v27, &pBackBuffer);
    // hWnda = &pDirect3D;
    pHost->QueryInterface(IID_IDirect3D3, (LPVOID *)&pDirect3D);

    if (FAILED(pFrontBuffer->GetAttachedSurface(&v27, &pBackBuffer))) {
        sprintf(pErrorMessage, "Init - Failed to get D3D interface.\n");
        if (pBackBuffer) {
            pBackBuffer->Release();
            pBackBuffer = NULL;
        }
        if (pFrontBuffer) {
            pFrontBuffer->Release();
            pFrontBuffer = NULL;
        }
        if (pHost) {
            pHost->Release();
            pHost = NULL;
        }
        return 0;
    }

    ddsd2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
    ddsd2.dwWidth = 640;
    ddsd2.dwHeight = 480;
    if (pDirect3D->EnumZBufferFormats(
            *pAvailableDevices[uDeviceID].pGUID,
            (HRESULT(__stdcall *)(DDPIXELFORMAT *,
                                  void *))D3DZBufferFormatEnumerator,
            &ddsd2.ddpfPixelFormat)) {
        sprintf(pErrorMessage,
                "Init - Failed to enumerate Z buffer formats.\n");
        if (pBackBuffer) {
            pBackBuffer->Release();
            pBackBuffer = 0;
        }
        if (pFrontBuffer) {
            pFrontBuffer->Release();
            pFrontBuffer = 0;
        }
        if (pHost) {
            pHost->Release();
            pHost = 0;
        }
        return 0;
    }
    if (uDeviceID == 2 || uDeviceID == 3) ddsd2.ddsCaps.dwCaps |= 8;
    // uDeviceIDa = &pZBuffer;
    if (pHost->CreateSurface(&ddsd2, &pZBuffer, NULL)) {
        sprintf(pErrorMessage, "Init - Failed to create z-buffer.\n");
        if (pBackBuffer) {
            pBackBuffer->Release();
            pBackBuffer = 0;
        }
        if (pFrontBuffer) {
            pFrontBuffer->Release();
            pFrontBuffer = 0;
        }
        if (pHost) {
            pHost->Release();
            pHost = 0;
        }
        return 0;
    }

    if (pBackBuffer->AddAttachedSurface(pZBuffer)) {
        sprintf(pErrorMessage,
                "Init - Failed to attach z-buffer to back buffer.\n");
        if (pZBuffer) {
            pZBuffer->Release();
            pZBuffer = 0;
        }
        if (pBackBuffer) {
            pBackBuffer->Release();
            pBackBuffer = 0;
        }
        if (pFrontBuffer) {
            pFrontBuffer->Release();
            pFrontBuffer = 0;
        }
        if (pHost) {
            pHost->Release();
            pHost = 0;
        }
        return 0;
    }
    // v33 = &pDevice;
    if (pDirect3D->CreateDevice(*pAvailableDevices[uDeviceID].pGUID,
                                pBackBuffer, &pDevice, 0)) {
        sprintf(pErrorMessage, "Init - Failed to create D3D device.\n");
        if (pDirect3D) {
            pDirect3D->Release();
            pDirect3D = 0;
        }
        if (pZBuffer) {
            pZBuffer->Release();
            pZBuffer = 0;
        }
        if (pBackBuffer) {
            pBackBuffer->Release();
            pBackBuffer = 0;
        }
        if (pFrontBuffer) {
            pFrontBuffer->Release();
            pFrontBuffer = 0;
        }
        if (pHost) {
            pHost->Release();
            pHost = 0;
        }
        return 0;
    }
    memset(&d3dvp2, 0, sizeof(D3DVIEWPORT2));
    d3dvp2.dvClipWidth = 2.0;
    d3dvp2.dvClipY = 1.0;
    d3dvp2.dvClipHeight = 2.0;
    d3dvp2.dvMaxZ = 1.0;

LABEL_54:
    d3dvp2.dwSize = sizeof(D3DVIEWPORT2);
    // v17 = *hWnda;
    d3dvp2.dwWidth = game_width;
    d3dvp2.dwHeight = game_height;
    d3dvp2.dvClipX = -1.0;
    // v18 = v17->lpVtbl;
    // v32 = &v4->pViewport;
    if (pDirect3D->CreateViewport(&pViewport, 0)) {
        sprintf(pErrorMessage, "Init - Failed to create viewport.\n");
        if (pDevice) {
            pDevice->Release();
            pDevice = 0;
        }
        if (pDirect3D) {
            pDirect3D->Release();
            pDirect3D = 0;
        }
        if (pZBuffer) {
            pZBuffer->Release();
            pZBuffer = 0;
        }
        if (pBackBuffer) {
            pBackBuffer->Release();
            pBackBuffer = 0;
        }
        if (pFrontBuffer) {
            pFrontBuffer->Release();
            pFrontBuffer = 0;
        }
        if (pHost) {
            pHost->Release();
            pHost = 0;
        }
        return 0;
    }

    pDevice->AddViewport(pViewport);
    pViewport->SetViewport2(&d3dvp2);
    pDevice->SetCurrentViewport(pViewport);
    return 1;
}

unsigned int RenderD3D::GetDeviceCaps() {
    unsigned int v1;        // ebx@1
    unsigned int result;    // eax@2
    D3DDEVICEDESC refCaps;  // [sp+Ch] [bp-1F8h]@1
    D3DDEVICEDESC halCaps;  // [sp+108h] [bp-FCh]@1

    v1 = 0;

    memset(&halCaps, 0, sizeof(halCaps));
    halCaps.dwSize = sizeof(halCaps);

    memset(&refCaps, 0, sizeof(refCaps));
    refCaps.dwSize = sizeof(refCaps);

    if (this->pDevice->GetCaps(&halCaps, &refCaps)) {
        result = 1;
    } else {
        if (!(halCaps.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA))
            v1 = 2;
        if (!(halCaps.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_ONE)) v1 |= 4;
        if (!(halCaps.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_ZERO)) v1 |= 8;
        if (!(halCaps.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA))
            v1 |= 16;
        if (!(halCaps.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE)) v1 |= 32;
        if (!(halCaps.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_SRCCOLOR))
            v1 |= 64;
        if (halCaps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
            v1 |= 128;
        result = v1;
    }
    return result;
}

void RenderD3D::ClearTarget(unsigned int bClearColor, unsigned int uClearColor,
                            unsigned int bClearDepth, float z_clear) {
    unsigned int uClearFlags = 0;

    if (bClearColor) uClearFlags |= D3DCLEAR_TARGET;
    if (bClearDepth) uClearFlags |= D3DCLEAR_ZBUFFER;

    D3DRECT rects[] = {{0, 0, (LONG)window->GetWidth(), (LONG)window->GetHeight()}};
    if (uClearFlags)
        pViewport->Clear2(1, rects, uClearFlags, uClearColor, z_clear, 0);
}

void RenderD3D::Present(bool bForceBlit) {
    RECT source_rect;       // [sp+18h] [bp-18h]@1
    struct tagPOINT Point;  // [sp+28h] [bp-8h]@4

    source_rect.left = 0;
    source_rect.top = 0;
    source_rect.bottom =
        480;  // window->GetHeight(); //Ritor1: проблема с кнопкой "развернуть"
    source_rect.right = 640;  // window->GetWidth();

    if (bWindowed || bForceBlit) {
        RECT dest_rect;
        GetClientRect(hWindow, &dest_rect);
        Point.y = 0;
        Point.x = 0;
        ClientToScreen(hWindow, &Point);
        OffsetRect(&dest_rect, Point.x, Point.y);
        pFrontBuffer->Blt(&dest_rect, pBackBuffer, &source_rect, DDBLT_WAIT,
                          NULL);
    } else {
        pFrontBuffer->Flip(NULL, DDFLIP_WAIT);
    }
}

unsigned int GetMaxMipLevels(unsigned int uDim) {
    int v2 = 0;
    unsigned int v3 = uDim - 1;
    while (v3 & 1) {
        v3 >>= 1;
        ++v2;
    }
    return v3 == 0 ? v2 : 0;
}

bool RenderD3D::CreateTexture(unsigned int uTextureWidth,
                              unsigned int uTextureHeight,
                              IDirectDrawSurface4 **pOutSurface,
                              IDirect3DTexture2 **pOutTexture,
                              bool bAlphaChannel, bool bMipmaps,
                              unsigned int uMinDeviceTexDim) {
    DDSURFACEDESC2 ddsd2 = { 0 };
    ddsd2.dwSize = sizeof(ddsd2);
    ddsd2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    ddsd2.dwHeight = uTextureHeight;
    ddsd2.dwWidth = uTextureWidth;
    if (bMipmaps) {
        if (uTextureHeight <= uTextureWidth) {
            ddsd2.dwMipMapCount = GetMaxMipLevels(uTextureHeight) -
                                  GetMaxMipLevels(uMinDeviceTexDim);
            if (ddsd2.dwMipMapCount) {
                ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH |
                                DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
                ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
            }
            goto LABEL_12;
        }
        if (uTextureWidth < uMinDeviceTexDim) {
            ddsd2.dwMipMapCount = GetMaxMipLevels(uMinDeviceTexDim);
            if (ddsd2.dwMipMapCount) {
                ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH |
                                DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
                ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
            }
            goto LABEL_12;
        }
        unsigned int v9 = GetMaxMipLevels(uTextureWidth);
        unsigned int v10 = GetMaxMipLevels(uMinDeviceTexDim);
        ddsd2.dwMipMapCount = v9 - v10;
        if (ddsd2.dwMipMapCount == 0) {
            ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
            ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX;
            goto LABEL_12;
        }
    } else {
        ddsd2.dwMipMapCount = 1;
    }
    ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT |
                    DDSD_MIPMAPCOUNT;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
LABEL_12:
    ddsd2.ddpfPixelFormat.dwRGBBitCount = 16;
    ddsd2.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    if (bAlphaChannel) {
        ddsd2.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
        ddsd2.ddpfPixelFormat.dwRBitMask = 0x7C00;
        ddsd2.ddpfPixelFormat.dwGBitMask = 0x03E0;
        ddsd2.ddpfPixelFormat.dwBBitMask = 0x001F;
        ddsd2.ddpfPixelFormat.dwRGBAlphaBitMask = 0x8000;
    } else {
        ddsd2.ddpfPixelFormat.dwFlags = DDPF_RGB;
        ddsd2.ddpfPixelFormat.dwRBitMask = 0xF800;
        ddsd2.ddpfPixelFormat.dwGBitMask = 0x07E0;
        ddsd2.ddpfPixelFormat.dwBBitMask = 0x001F;
        ddsd2.ddpfPixelFormat.dwRGBAlphaBitMask = 0;
    }
    if (FAILED(pHost->CreateSurface(&ddsd2, pOutSurface, NULL))) {
        return false;
    }

    if (FAILED((*pOutSurface)->QueryInterface(IID_IDirect3DTexture2, (void**)pOutTexture))) {
        (*pOutSurface)->Release();
        *pOutSurface = 0;
        return false;
    }
    return true;
}

void RenderD3D::HandleLostResources() {
    pBitmaps_LOD->ReleaseLostHardwareTextures();
    pSprites_LOD->ReleaseLostHardwareSprites();
}
