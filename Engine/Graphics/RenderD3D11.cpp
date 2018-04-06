#include <d3dcompiler.h>
#include <comdef.h> // _com_error

#include "Engine/Engine.h"

#include "Engine/Graphics/Image.h"

#include "RenderD3D11.h"
#include "Sprites.h"

#include "Platform/OsWindow.h"

#define ErrorD3D(x)\
{\
  HRESULT hr = x;\
  if (FAILED(hr))\
  {\
    _com_error com_error(hr);\
    Error("HRESULT = %08X\n%S", hr, com_error.ErrorMessage());\
  }\
}


RenderD3D11::RenderD3D11() : IRender() { this->ui_clip_rect = new D3D11_RECT; }
RenderD3D11::~RenderD3D11() {}


bool CompileShader(ID3D11Device *d3dd, const wchar_t *pShaderSourceFile, D3D11_INPUT_ELEMENT_DESC *input_desc, int input_desc_size,
                   ID3D11VertexShader **vertex_out, ID3D11PixelShader **pixel_out, ID3D11InputLayout **layout_out);


unsigned int RenderD3D11::GetRenderWidth() const { return window->GetWidth(); }
unsigned int RenderD3D11::GetRenderHeight() const { return window->GetHeight(); }

Texture *RenderD3D11::CreateTexture(const String &name) { __debugbreak(); return nullptr; }
void RenderD3D11::ClearBlack() {__debugbreak();}
void RenderD3D11::SaveWinnersCertificate(const char *a1) {__debugbreak();}
void RenderD3D11::_49FD3A_fullscreen() {__debugbreak();}
bool RenderD3D11::InitializeFullscreen() {__debugbreak(); return 0;}
void RenderD3D11::CreateZBuffer() {__debugbreak();}
void RenderD3D11::Release() {__debugbreak();}
void RenderD3D11::RasterLine2D(signed int uX, signed int uY, signed int uZ, signed int uW, unsigned __int16 uColor) {__debugbreak();}
bool RenderD3D11::LockSurface_DDraw4(IDirectDrawSurface4 *pSurface, DDSURFACEDESC2 *pDesc, unsigned int uLockFlags) {__debugbreak(); return 0;}
void RenderD3D11::LockRenderSurface(void **pOutSurfacePtr, unsigned int *pOutPixelsPerRow) {__debugbreak();}
void RenderD3D11::UnlockBackBuffer() {__debugbreak();}
void RenderD3D11::LockFrontBuffer(void **pOutSurface, unsigned int *pOutPixelsPerRow) {__debugbreak();}
void RenderD3D11::UnlockFrontBuffer() {__debugbreak();}
void RenderD3D11::RestoreFrontBuffer() {}
void RenderD3D11::RestoreBackBuffer() {}
void RenderD3D11::BltBackToFontFast(int a2, int a3, Rect *a4) {__debugbreak();}
void RenderD3D11::BeginSceneD3D() {}
unsigned int RenderD3D11::GetActorTintColor(float a2, int tint, int a4, int a5, RenderBillboard *a6) {__debugbreak(); return 0;}
void RenderD3D11::DrawPolygon(struct Polygon *a3) {__debugbreak();}
void RenderD3D11::DrawTerrainPolygon(struct Polygon *a4, bool transparent, bool clampAtTextureBorders) {__debugbreak();}
void RenderD3D11::DrawIndoorPolygon(unsigned int uNumVertices, struct BLVFace *a3, int uPackedID, unsigned int uColor, int a8) {__debugbreak();}
void RenderD3D11::MakeParticleBillboardAndPush_BLV(SoftwareBillboard *a2, Texture *a3, unsigned int uDiffuse, int angle) {__debugbreak();}
void RenderD3D11::MakeParticleBillboardAndPush_ODM(SoftwareBillboard *a2, Texture *a3, unsigned int uDiffuse, int angle) {__debugbreak();}
void RenderD3D11::DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene() {__debugbreak();}
void RenderD3D11::DrawBillboard_Indoor(SoftwareBillboard *pSoftBillboard, Sprite *pSprite, int dimming_level) {__debugbreak();}
void RenderD3D11::_4A4CC9_AddSomeBillboard(struct stru6_stru1_indoor_sw_billboard *a1, int diffuse) {__debugbreak();}
void RenderD3D11::TransformBillboardsAndSetPalettesODM() {__debugbreak();}
void RenderD3D11::DrawBillboardList_BLV() {__debugbreak();}
void RenderD3D11::DrawProjectile(float srcX, float srcY, float a3, float a4, float dstX, float dstY, float a7, float a8, Texture *texture) {__debugbreak();}
void RenderD3D11::ScreenFade(unsigned int color, float t) {__debugbreak();}
void RenderD3D11::DrawTextureOffset(int pX, int pY, int move_X, int move_Y, Image *pTexture) {__debugbreak();}
void RenderD3D11::ZBuffer_Fill_2(signed int a2, signed int a3, Image *pTexture, int a5) {__debugbreak();}
void RenderD3D11::ZDrawTextureAlpha(float u, float v, Image *pTexture, int zVal) {__debugbreak();}
void RenderD3D11::BlendTextures(int a2, int a3, Image *a4, Image *a5, int t, int start_opacity, int end_opacity) {__debugbreak();}
void RenderD3D11::_4A65CC(unsigned int x, unsigned int y, Image *a4, Image *a5, int a6, int a7, int a8) {__debugbreak();}
void RenderD3D11::DrawTransparentRedShade(float u, float v, Image *a4) {__debugbreak();}
void RenderD3D11::DrawTransparentGreenShade(float u, float v, Image *pTexture) {__debugbreak();}
void RenderD3D11::DrawFansTransparent(const RenderVertexD3D3 *vertices, unsigned int num_vertices) {__debugbreak();}
void RenderD3D11::DrawMasked(float u, float v, Image *pTexture, unsigned int, unsigned __int16 mask) {__debugbreak();}
void RenderD3D11::DrawTextureCustomHeight(float u, float v, Image *img, int height) {__debugbreak();}
void RenderD3D11::FillRectFast(unsigned int uX, unsigned int uY, unsigned int uWidth, unsigned int uHeight, unsigned int uColor16) {__debugbreak();}
void RenderD3D11::DrawTextureGrayShade(float a2, float a3, Image *a4) {__debugbreak();}
void RenderD3D11::DrawBuildingsD3D() {__debugbreak();}
void RenderD3D11::DrawIndoorSky(unsigned int uNumVertices, unsigned int uFaceID) {__debugbreak();}
void RenderD3D11::DrawOutdoorSkyD3D() {__debugbreak();}
void RenderD3D11::DrawOutdoorSkyPolygon(struct Polygon *pSkyPolygon) {__debugbreak();}
void RenderD3D11::DrawIndoorSkyPolygon(signed int uNumVertices, struct Polygon *pSkyPolygon) {__debugbreak();}
void RenderD3D11::PrepareDecorationsRenderList_ODM() {__debugbreak();}
void RenderD3D11::DrawSpriteObjects_ODM() {__debugbreak();}
void RenderD3D11::RenderTerrainD3D() {__debugbreak();}
bool RenderD3D11::AreRenderSurfacesOk() {return true;}
void RenderD3D11::SaveScreenshot(const String &filename, unsigned int width, unsigned int height) {__debugbreak();}
void RenderD3D11::SavePCXScreenshot() {__debugbreak();}
int RenderD3D11::_46ภ6ภั_GetActorsInViewport(int pDepth) {__debugbreak(); return 0;}
void RenderD3D11::BeginLightmaps() {__debugbreak();}
void RenderD3D11::EndLightmaps() {__debugbreak();}
void RenderD3D11::BeginLightmaps2() {__debugbreak();}
void RenderD3D11::EndLightmaps2() {__debugbreak();}
bool RenderD3D11::DrawLightmap(struct Lightmap *pLightmap, struct Vec3_float_ *pColorMult, float z_bias) {__debugbreak(); return 0;}
void RenderD3D11::BeginDecals() {__debugbreak();}
void RenderD3D11::EndDecals() {__debugbreak();}
void RenderD3D11::DrawDecal(struct Decal *pDecal, float z_bias) {__debugbreak();}
void RenderD3D11::do_draw_debug_line_d3d(const RenderVertexD3D3 *pLineBegin, signed int sDiffuseBegin, const RenderVertexD3D3 *pLineEnd, signed int sDiffuseEnd, float z_stuff) {__debugbreak();}
void RenderD3D11::DrawLines(const RenderVertexD3D3 *vertices, unsigned int num_vertices) {__debugbreak();}
void RenderD3D11::DrawSpecialEffectsQuad(const RenderVertexD3D3 *vertices, Texture *texture) {__debugbreak();}
void RenderD3D11::am_Blt_Copy(Rect *pSrcRect, Point *pTargetXY, int blend_mode) {__debugbreak();}
void RenderD3D11::am_Blt_Chroma(Rect *pSrcRect, Point *pTargetPoint, int a3, int blend_mode) {__debugbreak();}

void RenderD3D11::Sub01() {__debugbreak();}




void RenderD3D11::PackScreenshot(unsigned int width, unsigned int height, void *out_data, unsigned int data_size, unsigned int *screenshot_size) {}

void RenderD3D11::SetUIClipRect(unsigned int uX, unsigned int uY, unsigned int uZ, unsigned int uW)
{
  ui_clip_rect->left = uX;
  ui_clip_rect->top = uY;
  ui_clip_rect->right = uZ;
  ui_clip_rect->bottom = uW;
}

void RenderD3D11::ResetUIClipRect()
{
  ui_clip_rect->left = 0;
  ui_clip_rect->top = 0;
  ui_clip_rect->right = window->GetWidth();
  ui_clip_rect->bottom = window->GetHeight();
}

void RenderD3D11::PresentBlackScreen()
{
    ClearTarget(0xFF000000);
    ErrorD3D(pSwapChain->Present(0, 0));
}

void RenderD3D11::BeginScene() {}
void RenderD3D11::EndScene() {}

void RenderD3D11::ClearTarget(unsigned int uColor)
{
    float clear_color[] =
    {
      ((uColor & 0x00FF0000) >> 16) / 255.0f,
      ((uColor & 0x0000FF00) >> 8) / 255.0f,
      ((uColor & 0x000000FF) >> 0) / 255.0f,
      ((uColor & 0xFF000000) >> 24) / 255.0f
    };
    d3dc->ClearRenderTargetView(primary_srv, clear_color);
}

void RenderD3D11::ClearZBuffer(int, int)
{
    d3dc->ClearDepthStencilView(default_depth_srv, D3D11_CLEAR_DEPTH, 1.0f, 0);
}




void RenderD3D11::DrawTextureAlphaNew(float u, float v, Image *img)
{
    __debugbreak();
}


void RenderD3D11::DrawTextureNew(float u, float v, Image *tex)
{
    __debugbreak();
}

void RenderD3D11::DrawText(signed int uX, signed int uY, unsigned __int8 *pFontPixels, unsigned int uCharWidth, unsigned int uCharHeight, unsigned __int16 *pFontPalette, unsigned __int16 uFaceColor, unsigned __int16 uShadowColor)
{
    auto srv = PrepareFontTexture(pFontPixels, uCharWidth, uCharHeight, pFontPalette, uFaceColor, uShadowColor);
    {
        DrawTexture((float)uX / window->GetWidth(), (float)uY / window->GetHeight(), uCharWidth, uCharHeight, srv, ui_blend_alpha);
    }
    srv->Release();
}

void RenderD3D11::DrawTextAlpha(int x, int y, unsigned char* font_pixels, int a5, unsigned int uFontHeight, unsigned __int16 *pPalette, bool present_time_transparency)
{
    auto srv = PrepareFontTexture(font_pixels, a5, uFontHeight, pPalette);
    {
        DrawTexture((float)x / window->GetWidth(), (float)y / window->GetHeight(), a5, uFontHeight, srv, ui_blend_alpha);
    }
    srv->Release();
}

void RenderD3D11::DrawTexture(float u, float v, int texture_width, int texture_height, ID3D11ShaderResourceView *srv, ID3D11BlendState *blend)
{
  bool clipping = false;
  if (ui_clip_rect->left != 0 || ui_clip_rect->top != 0
      || ui_clip_rect->right != window->GetWidth()  || ui_clip_rect->bottom != window->GetHeight())
      clipping = true;

  float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  d3dc->OMSetRenderTargets(1, &primary_srv, default_depth_srv);
  d3dc->OMSetBlendState(blend, blendFactor, 0xFFFFFFFF);
  d3dc->OMSetDepthStencilState(ui_depthstencil, 1);

  {
    struct cb_fast
    {
      float pos_x;
      float pos_y;
      float size_x;
      float size_y;
    };
    
    D3D11_MAPPED_SUBRESOURCE map;
    d3dc->Map(ui_cb_fast, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
    {
      cb_fast data;
      {
        data.pos_x = u;
        data.pos_y = v;
        data.size_x = (float)texture_width / window->GetWidth();
        data.size_y = (float)texture_height / window->GetHeight();
      }
      memcpy(map.pData, &data, sizeof(data));
    }
    d3dc->Unmap(ui_cb_fast, 0);
  }
  
  d3dc->VSSetShader(ui_vs, nullptr, 0);
  d3dc->VSSetConstantBuffers(0, 1, &ui_cb_fast);

  d3dc->PSSetShader(ui_ps, nullptr, 0);
  d3dc->PSSetConstantBuffers(0, 1, &ui_cb_fast);
  d3dc->PSSetShaderResources(0, 1, &srv);

  if (clipping)
  {
    d3dc->RSSetState(ui_rasterizer);
    d3dc->RSSetScissorRects(1, ui_clip_rect);
  }
  //d3dc->RSSetViewports(1, &ui_viewport);

  uint uOffset = 0;
  uint uStride = 4 * sizeof(float);
  d3dc->IASetVertexBuffers(0, 1, &ui_vb, &uStride, &uOffset);
  d3dc->IASetInputLayout(ui_layout);
  d3dc->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  d3dc->Draw(6, 0);

  if (clipping)
  {
    d3dc->RSSetState(default_rasterizer);
    //d3dc->RSSetScissorRects(0, nullptr);
  }
}



void RenderD3D11::Present()
{
  pSwapChain->Present(0, 0);
}



struct
{
  unsigned char restore_resolution;
  unsigned char  _saved_screen_bpp;
  unsigned short _saved_screen_width;
  unsigned short _saved_screen_height;
} on_exit;

void ChangeResolution(int new_width, int new_height, int new_bpp)
{
  if (!on_exit.restore_resolution)
  {
    on_exit.restore_resolution = true;

    auto hdc = GetDC(nullptr);
    {
      on_exit._saved_screen_width = GetDeviceCaps(hdc, HORZRES);
      on_exit._saved_screen_height = GetDeviceCaps(hdc, VERTRES);
      on_exit._saved_screen_bpp = GetDeviceCaps(hdc, BITSPIXEL);
    }
    ReleaseDC(nullptr, hdc);
  }

  DEVMODEA dm;
  dm.dmSize = sizeof(dm);
  dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
  dm.dmBitsPerPel = new_bpp;
  dm.dmPelsWidth = new_width;
  dm.dmPelsHeight = new_height;
      
  if (ChangeDisplaySettingsA(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
    Error("ChangeDisplaySettingsA");
}

__declspec(noreturn) void ExitApp()
{
  if (on_exit.restore_resolution)
    ChangeResolution(on_exit._saved_screen_width, on_exit._saved_screen_height, on_exit._saved_screen_bpp);

  ExitProcess(0);
}



bool RenderD3D11::SwitchToWindow()
{
  if (on_exit.restore_resolution)
  {
    on_exit.restore_resolution = false;
    ChangeResolution(on_exit._saved_screen_width, on_exit._saved_screen_height, on_exit._saved_screen_bpp);
  }
  return true;
}




bool RenderD3D11::Initialize(OSWindow *window)
{
  this->window = window;

  auto d3d_lib = LoadLibraryW(L"d3d11.dll");
  if (!d3d_lib)
  {
    Error("d3d11.dll is missing");
    return false;
  }


  DXGI_SWAP_CHAIN_DESC swapChainDesc;
  memset(&swapChainDesc, 0, sizeof(swapChainDesc));
  swapChainDesc.BufferDesc.Width = window->GetWidth();
  swapChainDesc.BufferDesc.Height = window->GetHeight();
  //swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
  //swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
  swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  //swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  //swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  swapChainDesc.SampleDesc.Count = 1;
  //swapChainDesc.SampleDesc.Quality = 0;
  swapChainDesc.BufferUsage =  DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = 2;
  swapChainDesc.OutputWindow = (HWND)window->GetApiHandle();
  swapChainDesc.Windowed = true;
  //swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  //swapChainDesc.Flags = 0;


  D3D_FEATURE_LEVEL requested_feature_level =  D3D_FEATURE_LEVEL_11_0,
                    received_feature_level;

  unsigned int device_flags = 0;//D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
  #ifndef NODEBUG
    device_flags |= D3D11_CREATE_DEVICE_DEBUG;
  #endif

  HRESULT (__stdcall *dll_D3D11CreateDeviceAndSwapChain)(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);
  dll_D3D11CreateDeviceAndSwapChain = (decltype(dll_D3D11CreateDeviceAndSwapChain))GetProcAddress(d3d_lib, "D3D11CreateDeviceAndSwapChain");
  ErrorD3D(dll_D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, device_flags, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &d3dd, &received_feature_level, &d3dc));

  if (received_feature_level < D3D_FEATURE_LEVEL_10_0)
  {
    MessageBoxA(nullptr, "Received Direct3D 9 or lower", "", 0);
    __debugbreak();
  }
      /*if (fullscreen)
      {
        extern void ChangeResolution(int new_dith, int new_height, int new_bpp);
        ChangeResolution(target_window->GetWidth(), target_window->GetHeight(), 32);

        target_window->SetPosition(0, 0);
        target_window->SetTopmost(true);
      }
      ErrorD3D(pSwapChain->SetFullscreenState(fullscreen, nullptr));*/

  
  ID3D11Texture2D *pSwapChainSurface;
  {
    ErrorD3D(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pSwapChainSurface));
    ErrorD3D(d3dd->CreateRenderTargetView(pSwapChainSurface, nullptr, &primary_srv));
  }
  pSwapChainSurface->Release();


  D3D11_TEXTURE2D_DESC z_desc;
  memset(&z_desc, 0, sizeof(z_desc));
  z_desc.Width = window->GetWidth();
  z_desc.Height = window->GetHeight();
  z_desc.MipLevels = 1;
  z_desc.ArraySize = 1;
  z_desc.Format = DXGI_FORMAT_D32_FLOAT;
  z_desc.SampleDesc.Count = 1;
  //z_desc.SampleDesc.Quality = 0;
  //z_desc.Usage = D3D11_USAGE_DEFAULT;
  z_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  //z_desc.CPUAccessFlags = 0;
  //z_desc.MiscFlags = 0;

  ID3D11Texture2D *depth_surface;
  ErrorD3D(d3dd->CreateTexture2D(&z_desc, nullptr, &depth_surface));

  D3D11_DEPTH_STENCIL_VIEW_DESC depth_srv_desc;
  memset(&depth_srv_desc, 0, sizeof(depth_srv_desc));
  depth_srv_desc.Format = DXGI_FORMAT_D32_FLOAT;
  depth_srv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  //depth_srv_desc.Texture2D.MipSlice = 0;
  ErrorD3D(d3dd->CreateDepthStencilView(depth_surface, &depth_srv_desc, &default_depth_srv));

  d3dc->OMSetRenderTargets(1, &primary_srv, default_depth_srv);
  d3dc->ClearDepthStencilView(default_depth_srv, D3D11_CLEAR_DEPTH, 1.0f, 0);



  D3D11_RASTERIZER_DESC default_rasterizer_desc;
  memset(&default_rasterizer_desc, 0, sizeof(default_rasterizer_desc));
  default_rasterizer_desc.FillMode = D3D11_FILL_SOLID;
  default_rasterizer_desc.CullMode = D3D11_CULL_NONE;
  //default_rasterizer_desc.FrontCounterClockwise = false;
  //default_rasterizer_desc.DepthBias = 0;
  //default_rasterizer_desc.DepthBiasClamp = 0.0f;
  //default_rasterizer_desc.SlopeScaledDepthBias = 0.0f;
  //default_rasterizer_desc.DepthClipEnable = true;
  //default_rasterizer_desc.ScissorEnable = false;
  //default_rasterizer_desc.MultisampleEnable = false;
  //default_rasterizer_desc.AntialiasedLineEnable = false;

  ErrorD3D(d3dd->CreateRasterizerState(&default_rasterizer_desc, &default_rasterizer));
  d3dc->RSSetState(default_rasterizer);



  D3D11_RASTERIZER_DESC ui_rasterizer_desc;
  memset(&ui_rasterizer_desc, 0, sizeof(ui_rasterizer_desc));
  ui_rasterizer_desc.FillMode = D3D11_FILL_SOLID;
  ui_rasterizer_desc.CullMode = D3D11_CULL_NONE;
  //ui_rasterizer_desc.FrontCounterClockwise = false;
  //ui_rasterizer_desc.DepthBias = 0;
  //ui_rasterizer_desc.DepthBiasClamp = 0.0f;
  //ui_rasterizer_desc.SlopeScaledDepthBias = 0.0f;
  //ui_rasterizer_desc.DepthClipEnable = true;
  ui_rasterizer_desc.ScissorEnable = true;
  //ui_rasterizer_desc.MultisampleEnable = false;
  //ui_rasterizer_desc.AntialiasedLineEnable = false;

  ErrorD3D(d3dd->CreateRasterizerState(&ui_rasterizer_desc, &ui_rasterizer));


  default_depthstencil = nullptr;
  default_blend = nullptr;



  D3D11_VIEWPORT viewport;
  memset(&viewport, 0, sizeof(viewport));
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width    = window->GetWidth();
  viewport.Height   = window->GetHeight();
  viewport.MinDepth = 0;
  viewport.MaxDepth = 1;
  d3dc->RSSetViewports(1, &viewport);

  ResetUIClipRect();


  D3D11_INPUT_ELEMENT_DESC layout_desc[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    //{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
  };
  CompileShader(d3dd, L"data/shaders/UI.hlsl", layout_desc, 1/*2*/, &ui_vs, &ui_ps, &ui_layout);

      {
        uint uVertexSize = 4 * sizeof(float);

        float ui_mesh[] =
        {
          1, 1, 0, 0, // pos
          1, 1, 1, 0, // pos + size.x
          1, 1, 0, 1, // pos + size.y

          1, 1, 0, 1, // pos + size.y
          1, 1, 1, 0, // pos + size.x
          1, 1, 1, 1  // pos + size.xy
          /*-1,  1, 0.1, 1,
           1,  1, 0.1, 1,
          -1, -1, 0.1, 1,

          -1, -1, 0.1, 1,
           1,  1, 0.1, 1,
           1, -1, 0.1, 1*/
        };

        D3D11_BUFFER_DESC vbdesc;
        vbdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbdesc.ByteWidth = 6 * uVertexSize;
        vbdesc.CPUAccessFlags = 0;
        vbdesc.MiscFlags = 0;
        vbdesc.StructureByteStride = 0;
        vbdesc.Usage = D3D11_USAGE_DEFAULT;

        D3D11_SUBRESOURCE_DATA vbdata;
        vbdata.pSysMem = ui_mesh;
        vbdata.SysMemPitch = 0;
        vbdata.SysMemSlicePitch = 0;

        d3dd->CreateBuffer(&vbdesc, &vbdata, &ui_vb);
      }

  D3D11_BUFFER_DESC ui_cb_fast_desc;
  ZeroMemory(&ui_cb_fast_desc, sizeof(ui_cb_fast_desc));
  // TODO: Fix Bug Prone size
  ui_cb_fast_desc.ByteWidth = 1 * 4 * sizeof(float);
  ui_cb_fast_desc.Usage = D3D11_USAGE_DYNAMIC;
  ui_cb_fast_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  ui_cb_fast_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  ui_cb_fast_desc.MiscFlags = 0;
  ui_cb_fast_desc.StructureByteStride = 0;
  d3dd->CreateBuffer(&ui_cb_fast_desc, nullptr, &ui_cb_fast);

  D3D11_DEPTH_STENCIL_DESC ui_depthstencil_desc;
  ui_depthstencil_desc.DepthEnable = true;
  ui_depthstencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  ui_depthstencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
  ui_depthstencil_desc.StencilEnable = false;
  ui_depthstencil_desc.StencilReadMask = 0xFF;
  ui_depthstencil_desc.StencilWriteMask = 0xFF;
  // Stencil operations if pixel is front-facing
  ui_depthstencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  ui_depthstencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  ui_depthstencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  ui_depthstencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
  // Stencil operations if pixel is back-facing
  ui_depthstencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  ui_depthstencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  ui_depthstencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  ui_depthstencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  ErrorD3D(d3dd->CreateDepthStencilState(&ui_depthstencil_desc, &ui_depthstencil));


  D3D11_BLEND_DESC ui_blend_solid_desc;
  memset(&ui_blend_solid_desc, 0, sizeof(ui_blend_solid_desc));
  ui_blend_solid_desc.RenderTarget[0].BlendEnable = false;
  ui_blend_solid_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  ErrorD3D(d3dd->CreateBlendState(&ui_blend_solid_desc, &ui_blend_solid));

  D3D11_BLEND_DESC ui_blend_alpha_desc;
  memset(&ui_blend_alpha_desc, 0, sizeof(ui_blend_alpha_desc));
  ui_blend_alpha_desc.RenderTarget[0].BlendEnable = true;
  ui_blend_alpha_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  ui_blend_alpha_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  ui_blend_alpha_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  ui_blend_alpha_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  ui_blend_alpha_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  ui_blend_alpha_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  ui_blend_alpha_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
  ErrorD3D(d3dd->CreateBlendState(&ui_blend_alpha_desc, &ui_blend_alpha));


  return pD3DBitmaps.Load(MakeDataPath("data\\d3dbitmap.hwl").c_str()) &&
         pD3DSprites.Load(MakeDataPath("data\\d3dsprite.hwl").c_str());
}


HWLTexture *RenderD3D11::LoadHwlBitmap(const char *name)
{
    return pD3DBitmaps.LoadTexture(name, 0);
}

HWLTexture *RenderD3D11::LoadHwlSprite(const char *name)
{
    return pD3DSprites.LoadTexture(name, 0);
}


/*bool RenderD3D11::MoveSpriteToDevice(Sprite *pSprite)
{
  HWLTexture *sprite_texture; // eax@1
  unsigned __int16 *v9; // edx@5
  LPVOID v10; // eax@5
  DDSURFACEDESC2 Dst; // [sp+Ch] [bp-7Ch]@4

  sprite_texture = pD3DSprites.LoadTexture(pSprite->pName, pSprite->uPaletteID);
  if ( sprite_texture )
  {
    pSprite->uAreaX = sprite_texture->uAreaX;
    pSprite->uAreaY = sprite_texture->uAreaY;
    pSprite->uBufferWidth = sprite_texture->uBufferWidth;
    pSprite->uBufferHeight = sprite_texture->uBufferHeight;
    pSprite->uAreaWidth = sprite_texture->uAreaWidth;
    pSprite->uAreaHeight = sprite_texture->uAreaHeigth;

    {
      D3D11_TEXTURE2D_DESC desc;
      desc.Width = sprite_texture->uWidth;
      desc.Height = sprite_texture->uHeight;
      desc.ArraySize = 1;
      desc.MipLevels = 1;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.SampleDesc.Count = 1;
      desc.SampleDesc.Quality = 0;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
      desc.CPUAccessFlags = 0;
      desc.MiscFlags = 0;
    
      ID3D11Texture2D *vram_texture = nullptr;
      ErrorD3D(d3dd->CreateTexture2D(&desc, nullptr, &vram_texture));

      D3D11_TEXTURE2D_DESC ram_desc;
      memcpy(&ram_desc, &desc, sizeof(ram_desc));
      ram_desc.Usage = D3D11_USAGE_STAGING;
      ram_desc.BindFlags = 0;
      ram_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

      ID3D11Texture2D *ram_texture = nullptr;
      ErrorD3D(d3dd->CreateTexture2D(&ram_desc, nullptr, &ram_texture));

      D3D11_MAPPED_SUBRESOURCE map;
      ErrorD3D(d3dc->Map(ram_texture, 0, D3D11_MAP_WRITE, 0, &map));
      for (unsigned int y = 0; y < desc.Height; ++y)
        for (unsigned int x = 0; x < desc.Width; ++x)
        {
          auto src = sprite_texture->pPixels + y * desc.Width + x;
          auto dst = (unsigned int *)((char *)map.pData + y * map.RowPitch) + x;

          extern unsigned __int32 Color32_SwapRedBlue(unsigned __int16 color16);
          *dst = 0xFF000000 | Color32_SwapRedBlue(*src);
        }


      d3dc->Unmap(ram_texture, 0);
      d3dc->CopyResource(vram_texture, ram_texture);
      ram_texture->Release();
    
      ID3D11ShaderResourceView *srv = nullptr;
      ErrorD3D(d3dd->CreateShaderResourceView(vram_texture, nullptr, &srv));
      vram_texture->Release();

      pSprite->d3d11_srv = srv;
    }

    delete [] sprite_texture->pPixels;
    delete sprite_texture;
	return true;
  }
  return false;
}*/


ID3DBlob *DoD3DCompiling(const wchar_t *shader_name, const char *pShaderSource, uint uShaderSourceLen, const char *pEntry, const char *pVersionString, uint uCompileOptions);
void DoCompile(const wchar_t *pFilename, const char *pShaderSource, unsigned int uShaderSourceLen,
               unsigned char **vs, unsigned int *vs_size, unsigned char **ps, unsigned int *ps_size);
bool CompileShader(ID3D11Device *d3dd, const wchar_t *pShaderSourceFile, D3D11_INPUT_ELEMENT_DESC *input_desc, int input_desc_size,
                   ID3D11VertexShader **vertex_out, ID3D11PixelShader **pixel_out, ID3D11InputLayout **layout_out)
{
  unsigned char *vs, *ps;
  unsigned int vs_size, ps_size;

  HANDLE hSourceFile = CreateFileW(pShaderSourceFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
  if (hSourceFile == INVALID_HANDLE_VALUE)
    Error("%S:\n\nCannot read shader file", pShaderSourceFile);

  char *pShaderSource = nullptr;
  int   uShaderSourceLen = 0;
  uint uFileSize = GetFileSize(hSourceFile, nullptr);
  {
    DWORD w;
    char *p = new char[uFileSize + 1];
    ReadFile(hSourceFile, p, uFileSize, &w, nullptr);
    CloseHandle(hSourceFile);
    p[uFileSize] = 0;

    pShaderSource = p;
    uShaderSourceLen = uFileSize;
  }

  DoCompile(pShaderSourceFile, pShaderSource, uShaderSourceLen, &vs, &vs_size, &ps, &ps_size);

  ErrorD3D(d3dd->CreateVertexShader(vs, vs_size, nullptr, vertex_out));
  ErrorD3D(d3dd->CreatePixelShader(ps, ps_size, nullptr, pixel_out));
  ErrorD3D(d3dd->CreateInputLayout(input_desc, input_desc_size, vs, vs_size, layout_out));

  delete [] vs;
  delete [] ps;

  return true;
}



static HMODULE compiler_dll = nullptr;
void DoCompile(const wchar_t *pFilename, const char *pSahderSource, unsigned int uShaderSourceLen,
               unsigned char **vs, unsigned int *vs_size, unsigned char **ps, unsigned int *ps_size)
{
  const char *pVSEntry = "vs",
             *pVSVersionString = "vs_4_0";
  const char *pPSEntry = "main",
             *pPSVersionString = "ps_4_0";


  uint uVSCompileOptions =
  #ifndef NDEBUG
    D3D10_SHADER_SKIP_VALIDATION     |
    D3D10_SHADER_SKIP_OPTIMIZATION   |
    D3D10_SHADER_PREFER_FLOW_CONTROL;
  #else
    D3D10_SHADER_SKIP_VALIDATION     |
    D3D10_SHADER_OPTIMIZATION_LEVEL3 |
    D3D10_SHADER_WARNINGS_ARE_ERRORS |
    D3D10_SHADER_AVOID_FLOW_CONTROL  |
    D3D10_SHADER_ENABLE_STRICTNESS   |
    D3D10_SHADER_IEEE_STRICTNESS;
  #endif
  
  *vs_size = 0;
  *vs = nullptr;
  {
    auto pVSBlob = DoD3DCompiling(pFilename, pSahderSource, uShaderSourceLen, pVSEntry, pVSVersionString, uVSCompileOptions);
    if (pVSBlob)
    {
      *vs = new unsigned char[*vs_size = pVSBlob->GetBufferSize()];
      memcpy(*vs, pVSBlob->GetBufferPointer(), *vs_size);

      pVSBlob->Release();
    }
  }


  uint uPSCompileOptions =
  #ifndef NDEBUG
    D3D10_SHADER_SKIP_VALIDATION     |
    D3D10_SHADER_SKIP_OPTIMIZATION   |
    D3D10_SHADER_PREFER_FLOW_CONTROL;
  #else
    D3D10_SHADER_SKIP_VALIDATION     |
    D3D10_SHADER_OPTIMIZATION_LEVEL3 |
    D3D10_SHADER_WARNINGS_ARE_ERRORS |
    D3D10_SHADER_AVOID_FLOW_CONTROL  |
    D3D10_SHADER_ENABLE_STRICTNESS   |
    D3D10_SHADER_IEEE_STRICTNESS;
  #endif

  *ps_size = 0;
  *ps = nullptr;
  {
    auto pPSBlob = DoD3DCompiling(pFilename, pSahderSource, uShaderSourceLen, pPSEntry, pPSVersionString, uPSCompileOptions);
    if (pPSBlob)
    {
      *ps = new unsigned char[*ps_size = pPSBlob->GetBufferSize()];
      memcpy(*ps, pPSBlob->GetBufferPointer(), *ps_size);

      pPSBlob->Release();
    }
  }
}

ID3DBlob *DoD3DCompiling(const wchar_t *shader_name, const char *pShaderSource, uint uShaderSourceLen, const char *pEntry, const char *pVersionString, uint uCompileOptions)
{
  ID3DBlob *pShader,
           *pErrors;

  if (!compiler_dll)
    for (int i = 45; i >= 38; --i)
    {
      wchar_t dll_name[64];
      swprintf(dll_name, L"d3dcompiler_%u.dll", i);

      compiler_dll = LoadLibraryW(dll_name);
      if (compiler_dll)
        break;
    }

  if (!compiler_dll)
    Error("Cannot find any suitable d3dcompiler.dll");

  typedef HRESULT (__stdcall *fnD3DCompile)(const void * pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, CONST D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);
  auto pD3DCompile  = (fnD3DCompile)GetProcAddress(compiler_dll, "D3DCompile");

  HRESULT hr = pD3DCompile(pShaderSource, uShaderSourceLen, nullptr, nullptr, nullptr, pEntry, pVersionString, uCompileOptions, 0, &pShader, &pErrors);

  if (FAILED(hr))
  {
    if (pErrors)
      Error("%S (%s) build failed:\n\n%s", shader_name, pVersionString, pErrors->GetBufferPointer());
    else
      Error("%S (%s) build failed", shader_name, pVersionString);
  }
  else if (pErrors)
  {
    logger->Warning(L"%s (%S) build warnings:\n\n%S", shader_name, pVersionString, pErrors->GetBufferPointer());
    pErrors->Release();
  }

  return pShader;
}



void RenderD3D11::PrepareTextureIndexed(Texture_MM7 *p)
{
  if (!p->d3d11_srv)
  {
    auto desc = p->d3d11_desc = new D3D11_TEXTURE2D_DESC;
    desc->Width = p->uTextureWidth;
    desc->Height = p->uTextureHeight;
    desc->ArraySize = 1;
    desc->MipLevels = 1;
    desc->Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc->SampleDesc.Count = 1;
    desc->SampleDesc.Quality = 0;
    desc->Usage = D3D11_USAGE_DEFAULT;
    desc->BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc->CPUAccessFlags = 0;
    desc->MiscFlags = 0;
    
    ID3D11Texture2D *vram_texture = nullptr;
    ErrorD3D(d3dd->CreateTexture2D(desc, nullptr, &vram_texture));

    D3D11_TEXTURE2D_DESC ram_desc;
    memcpy(&ram_desc, desc, sizeof(ram_desc));
    ram_desc.Usage = D3D11_USAGE_STAGING;
    ram_desc.BindFlags = 0;
    ram_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    ID3D11Texture2D *ram_texture = nullptr;
    ErrorD3D(d3dd->CreateTexture2D(&ram_desc, nullptr, &ram_texture));

    D3D11_MAPPED_SUBRESOURCE map;
    ErrorD3D(d3dc->Map(ram_texture, 0, D3D11_MAP_WRITE, 0, &map));
    for (unsigned int y = 0; y < desc->Height; ++y)
      for (unsigned int x = 0; x < desc->Width; ++x)
      {
        auto index = p->paletted_pixels[y * p->uTextureWidth + x];
        auto src = p->pPalette16[index];
        auto dst = (unsigned int *)((char *)map.pData + y * map.RowPitch) + x;

        if (index)
        {
          extern unsigned __int32 Color32_SwapRedBlue(unsigned __int16 color16);
          *dst = 0xFF000000 | Color32_SwapRedBlue(src);
        }
        else
          *dst = 0x00000000;
      }

    d3dc->Unmap(ram_texture, 0);
    d3dc->CopyResource(vram_texture, ram_texture);
    ram_texture->Release();
    
    ErrorD3D(d3dd->CreateShaderResourceView(vram_texture, nullptr, (ID3D11ShaderResourceView **)&p->d3d11_srv));
    vram_texture->Release();
  }
}


ID3D11ShaderResourceView *RenderD3D11::PrepareFontTexture(unsigned char *pFontPixels, unsigned int uCharWidth, unsigned int uCharHeight, unsigned short *pFontPalette, unsigned short uFaceColor, unsigned short uShadowColor)
{
  D3D11_TEXTURE2D_DESC desc;
  desc.Width = uCharWidth;
  desc.Height = uCharHeight;
  desc.ArraySize = 1;
  desc.MipLevels = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
    
  ID3D11Texture2D *vram_texture = nullptr;
  ErrorD3D(d3dd->CreateTexture2D(&desc, nullptr, &vram_texture));

  D3D11_TEXTURE2D_DESC ram_desc;
  memcpy(&ram_desc, &desc, sizeof(ram_desc));
  ram_desc.Usage = D3D11_USAGE_STAGING;
  ram_desc.BindFlags = 0;
  ram_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
  ID3D11Texture2D *ram_texture = nullptr;
  ErrorD3D(d3dd->CreateTexture2D(&ram_desc, nullptr, &ram_texture));

  D3D11_MAPPED_SUBRESOURCE map;
  ErrorD3D(d3dc->Map(ram_texture, 0, D3D11_MAP_WRITE, 0, &map));
  for (unsigned int y = 0; y < desc.Height; ++y)
    for (unsigned int x = 0; x < desc.Width; ++x)
    {
      auto index = pFontPixels[y * desc.Width + x];
      auto src = pFontPalette[index];
      auto dst = (unsigned int *)((char *)map.pData + y * map.RowPitch) + x;

      if (index)
      {
        extern unsigned __int32 Color32_SwapRedBlue(unsigned __int16 color16);
        *dst = 0xFF000000 | Color32_SwapRedBlue(index == 1 ? uShadowColor : uFaceColor);
      }
      else
        *dst = 0x00000000;
    }

  d3dc->Unmap(ram_texture, 0);
  d3dc->CopyResource(vram_texture, ram_texture);
  ram_texture->Release();
    
  ID3D11ShaderResourceView *srv = nullptr;
  ErrorD3D(d3dd->CreateShaderResourceView(vram_texture, nullptr, &srv));
  vram_texture->Release();

  return srv;
}


ID3D11ShaderResourceView *RenderD3D11::PrepareFontTexture(unsigned char *pFontPixels, unsigned int uCharWidth, unsigned int uCharHeight, unsigned short *pFontPalette)
{
  D3D11_TEXTURE2D_DESC desc;
  desc.Width = uCharWidth;
  desc.Height = uCharHeight;
  desc.ArraySize = 1;
  desc.MipLevels = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
    
  ID3D11Texture2D *vram_texture = nullptr;
  ErrorD3D(d3dd->CreateTexture2D(&desc, nullptr, &vram_texture));

  D3D11_TEXTURE2D_DESC ram_desc;
  memcpy(&ram_desc, &desc, sizeof(ram_desc));
  ram_desc.Usage = D3D11_USAGE_STAGING;
  ram_desc.BindFlags = 0;
  ram_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
  ID3D11Texture2D *ram_texture = nullptr;
  ErrorD3D(d3dd->CreateTexture2D(&ram_desc, nullptr, &ram_texture));

  D3D11_MAPPED_SUBRESOURCE map;
  ErrorD3D(d3dc->Map(ram_texture, 0, D3D11_MAP_WRITE, 0, &map));
  for (unsigned int y = 0; y < desc.Height; ++y)
    for (unsigned int x = 0; x < desc.Width; ++x)
    {
      auto index = pFontPixels[y * desc.Width + x];
      auto src = pFontPalette[index];
      auto dst = (unsigned int *)((char *)map.pData + y * map.RowPitch) + x;

      if (index)
      {
        extern unsigned __int32 Color32_SwapRedBlue(unsigned __int16 color16);
        *dst = 0xFF000000 | Color32_SwapRedBlue(src);
      }
      else
        *dst = 0x00000000;
    }

  d3dc->Unmap(ram_texture, 0);
  d3dc->CopyResource(vram_texture, ram_texture);
  ram_texture->Release();
    
  ID3D11ShaderResourceView *srv = nullptr;
  ErrorD3D(d3dd->CreateShaderResourceView(vram_texture, nullptr, &srv));
  vram_texture->Release();

  return srv;
}


void d3d11_release(ID3D11ShaderResourceView *srv)
{
  srv->Release();
}


//----- (004A4DE1) --------------------------------------------------------
/*bool RenderD3D11::LoadTexture(const char *pName, unsigned int bMipMaps, void **pOutSurface, void **pOutTexture)
{
  unsigned __int16 *v13; // ecx@19
  unsigned __int16 *v14; // eax@19
  DWORD v15; // edx@20

  HWLTexture* pHWLTexture = pD3DBitmaps.LoadTexture(pName, bMipMaps);
  if (!pHWLTexture)
    return false;

  int num_min_levels = 1;
  {
    int dim = min(pHWLTexture->uWidth, pHWLTexture->uHeight);
    while (dim > 1)
    {
      dim /= 2;
      num_min_levels++;
    }
  }

  
    {
      D3D11_TEXTURE2D_DESC desc;
      desc.Width = pHWLTexture->uWidth;
      desc.Height = pHWLTexture->uHeight;
      desc.ArraySize = 1;
      desc.MipLevels = num_min_levels;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.SampleDesc.Count = 1;
      desc.SampleDesc.Quality = 0;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;// for mipmap generation;
      desc.CPUAccessFlags = 0;
      desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    
      ID3D11Texture2D *vram_texture = nullptr;
      ErrorD3D(d3dd->CreateTexture2D(&desc, nullptr, &vram_texture));

      D3D11_TEXTURE2D_DESC ram_desc;
      memcpy(&ram_desc, &desc, sizeof(ram_desc));
      ram_desc.Usage = D3D11_USAGE_STAGING;
      ram_desc.BindFlags = 0;
      ram_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      ram_desc.MiscFlags = 0;

      ID3D11Texture2D *ram_texture = nullptr;
      ErrorD3D(d3dd->CreateTexture2D(&ram_desc, nullptr, &ram_texture));

      D3D11_MAPPED_SUBRESOURCE map;
      ErrorD3D(d3dc->Map(ram_texture, 0, D3D11_MAP_WRITE, 0, &map));
      for (unsigned int y = 0; y < desc.Height; ++y)
        for (unsigned int x = 0; x < desc.Width; ++x)
        {
          auto src = pHWLTexture->pPixels + y * desc.Width + x;
          auto dst = (unsigned int *)((char *)map.pData + y * map.RowPitch) + x;

          extern unsigned __int32 Color32_SwapRedBlue(unsigned __int16 color16);
          *dst = 0xFF000000 | Color32_SwapRedBlue(*src);
        }

      d3dc->Unmap(ram_texture, 0);
      d3dc->CopyResource(vram_texture, ram_texture);
      ram_texture->Release();
    
      ID3D11ShaderResourceView *srv = nullptr;
      ErrorD3D(d3dd->CreateShaderResourceView(vram_texture, nullptr, &srv));
      vram_texture->Release();
      
      d3dc->GenerateMips(srv);
      *pOutTexture = (IDirect3DTexture2 *)srv;
      *pOutSurface = nullptr;
    }

    delete [] pHWLTexture->pPixels;
    delete pHWLTexture;
    return true;
}*/