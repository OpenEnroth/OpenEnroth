#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/ZlibWrapper.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Sprites.h"

#include "Platform/Api.h"






LODFile_IconsBitmaps *pEvents_LOD = nullptr;

LODFile_IconsBitmaps *pIcons_LOD = nullptr;
LODFile_IconsBitmaps *pIcons_LOD_mm6 = nullptr;
LODFile_IconsBitmaps *pIcons_LOD_mm8 = nullptr;

LODFile_IconsBitmaps *pBitmaps_LOD = nullptr;
LODFile_IconsBitmaps *pBitmaps_LOD_mm6 = nullptr;
LODFile_IconsBitmaps *pBitmaps_LOD_mm8 = nullptr;

LODFile_Sprites *pSprites_LOD = nullptr;
LODFile_Sprites *pSprites_LOD_mm6 = nullptr;
LODFile_Sprites *pSprites_LOD_mm8 = nullptr;

LODWriteableFile *pNew_LOD = nullptr;
LODWriteableFile *pGames_LOD = nullptr;




int _6A0CA4_lod_binary_search;
int _6A0CA8_lod_unused;


inline int LODFile_IconsBitmaps::LoadDummyTexture()
{
    for (unsigned int i = 0; i < uNumLoadedFiles; ++i)
        if (!strcmp(pTextures[i].pName, "pending"))
            return i;
    return LoadTextureFromLOD(&pTextures[uNumLoadedFiles], "pending", TEXTURE_16BIT_PALETTE);
}


// inlined
//----- (mm6c::00408860) --------------------------------------------------
void LODFile_IconsBitmaps::_inlined_sub2()
{
  ++uTexturePacksCount;
  if (!uNumPrevLoadedFiles)
    uNumPrevLoadedFiles = uNumLoadedFiles;
}

// inlined
//----- (mm6c::0045BE60) --------------------------------------------------
void LODFile_IconsBitmaps::_inlined_sub1()
{
  dword_11B84 = uNumLoadedFiles;
}

// inlined
//----- (mm6c::0045C310) --------------------------------------------------
void LODFile_Sprites::_inlined_sub1()
{
  field_ECA0 = uNumLoadedSprites;
}

// inlined
//----- (mm6c::0045C5B0) --------------------------------------------------
void LODFile_IconsBitmaps::_inlined_sub0()
{
  dword_11B80 = uNumLoadedFiles;
  if (dword_11B84 < uNumLoadedFiles)
    dword_11B84 = uNumLoadedFiles;
}


// inlined
//----- (mm6c::0045C660) --------------------------------------------------
void LODFile_Sprites::_inlined_sub0()
{
  field_ECA4 = uNumLoadedSprites;
  if (field_ECA0 < uNumLoadedSprites)
    field_ECA0 = uNumLoadedSprites;
}

//----- (004355F7) --------------------------------------------------------
void LODFile_IconsBitmaps::RemoveTexturesFromTextureList()
{
  if ( this->uTexturePacksCount )
  {
    if ( (this->uNumLoadedFiles - 1) >= this->uNumPrevLoadedFiles )
    {
      for ( uint i = this->uNumLoadedFiles - 1; i >= this->uNumPrevLoadedFiles; --i )
      {
        this->pTextures[i].Release();
      }
    }
    this->uNumLoadedFiles = this->uNumPrevLoadedFiles;
    this->uNumPrevLoadedFiles = 0;
    this->uTexturePacksCount = 0;
  }
}

//----- (004114F2) --------------------------------------------------------
void LODFile_IconsBitmaps::RemoveTexturesPackFromTextureList()
{
  if ( this->uTexturePacksCount )
  {
    this->uTexturePacksCount--;
    if ( !this->uTexturePacksCount )
    {
      if ( (this->uNumLoadedFiles - 1) >= this->uNumPrevLoadedFiles )
      {
        for ( uint i = this->uNumLoadedFiles - 1; i >= this->uNumPrevLoadedFiles; --i )
        {
          this->pTextures[i].Release();
        }
      }
      this->uNumLoadedFiles = this->uNumPrevLoadedFiles;
      this->uNumPrevLoadedFiles = 0;
    }
  }
}

//----- (004AC67E) --------------------------------------------------------
int LODFile_Sprites::LoadSpriteFromFile(LODSprite *pSpriteHeader, const char *pContainer)
{
  FILE *File; // [sp+4h] [bp-4h]@1
  void *DstBufa; // [sp+10h] [bp+8h]@4
  int Sizea; // [sp+14h] [bp+Ch]@3

  File = FindContainer(pContainer, 0);
  if ( File )
  {
    fread(pSpriteHeader, 1, 0x20u, File);
    strcpy(pSpriteHeader->pName, pContainer);
    Sizea = pSpriteHeader->uSpriteSize;
    pSpriteHeader->pSpriteLines = (LODSprite_stru0 *)malloc(8 * pSpriteHeader->uHeight);
    fread(pSpriteHeader->pSpriteLines, 1, 8 * pSpriteHeader->uHeight, File);

    if ( pSpriteHeader->uDecompressedSize )
    {
      pSpriteHeader->pDecompressedBytes = malloc(pSpriteHeader->uDecompressedSize);
      DstBufa = malloc(Sizea);
      fread(DstBufa, 1, Sizea, File);
      zlib::Uncompress(pSpriteHeader->pDecompressedBytes, (unsigned int *)&pSpriteHeader->uDecompressedSize, DstBufa, pSpriteHeader->uSpriteSize);
      pSpriteHeader->uSpriteSize = pSpriteHeader->uDecompressedSize;
      free(DstBufa);
    }
    else
    {
      pSpriteHeader->pDecompressedBytes = malloc(Sizea);
      fread(pSpriteHeader->pDecompressedBytes, 1, Sizea, File);
    }
	for ( uint i = 0; i < pSpriteHeader->uHeight; i++ )
      pSpriteHeader->pSpriteLines[i].pos += (unsigned int)pSpriteHeader->pDecompressedBytes;
    return 1;
  }
  else
    return -1;
}

//----- (004AC795) --------------------------------------------------------
bool LODFile_Sprites::LoadSprites(const char *pFilename)
{
    if (LoadHeader(pFilename, 1) == 0)
    {
        return LoadSubIndices("sprites08") == 0;
    }
    return false;
}


//----- (004AC7C0) --------------------------------------------------------
int LODFile_Sprites::LoadSprite(const char *pContainerName, unsigned int uPaletteID)
{
    FILE *sprite_file; // eax@12
    LODSprite temp_sprite_hdr; // [sp+Ch] [bp-3Ch]@12
    int i;//, sprite_indx;

    for (i = 0; i < uNumLoadedSprites; ++i)
    {
        if (!(_stricmp(pHardwareSprites[i].pName, pContainerName)))
            return i;
    }


    if (uNumLoadedSprites >= 1500)
        return -1;
    //if not loaded - load from file   

    if (!pHardwareSprites)
    {
        pHardwareSprites = (Sprite *)malloc(1500 * sizeof(Sprite));//0xEA60u
        for (i = 0; i < 1500; ++i)
        {
            pHardwareSprites[i].pName = nullptr;
            //pHardwareSprites[i].pTextureSurface = nullptr;
            //pHardwareSprites[i].pTexture = nullptr;
        }
    }
    temp_sprite_hdr.uHeight = 0;
    temp_sprite_hdr.uPaletteId = 0;
    temp_sprite_hdr.word_1A = 0;
    temp_sprite_hdr.pSpriteLines = nullptr;
    temp_sprite_hdr.pDecompressedBytes = nullptr;
    sprite_file = FindContainer(pContainerName, 0);
    if (!sprite_file)
        return -1;

    fread(&temp_sprite_hdr, 1, 0x20, sprite_file);
    pSpriteHeaders[uNumLoadedSprites].uWidth = temp_sprite_hdr.uWidth;
    pSpriteHeaders[uNumLoadedSprites].uHeight = temp_sprite_hdr.uHeight;
    LoadSpriteFromFile(&pSpriteHeaders[uNumLoadedSprites], pContainerName);        //this line is not present here in the original. necessary for Grayface's mouse picking fix

    pHardwareSprites[uNumLoadedSprites].uBufferWidth = temp_sprite_hdr.uWidth;
    pHardwareSprites[uNumLoadedSprites].uBufferHeight = temp_sprite_hdr.uHeight;
    pHardwareSprites[uNumLoadedSprites].pName = (const char *)malloc(20);
    strcpy((char *)pHardwareSprites[uNumLoadedSprites].pName, pContainerName);
    pHardwareSprites[uNumLoadedSprites].uPaletteID = uPaletteID;
    //render->MoveSpriteToDevice(&pHardwareSprites[uNumLoadedSprites]);
    pHardwareSprites[uNumLoadedSprites].texture = assets->GetSprite(pContainerName, uPaletteID, uNumLoadedSprites);
    pHardwareSprites[uNumLoadedSprites].sprite_header = &pSpriteHeaders[uNumLoadedSprites];

    HWLTexture *hwl = render->LoadHwlSprite(pContainerName);
    if (hwl)
    {
        pHardwareSprites[uNumLoadedSprites].uBufferWidth = hwl->uBufferWidth;
        pHardwareSprites[uNumLoadedSprites].uBufferHeight = hwl->uBufferHeight;
        pHardwareSprites[uNumLoadedSprites].uAreaX = hwl->uAreaX;
        pHardwareSprites[uNumLoadedSprites].uAreaY = hwl->uAreaY;
        pHardwareSprites[uNumLoadedSprites].uAreaWidth = hwl->uAreaWidth;
        pHardwareSprites[uNumLoadedSprites].uAreaHeight = hwl->uAreaHeigth;

        delete[] hwl->pPixels;
        delete hwl;
    }

    ++uNumLoadedSprites;
    return uNumLoadedSprites - 1;
}

//----- (004ACADA) --------------------------------------------------------
void LODFile_Sprites::ReleaseLostHardwareSprites()
{
}

//----- (004ACB70) --------------------------------------------------------
void LODFile_Sprites::ReleaseAll()
{
}

//----- (004ACBE0) --------------------------------------------------------
void LODFile_Sprites::MoveSpritesToVideoMemory()
{
}


//----- (004AD2D1) --------------------------------------------------------
int LODSprite::_4AD2D1_overlays(struct SoftwareBillboard *a2, int a3)
{
  int result; // eax@1
  unsigned int v4; // esi@1
  int v5; // edi@1
  LODSprite_stru0 *v6; // edx@2
  __int16 v7; // bx@2
  int v8; // ecx@3
  unsigned __int16 *v9; // esi@3
  int v10; // ebx@3
  void *v11; // edx@3
  unsigned __int16 *v12; // ecx@3
  int v13; // ebx@4
  //LODSprite *v14; // [sp+8h] [bp-10h]@1
  unsigned __int16 *v15; // [sp+10h] [bp-8h]@1
  unsigned __int16 *v16; // [sp+14h] [bp-4h]@1
  int i; // [sp+20h] [bp+8h]@1

  result = (int)a2;
  v4 = a2->uTargetPitch;

  __debugbreak(); // sub expects 16bit target surface, we may have 32bit
  v16 = (unsigned short *)a2->pTarget;
  v15 = a2->pPalette;
  v5 = this->uHeight - 1;
  for ( i = v4 * a2->screen_space_y - (this->uWidth >> 1) + a2->screen_space_x + 1; v5 >= 0; --v5 )
  {
    v6 = &this->pSpriteLines[v5];
    v7 = this->pSpriteLines[v5].a1;
    if ( this->pSpriteLines[v5].a1 != -1 )
    {
      v8 = v7;
      v9 = &v16[v7 + i];
      v10 = v6->a2;
      v11 = v6->pos;
      v12 = &v9[v10 - v8];
      while ( v9 <= v12 )
      {
        v13 = *(char *)v11;
        v11 = (char *)v11 + 1;
        if ( v13 )
          *v9 = v15[v13];
        ++v9;
      }
      v4 = *(int *)(result + 48);
      //this = v14;
    }
    i -= v4;
  }
  return result;
}

//----- (0046454B) --------------------------------------------------------
void LODFile_IconsBitmaps::ReleaseAll2()
{
  for ( uint i = (uint)this->dword_11B84; i < this->uNumLoadedFiles; i++ )
  {
    this->pTextures[i].Release();
  }
  this->uTexturePacksCount = 0;
  this->uNumPrevLoadedFiles = 0;
  this->uNumLoadedFiles = this->dword_11B84;
}

//----- (004645DC) --------------------------------------------------------
void LODFile_Sprites::DeleteSomeOtherSprites()
{
  int *v1; // esi@1
  int *v2; // edi@1

  v1 = (int *)&this->uNumLoadedSprites;
  v2 = &this->field_ECA0;
  DeleteSpritesRange(field_ECA0, uNumLoadedSprites);
  *v1 = *v2;
}

//----- (00461431) --------------------------------------------------------
void LOD::File::Close()
{
  if (isFileOpened )
  {
    this->pContainerName[0] = 0;
    this->uCurrentIndexDir = 0;
    free(pSubIndices);
    free(pRoot);
    pSubIndices = nullptr;
    pRoot = nullptr;
    fclose(pFile);
    isFileOpened = false;
    _6A0CA8_lod_unused = 0;
  }
}

//----- (00461492) --------------------------------------------------------
int LODWriteableFile::CreateNewLod(LOD::FileHeader *pHeader, LOD::Directory *pDir, const char *lod_name)
{
  if (isFileOpened)
    return 1;
  if ( !pDir->pFilename[0] )
    return 2;
  strcpy_s(pHeader->pSignature, "LOD");
  pHeader->LODSize = 100;
  pHeader->uNumIndices = 1;
  pDir->field_F = 0;
  pDir->uDataSize = 0;
  pDir->uOfsetFromSubindicesStart = 288;
  strcpy_s(pLODName, lod_name);

  pFile = fopen(pLODName, "wb+");
  if (!pFile)
    return 3;
  fwrite(pHeader,sizeof(LOD::FileHeader), 1, pFile);
  fwrite(pDir, sizeof(LOD::Directory), 1, pFile);
  fclose(pFile);
  pFile = nullptr;
  return 0;
}

//----- (0046153F) --------------------------------------------------------
void LOD::File::ResetSubIndices()
{
  if ( isFileOpened )
  {
    pContainerName[0] = 0;
    uCurrentIndexDir = 0;
    uOffsetToSubIndex = 0;
    uNumSubDirs = 0;
    uLODDataSize = 0;
    free(pSubIndices);
    pSubIndices = nullptr;
  }
}

//----- (00450C8B) --------------------------------------------------------
void LODFile_Sprites::DeleteSomeSprites()
{
  int *v1; // esi@1
  int *v2; // edi@1

  v1 = (int *)&this->uNumLoadedSprites;
  v2 = &this->field_ECA8;
  DeleteSpritesRange(this->field_ECA8, this->uNumLoadedSprites);
  *v1 = *v2;
}

//----- (00450CA9) --------------------------------------------------------
void LODFile_Sprites::DeleteSpritesRange(int uStartIndex, int uStopIndex)
{
  if ( this->pHardwareSprites )
  {
    if ( uStartIndex < uStopIndex )
    {
      for ( int i = uStartIndex; i < uStopIndex; i++ )
      {
        this->pSpriteHeaders[i].Release();
        pHardwareSprites[i].Release();
      }
    }
  }
  else
  {
    if ( uStartIndex < uStopIndex )
    {
      for ( int i = uStartIndex; i < uStopIndex; i++ )
        this->pSpriteHeaders[i].Release();
    }
  }
}

//----- (00450D1D) --------------------------------------------------------
void LODSprite::Release()
{
  if ( !(this->word_1A & 0x400) )
  {
    free(this->pDecompressedBytes);
    free(this->pSpriteLines);
  }

  this->word_1A = 0;
  this->pDecompressedBytes = nullptr;
  this->pSpriteLines = nullptr;
  this->pName[0] = 0;
  this->word_16 = 0;
  this->uPaletteId = 0;
  this->uTexturePitch = 0;
  this->uHeight = 0;
  this->uWidth = 0;
  this->uSpriteSize = 0;
}

//----- (00450D68) --------------------------------------------------------
void Sprite::Release()
{
}

//----- (0040FAEE) --------------------------------------------------------
//----- (0040FA2E) --------------------------------------------------------
bool LODFile_IconsBitmaps::Load(const char *pLODFilename, const char *pFolderName)
{
  ReleaseAll();

  if (LoadHeader(pLODFilename, 1))
    return false;

  return LoadSubIndices(pFolderName) == 0;
}

//----- (0040FA60) --------------------------------------------------------
void LODFile_IconsBitmaps::ReleaseAll()
{
  for( uint i = 0; i < this->uNumLoadedFiles; i++ )
  {
    this->pTextures[i].Release();
  }
  this->uTexturePacksCount = 0;
  this->uNumPrevLoadedFiles = 0;
  this->dword_11B84 = 0;
  this->dword_11B80 = 0;
  this->uNumLoadedFiles = 0;
}

//----- (0040F9F0) --------------------------------------------------------
unsigned int LODFile_IconsBitmaps::FindTextureByName(const char *pName)
{
  for ( uint i = 0; i < this->uNumLoadedFiles; i++ )
  {
    if ( !_stricmp(this->pTextures[i].pName, pName) )
      return i;
  }
  return -1;
}

//----- (0040F9C5) --------------------------------------------------------
void LODFile_IconsBitmaps::SyncLoadedFilesCount()
    {
  signed int loaded_files; // eax@1
  Texture_MM7 *pTex; // edx@1

  loaded_files = this->uNumLoadedFiles;
  for ( pTex = &this->pTextures[loaded_files]; !pTex->pName[0]; --pTex )
    --loaded_files;
  if ( loaded_files < (signed int)this->uNumLoadedFiles )
  {
    ++loaded_files;
    this->uNumLoadedFiles = loaded_files;
  }
 
}

//----- (0046249B) --------------------------------------------------------
LODFile_Sprites::~LODFile_Sprites()
{
    if (this->pHardwareSprites)
    {
        for (int i = 0; i < this->uNumLoadedSprites; ++i)
        {
            this->pSpriteHeaders[i].Release();
            this->pHardwareSprites[i].Release();
        }
    }
    else
    {
        for (int i = 0; i < this->uNumLoadedSprites; ++i)
            this->pSpriteHeaders[i].Release();
    }
}

//----- (00462463) --------------------------------------------------------
LODSprite::~LODSprite()
{
    if (!(this->word_1A & 0x400))
    {
        free(pDecompressedBytes);
        free(pSpriteLines);
    }
    pDecompressedBytes = nullptr;
    pSpriteLines = nullptr;
}

//----- (004623E5) --------------------------------------------------------
LODFile_Sprites::LODFile_Sprites():
  LOD::File()
{
  /*_eh_vector_constructor_iterator_(
    v1->pSpriteHeaders,
    40,
    1500,
    (void ( *)(void *))LODSprite::LODSprite,
    (void ( *)(void *))LODSprite::dtor);*/
  field_ECA4 = 0;
  field_ECA0 = 0;
  pHardwareSprites = 0;
  //can_load_hardware_sprites = 0;
  field_ECB4 = 0;
  uNumLoadedSprites = 0;
}

//----- (00462303) --------------------------------------------------------
LODFile_IconsBitmaps::~LODFile_IconsBitmaps()
{

  for ( uint i = 0; i < this->uNumLoadedFiles; i++ )
  {
    this->pTextures[i].Release();
  }
  free(this->pHardwareSurfaces);
  free(this->pHardwareTextures);
  free(this->ptr_011BB4);
  //LOD::File::vdtor((LOD::File *)v1);
}

//----- (00462272) --------------------------------------------------------
LODFile_IconsBitmaps::LODFile_IconsBitmaps():
  LOD::File()
{
  /*v2 = v1->pTextures;
  v3 = 1000;
  do
  {
    Texture_MM7::Texture_MM7(v2);
    ++v2;
    --v3;
  }
  while ( v3 );*/
  this->uTexturePacksCount = 0;
  this->uNumPrevLoadedFiles = 0;
  this->dword_11B84 = 0;
  this->dword_11B80 = 0;
  this->uNumLoadedFiles = 0;
  this->_011BA4_debug_paletted_pixels_uncompressed = false;
  //this->can_load_hardware_sprites = 0;
  this->pHardwareSurfaces = 0;
  this->pHardwareTextures = 0;
  this->ptr_011BB4 = 0;
  this->uTextureRedBits = 0;
  this->uTextureGreenBits = 0;
  this->uTextureBlueBits = 0;
}

//----- (004621A7) --------------------------------------------------------
bool LODWriteableFile::_4621A7()//закрыть и загрузить записываемый ф-л(при сохранении)
{
  CloseWriteFile();
  return LoadFile(pLODName, 0);
}

//----- (00461FD4) ---LODFile_sub_461FD4---text:004632EA  --------------------------------------------------
int LODWriteableFile::FixDirectoryOffsets()
{
  int total_size; // edi@1
  int temp_offset; // ecx@5
  FILE *tmp_file; // eax@9
  size_t write_size; // edi@12
  int result;
  char Filename[256]; // [sp+Ch] [bp-228h]@9
  char NewFilename[256]; // [sp+10Ch] [bp-128h]@15
  int i;

  total_size = 0;
  for ( i = 0; i < uNumSubDirs; i++ )
    total_size += pSubIndices[i].uDataSize;
  //fix offsets
  temp_offset = sizeof(LOD::Directory) * uNumSubDirs;
  for ( i = 0; i < uNumSubDirs; i++ )
  {
    pSubIndices[i].uOfsetFromSubindicesStart=temp_offset;
    temp_offset+=pSubIndices[i].uDataSize;
  }
  strcpy_s(Filename, "lod.tmp");
  tmp_file = fopen(Filename, "wb+");

  if ( tmp_file )
  {
    fwrite((const void *)&header, sizeof(LOD::FileHeader), 1, tmp_file);

    LOD::Directory Lindx;
    strcpy_s(Lindx.pFilename, "chapter");
    Lindx.uOfsetFromSubindicesStart = uOffsetToSubIndex; //10h 16
    Lindx.uDataSize = sizeof(LOD::Directory) * uNumSubDirs + total_size;		   //14h 20
    Lindx.dword_000018 = 0;		   //18h 24 
    Lindx.uNumSubIndices = uNumSubDirs;		   //1ch 28
    Lindx.word_00001E = 0;		   // 1Eh 30
    fwrite(&Lindx, sizeof(LOD::Directory), 1, tmp_file);
    fwrite(pSubIndices, sizeof(LOD::Directory), uNumSubDirs, tmp_file);
    fseek(pOutputFileHandle, 0, 0);
    if ( total_size > 0 )
    {
      do
      {
        write_size = uIOBufferSize;
        if ( total_size <= (signed int)uIOBufferSize )
          write_size =total_size;
        fread(pIOBuffer, 1,  write_size,  pOutputFileHandle);
        fwrite(pIOBuffer, 1,  write_size, tmp_file);
        total_size -=  write_size;
      }
      while ( total_size > 0 );
    }
    strcpy(NewFilename, (const char *)&pLODName);
    fclose(tmp_file);
    fclose(pOutputFileHandle);
    CloseWriteFile();
    remove("lodapp.tmp");
    remove(NewFilename);
    rename(Filename, NewFilename);
    CloseWriteFile();
    LoadFile( (const char *)&pLODName, 0);
    result = 0;
  }
  else
    result = 5;
  return result;
}

//----- (00461F71) --------------------------------------------------------
bool LOD::File::AppendDirectory(LOD::Directory *pDir, const void *pData)
{
  Assert(uNumSubDirs < 299);

  memcpy(&pSubIndices[uNumSubDirs++], pDir, sizeof(LOD::Directory));
  fwrite(pData, 1, pDir->uDataSize, pOutputFileHandle);
  return true;
}

//----- (00461F1E) --------------------------------------------------------
int LODWriteableFile::CreateTempFile()
{
  if (!isFileOpened)
    return 1;

  if (pIOBuffer && uIOBufferSize )
  {
    uCurrentIndexDir = 0;
    uNumSubDirs = 0;
    pOutputFileHandle = fopen("lodapp.tmp", "wb+");
    return pOutputFileHandle ? 1 : 7;  
  }
  else
    return 5;
}

//----- (00461EE9) --------------------------------------------------------
void LODWriteableFile::CloseWriteFile()
{
  if (isFileOpened)
  {
    pContainerName[0] = 0;
    uCurrentIndexDir = 0;
    _6A0CA8_lod_unused = 0;
    
    isFileOpened = false;
    fflush(pFile);
    fclose(pFile);
    pFile = nullptr;
  }
  //else 
    //__debugbreak();
}
// 6A0CA8: using guessed type int 6A0CA8_lod_unused;


//----- (00461B48) --------------------------------------------------------
unsigned int LODWriteableFile::Write(const LOD::Directory *pDir, const void *pDirData, int a4)
{
  char Filename[256]; 
  char NewFilename[256];
  FILE *tmp_file; 
  int comp_res;
  bool bRewrite_data;
  int offset_to_data;
  int total_data_size;
  int size_correction;
  int to_copy_size;
  int read_size;
  int curr_position;
  int insert_index;

  //insert new data in sorted index lod file
  bRewrite_data = false;
  insert_index = -1;
  if ( !isFileOpened )//sometimes gives crash
    return 1;
  if ( !pSubIndices )
    return 2;
  if ( !pIOBuffer || !uIOBufferSize )
    return 3;

  for ( int i = 0; i < uNumSubDirs; i++ )
  {
    comp_res = _stricmp(pSubIndices[i].pFilename, pDir->pFilename);
    if( comp_res == 0 )
    {
      insert_index = i;
      if ( a4 == 0 )
      {
        bRewrite_data = true;
        break;
      }
      if ( a4 == 1 )
      {
        if ( pSubIndices[i].uNumSubIndices < pDir->uNumSubIndices )
        {
          if ( pSubIndices[i].word_00001E < pDir->word_00001E )
            return 4;
        }
        else
          bRewrite_data = true;
        break;
      }
      if ( a4 == 2 )
        return 4;
    }
    else if ( comp_res > 0 )
    {
      if ( insert_index == -1 )
      {
        insert_index=i;
        break;
      }
    }
  }
  strcpy(Filename, "lod.tmp");
  tmp_file = fopen(Filename, "wb+");
  if ( !tmp_file )
    return 5;
  if (!bRewrite_data)
    size_correction = 0;
  else
    size_correction = pSubIndices[insert_index].uDataSize;

  //create chapter index
  LOD::Directory Lindx;
  strcpy(Lindx.pFilename, "chapter");
  Lindx.dword_000018 = 0;
  Lindx.word_00001E = 0;
  Lindx.uNumSubIndices = uNumSubDirs;
  Lindx.uOfsetFromSubindicesStart = sizeof(LOD::FileHeader) + sizeof(LOD::Directory);
  total_data_size = uLODDataSize + pDir->uDataSize-size_correction;
  if (!bRewrite_data)
  {
    total_data_size += sizeof(LOD::Directory);
    Lindx.uNumSubIndices++;
  }

  Lindx.uDataSize = total_data_size;
  uNumSubDirs = Lindx.uNumSubIndices;
  //move indexes +1 after insert point
  if ( !bRewrite_data && (insert_index < uNumSubDirs) )//перезаписывание файлов для освобождения места для нового ф-ла
  {
    for( int i = uNumSubDirs; i > insert_index; --i )
      memcpy(&pSubIndices[i], &pSubIndices[i - 1], sizeof(LOD::Directory));	//Uninitialized memory access
  }
  //insert
  memcpy(&pSubIndices[insert_index], pDir, sizeof(LOD::Directory));//записать текущий файл
  //correct offsets to data
  if (uNumSubDirs > 0)
  {
    offset_to_data = sizeof(LOD::Directory) * uNumSubDirs;
    for ( int i = 0; i < uNumSubDirs; i++ )
    {
      pSubIndices[i].uOfsetFromSubindicesStart = offset_to_data;
      offset_to_data += pSubIndices[i].uDataSize;
    }
  }

  //construct  lod file	with added data
  fwrite(&header, sizeof(LOD::FileHeader), 1, tmp_file);
  fwrite(&Lindx, sizeof(LOD::Directory), 1, tmp_file);
  fseek(pFile,Lindx.uOfsetFromSubindicesStart, SEEK_SET);
  fwrite(pSubIndices, sizeof(LOD::Directory), uNumSubDirs, tmp_file);

  offset_to_data = sizeof(LOD::Directory) * uNumSubDirs;
  if ( !bRewrite_data )
    offset_to_data -= sizeof(LOD::Directory);

  fseek(pFile, offset_to_data, SEEK_CUR);
  //copy from open lod to temp 	lod	  first half
  to_copy_size = pSubIndices[insert_index].uOfsetFromSubindicesStart - pSubIndices[0].uOfsetFromSubindicesStart;
  while(to_copy_size > 0)
  {
    read_size = uIOBufferSize;
    if ( to_copy_size <= uIOBufferSize )
      read_size = to_copy_size;
    fread(pIOBuffer, 1, read_size, pFile);
    fwrite(pIOBuffer, 1, read_size, tmp_file);
    to_copy_size -= read_size;
  }
  // add container data
  fwrite(pDirData, 1, pDir->uDataSize, tmp_file);// Uninitialized memory access(tmp_file)
  if ( bRewrite_data )
    fseek(pFile,size_correction , SEEK_CUR);

  //add remainng data  last half
  curr_position = ftell(pFile);
  fseek(pFile, 0, SEEK_END);
  to_copy_size = ftell(pFile) - curr_position;
  fseek(pFile, curr_position, SEEK_SET);
  while ( to_copy_size > 0 )
  {
    read_size = uIOBufferSize;
    if ( to_copy_size <= uIOBufferSize )
      read_size = to_copy_size;
    fread(pIOBuffer, 1, read_size, pFile);
    fwrite(pIOBuffer, 1, read_size, tmp_file);
    to_copy_size -= read_size;
  }
  //replace	  old file by new with added  data
  strcpy(NewFilename, (const char *)&pLODName);
  fclose(tmp_file);
  fclose(pFile);
  CloseWriteFile();
  remove(NewFilename);
  rename(Filename, NewFilename);
  CloseWriteFile();
  //reload new
  LoadFile((const char *)&pLODName, 0);//isFileOpened == true, next file
  return 0;
}

//----- (00461A43) --------------------------------------------------------
bool LODWriteableFile::LoadFile(const char *pFilename, bool bWriting)
{
  if (bWriting & 1)
    pFile = fopen(pFilename, "rb");
  else
    pFile = fopen(pFilename, "rb+");
  if (!pFile)
  {
	 // __debugbreak();
    return false;// возможно файл не закрыт, поэтому не открывается
  }

  strcpy_s(pLODName, pFilename);
  fread(&header, sizeof(LOD::FileHeader), 1, pFile);
  
  LOD::Directory lod_indx;
  fread( &lod_indx,sizeof(LOD::Directory), 1, pFile);

  fseek(pFile, 0, SEEK_SET);
  isFileOpened = true;
  strcpy_s(pContainerName, "chapter");
  uCurrentIndexDir = 0;
  uLODDataSize = lod_indx.uDataSize;
  uNumSubDirs = lod_indx.uNumSubIndices;
  Assert(uNumSubDirs <= 300);

  uOffsetToSubIndex = lod_indx.uOfsetFromSubindicesStart;
  fseek(pFile, uOffsetToSubIndex, SEEK_SET);

  fread(pSubIndices, sizeof(LOD::Directory), uNumSubDirs, pFile);
  return true;
}

//----- (00461A11) --------------------------------------------------------
void LOD::File::FreeSubIndexAndIO()
{
  free(pSubIndices);
  free(pIOBuffer);// delete [] pIOBuffer;
  pIOBuffer = nullptr;
  pSubIndices = nullptr;
}

//----- (00461954) --------------------------------------------------------
void LOD::File::AllocSubIndicesAndIO(unsigned int uNumSubIndices, unsigned int uBufferSize)
{
    if (pSubIndices)
    {
        logger->Warning(L"Attempt to reset a LOD subindex!");
        free(pSubIndices);
        pSubIndices = nullptr;
    }
    pSubIndices = (LOD::Directory *)malloc(32 * uNumSubIndices);
    if (pIOBuffer)
    {
        logger->Warning(L"Attempt to reset a LOD IObuffer!");
        free(pIOBuffer);
        pIOBuffer = nullptr;
        uIOBufferSize = 0;
    }
    if (uBufferSize)
    {
        pIOBuffer = (unsigned __int8 *)malloc(uBufferSize);
        uIOBufferSize = uBufferSize;
    }
}

//----- (0046188A) --------------------------------------------------------
int LOD::File::LoadSubIndices(const char *pContainer)
{
  unsigned int uDir; // edi@1
  LOD::Directory *curr_index; // eax@7

  ResetSubIndices();
  uDir = 0;

  for (uDir=0; uDir <header.uNumIndices;++uDir)
  {
    if (!_stricmp(pContainer, pRoot[uDir].pFilename))
    {
        strcpy_s(pContainerName, pContainer);
      uCurrentIndexDir = uDir;
      curr_index = (LOD::Directory *)&pRoot[uDir];
      uOffsetToSubIndex = curr_index->uOfsetFromSubindicesStart ;
      uNumSubDirs = curr_index->uNumSubIndices;// *(_WORD *)(v8 + 28);
      fseek( pFile, uOffsetToSubIndex, SEEK_SET);
      pSubIndices = (LOD::Directory *)malloc(sizeof(LOD::Directory)*(uNumSubDirs + 5));

      if ( pSubIndices)
        fread( pSubIndices, sizeof(LOD::Directory),  uNumSubDirs,  pFile);
      return 0;
    }
  }
  return 3;
}

//----- (004617D5) --------------------------------------------------------
bool LOD::File::LoadHeader(const char *pFilename, bool bWriting)
{
  const char *v6; // [sp-4h] [bp-Ch]@4

  if ( this->isFileOpened )
    Close();
  if ( bWriting & 1 )
    v6 = "rb";
  else
    v6 = "rb+";

  pFile = fopen(pFilename, v6);
  if ( pFile )
  {
      strcpy_s(pLODName, pFilename);
    fread(&header, sizeof(LOD::FileHeader), 1, pFile);
    pRoot = (LOD::Directory *)malloc(160);
    if ( pRoot )
    {
      fread(pRoot, sizeof(LOD::Directory), header.uNumIndices, pFile);
      fseek(pFile, 0, SEEK_SET);
      isFileOpened = true;
      return false;
    }
    else
    {
      fclose(pFile);
      return true;
    }
  }
  return true;
}

//----- (00461790) --------------------------------------------------------
LOD::File::~File()
{
  if ( this->isFileOpened )
  {
    fclose(this->pFile);
    free(this->pSubIndices);
  }
}

//----- (0046175B) --------------------------------------------------------
LOD::File::File():
  pRoot(nullptr),
  isFileOpened(false)
{
  memset(pLODName, 0, 256);
  memset(pContainerName, 0, 16);
  this->pFile = nullptr;
  this->pSubIndices = nullptr;
  this->pIOBuffer = nullptr;
  this->isFileOpened = false;
  this->uIOBufferSize = 0;
  Close();
}

//----- (0046172B) --------------------------------------------------------
LOD::Directory::Directory()
{
  memset(pFilename, 0, 16);
  this->pFilename[0] = 0;
  this->uOfsetFromSubindicesStart = 0;
  this->uDataSize = 0;
  this->uNumSubIndices = 0;
  this->dword_000018 = 0;
  this->word_00001E = 0;
}

//----- (0046165E) --------------------------------------------------------
int LOD::File::CalcIndexFast(int startIndex, int maxIndex, const char *pContainerName)
{
  int pCurrent_position; // esi@1
  int v5; // ebx@2
  int result; // eax@2

  pCurrent_position = startIndex;
  while ( 1 )                                   // binary search in LOD indices
  {

    while ( 1 )
    {
      v5 = maxIndex - pCurrent_position;
      result = _stricmp(pContainerName, (const char *)(&this->pSubIndices[(maxIndex - pCurrent_position) / 2] + pCurrent_position));
      if ( !_stricmp(pContainerName, (const char *)(&this->pSubIndices[(maxIndex - pCurrent_position) / 2] + pCurrent_position)) )
        _6A0CA4_lod_binary_search = (maxIndex - pCurrent_position) / 2 + pCurrent_position;
      if ( pCurrent_position == maxIndex )
      {
        _6A0CA4_lod_binary_search = -1;
        return result;
      }
      if ( result < 0 )//первая строка меньше второй
        break;
      if ( v5 <= 4 )
      {
        for ( int i = pCurrent_position; i < maxIndex; ++i )
        {
          result = _stricmp(pContainerName, this->pSubIndices[i].pFilename);
          if ( !_stricmp(pContainerName, this->pSubIndices[i].pFilename) )
          {
            _6A0CA4_lod_binary_search = i;
            return 0;//строки ровны
          }
        }
        _6A0CA4_lod_binary_search = -1;
        return result;
      }
      pCurrent_position += (maxIndex - pCurrent_position) / 2;
    }

    if ( v5 <= 4 )
      break;
    maxIndex = (maxIndex - pCurrent_position) / 2 + pCurrent_position;
  }

  for (int i = pCurrent_position; i < maxIndex; ++i)
  {
    result = _stricmp(pContainerName, this->pSubIndices[i].pFilename);
    if ( !_stricmp(pContainerName, this->pSubIndices[i].pFilename) )
    {
      _6A0CA4_lod_binary_search = i;
      return 0;
    }
  }
  _6A0CA4_lod_binary_search = -1;
  return result;
}

bool LOD::File::DoesContainerExist(const String &filename)
{
    return this->DoesContainerExist(filename.c_str());
}

//----- (0046161C) --------------------------------------------------------
bool LOD::File::DoesContainerExist(const char *pContainer)
{
  for ( int i = 0; i < (signed int)this->uNumSubDirs; ++i )
  {
    if ( !_stricmp(pContainer, this->pSubIndices[i].pFilename) )
      return 1;
  }
  return 0;
}

//----- (00461397) --------------------------------------------------------
int LODFile_Sprites::_461397()
{
  this->field_ECA8 = this->uNumLoadedSprites;
  if ( this->uNumLoadedSprites < this->field_ECA0 )
    this->field_ECA8 = this->field_ECA0;
  if ( this->field_ECA0 < this->field_ECA4 )
    field_ECA0 = this->field_ECA4;
  return this->uNumLoadedSprites;
}


FILE *LOD::File::FindContainer(const String &filename, bool linear_search)
{
    return this->FindContainer(filename.c_str(), linear_search);
}

//----- (00461580) --------------------------------------------------------
FILE *LOD::File::FindContainer(const char *pContainer_Name, bool bLinearSearch)
{
  if (!isFileOpened)
    return 0;

  if (bLinearSearch)
  {
    for (uint i = 0; i < uNumSubDirs; ++i)
      if (!_stricmp(pContainer_Name, pSubIndices[i].pFilename))
      {
        fseek(pFile, uOffsetToSubIndex + pSubIndices[i].uOfsetFromSubindicesStart, SEEK_SET);
        return pFile;
      }
    return nullptr;
  }
  else
  {
    CalcIndexFast(0, uNumSubDirs, pContainer_Name);
    if ( _6A0CA4_lod_binary_search < 0 )
      return 0;
    fseek(pFile, uOffsetToSubIndex + pSubIndices[_6A0CA4_lod_binary_search].uOfsetFromSubindicesStart, SEEK_SET);
    return pFile;
  }
}

//----- (0041097D) --------------------------------------------------------
void LODFile_IconsBitmaps::SetupPalettes(unsigned int uTargetRBits, unsigned int uTargetGBits, unsigned int uTargetBBits)
{
  FILE *File; // [sp+50h] [bp-4h]@7

  if ( this->uTextureRedBits != uTargetRBits
    || this->uTextureGreenBits != uTargetGBits
    || this->uTextureBlueBits != uTargetBBits )	 //Uninitialized memory access
  {
    this->uTextureRedBits = uTargetRBits;
    this->uTextureGreenBits = uTargetGBits;
    this->uTextureBlueBits = uTargetBBits;
    for ( uint i = 0; i < this->uNumLoadedFiles; ++i )
    {
      Texture_MM7 DstBuf; // [sp+4h] [bp-50h]@6
      //Texture_MM7::Texture_MM7(&DstBuf);
      if ( this->pTextures[i].pPalette16 )
      {
        File = FindContainer((const char *)this->pTextures[i].pName, 0);
        if ( File )
        {
          fread(&DstBuf, 1, 0x30u, File);
          fseek(File, DstBuf.uTextureSize, 1);
          for ( uint j = 0; j < 256; ++j )
          {
            fread((char *)&uTargetRBits + 3, 1, 1, File);
            fread((char *)&uTargetGBits + 3, 1, 1, File);
            fread((char *)&uTargetBBits + 3, 1, 1, File);
            this->pTextures[i].pPalette16[j] = (BYTE3(uTargetRBits) >> (8 - (unsigned char)(this->uTextureRedBits)))
                                      << ((unsigned char)(this->uTextureGreenBits) + (unsigned char)(this->uTextureBlueBits));
            this->pTextures[i].pPalette16[j] |= (BYTE3(uTargetGBits) >> (8 - (unsigned char)(this->uTextureGreenBits)))
                                      << this->uTextureBlueBits;
            this->pTextures[i].pPalette16[j] |= BYTE3(uTargetBBits) >> (8 - (unsigned char)(this->uTextureBlueBits));
          }
        }
      }
    }
  }
}

//----- (0041088B) --------------------------------------------------------
void *LOD::File::LoadRaw(const char *pContainer, int a3)
{
  FILE *File; // eax@1
  void *v7; // ebx@7
  void *v8; // edi@7
  Texture_MM7 DstBuf; // [sp+Ch] [bp-4Ch]@1

  File = FindContainer(pContainer, 0);
  if ( !File )
    Error("Unable to load %s", pContainer);

  fread(&DstBuf, 1, 0x30u, File);
  if ( DstBuf.uDecompressedSize )
  {
    if ( a3 )
      v7 = malloc(DstBuf.uDecompressedSize+1);
    else
      v7 = malloc(DstBuf.uDecompressedSize+1);
    v8 = malloc(DstBuf.uTextureSize+1);
    fread(v8, 1, DstBuf.uTextureSize, File);
    zlib::Uncompress(v7, &DstBuf.uDecompressedSize, v8, DstBuf.uTextureSize);
    DstBuf.uTextureSize = DstBuf.uDecompressedSize;
    free(v8);
  }
  else
  {
    if ( a3 )
      v7 = malloc(DstBuf.uTextureSize+1);
    else
      v7 = malloc(DstBuf.uTextureSize+1);
    fread(v7, 1, DstBuf.uTextureSize, File);
  }
  return v7;
}

//----- (00410522) --------------------------------------------------------
int LODFile_IconsBitmaps::PlacementLoadTexture(Texture_MM7 *pDst, const char *pContainer, unsigned int uTextureType)
{
  void *v9; // ST2C_4@6
  int v15; // ecx@12
  int v16; // ecx@12
  int v17; // eax@12
  FILE *File; // [sp+68h] [bp-4h]@1
  unsigned int uTargetRBits;
  unsigned int uTargetGBits;
  unsigned int uTargetBBits;

  File = FindContainer(pContainer, 0);
  if ( !File )
  {
    File = FindContainer("pending", 0);
    if ( !File )
      Error("Can't find %s!", pContainer);
  }

  fread(pDst, 1, 0x30u, File);
  strcpy_s(pDst->pName, pContainer);
  pDst->paletted_pixels = 0;
  if ( pDst->uDecompressedSize )
  {
    pDst->paletted_pixels = (unsigned __int8 *)malloc(pDst->uDecompressedSize);
    v9 = malloc(pDst->uTextureSize);
    fread((void *)v9, 1, (size_t)pDst->uTextureSize, File);
    zlib::Uncompress(pDst->paletted_pixels, &pDst->uDecompressedSize, v9, pDst->uTextureSize);
    pDst->uTextureSize = pDst->uDecompressedSize;
    free(v9);
  }
  else
  {
    pDst->paletted_pixels = (unsigned __int8 *)malloc(0);
    fread(pDst->paletted_pixels, 1, (size_t)pDst->uTextureSize, File);
  }
  pDst->pPalette16 = 0;
  pDst->pPalette24 = 0;
  if ( uTextureType == 1 )
  {
    pDst->pPalette24 = (unsigned __int8 *)malloc(0x300u);
    fread(pDst->pPalette24, 1, 0x300u, File);
  }
  else if ( uTextureType == 2 )
  {
    pDst->pPalette16 = (unsigned __int16 *)malloc(0x400u);
    for ( uint i = 0; i < 256; ++i )
    {
      fread((char *)&uTargetRBits + 3, 1, 1, File);
      fread((char *)&uTargetGBits + 3, 1, 1, File);
      fread((char *)&uTargetBBits + 3, 1, 1, File);
      pDst->pPalette16[i] = (unsigned __int8)(BYTE3(uTargetRBits) >> (8 - (unsigned char)(this->uTextureRedBits)))
                          << ((unsigned char)(this->uTextureBlueBits) + (unsigned char)(this->uTextureGreenBits));
      pDst->pPalette16[i] += (unsigned __int8)(BYTE3(uTargetGBits) >> (8 - (unsigned char)(this->uTextureGreenBits)))
                          << this->uTextureBlueBits;
      pDst->pPalette16[i] += (unsigned __int8)(BYTE3(uTargetBBits) >> (8 - (unsigned char)(this->uTextureBlueBits)));
    }
  }

  if ( pDst->pBits & 2 )
  {
    v15 = (int)&pDst->paletted_pixels[pDst->uSizeOfMaxLevelOfDetail];
    pDst->pLevelOfDetail1 = (unsigned __int8 *)v15;
    v16 = (pDst->uSizeOfMaxLevelOfDetail >> 2) + v15;
    //pDst->pLevelOfDetail2 = (unsigned __int8 *)v16;
    v17 = v16 + (pDst->uSizeOfMaxLevelOfDetail >> 4);
  }
  else
  {
    v17 = 0;
    //pDst->pLevelOfDetail2 = 0;
    //pDst->pLevelOfDetail1 = 0;
  }

  pDst->uWidthLn2 = ImageHelper::GetPowerOf2(pDst->uTextureWidth);
  if (pDst->uWidthLn2 >= 15)
  {
      pDst->uWidthLn2 = 1;
  }

  pDst->uHeightLn2 = ImageHelper::GetPowerOf2(pDst->uTextureHeight);
  if (pDst->uHeightLn2 >= 15)
  {
      pDst->uHeightLn2 = 1;
  }

  switch ( pDst->uWidthLn2 )
  {
    case 2:
      pDst->uWidthMinus1 = 3;
      break;
    case 3:
      pDst->uWidthMinus1 = 7;
      break;
    case 4:
      pDst->uWidthMinus1 = 15;
      break;
    case 5:
      pDst->uWidthMinus1 = 31;
      break;
    case 6:
      pDst->uWidthMinus1 = 63;
      break;
    case 7:
      pDst->uWidthMinus1 = 127;
      break;
    case 8:
      pDst->uWidthMinus1 = 255;
      break;
    case 9:
      pDst->uWidthMinus1 = 511;
      break;
    case 10:
      pDst->uWidthMinus1 = 1023;
      break;
    case 11:
      pDst->uWidthMinus1 = 2047;
      break;
    case 12:
      pDst->uWidthMinus1 = 4095;
      break;
    default:
      break;
  }
  switch ( pDst->uHeightLn2 )
  {
    case 2:
      pDst->uHeightMinus1 = 3;
      break;
    case 3:
      pDst->uHeightMinus1 = 7;
      break;
    case 4:
      pDst->uHeightMinus1 = 15;
      break;
    case 5:
      pDst->uHeightMinus1 = 31;
      break;
    case 6:
      pDst->uHeightMinus1 = 63;
      break;
    case 7:
      pDst->uHeightMinus1 = 127;
      break;
    case 8:
      pDst->uHeightMinus1 = 255;
      break;
    case 9:
      pDst->uHeightMinus1 = 511;
      break;
    case 10:
      pDst->uHeightMinus1 = 1023;
      break;
    case 11:
      pDst->uHeightMinus1 = 2047;
      break;
    case 12:
      pDst->uHeightMinus1 = 4095;
      break;
    default:
      return 1;
  }
  return 1;
}

//----- (00410423) --------------------------------------------------------
/*void LODFile_IconsBitmaps::_410423_move_textures_to_device()
{
    size_t v4; // eax@9
    char *v5; // ST1C_4@9

    for (uint i = 0; i < this->uNumLoadedFiles; i++)
    {
        if (this->ptr_011BB4[i])
        {
            if (this->pTextures[i].pName[0] != 'w' || this->pTextures[i].pName[1] != 't'
                || this->pTextures[i].pName[2] != 'r' || this->pTextures[i].pName[3] != 'd' || this->pTextures[i].pName[4] != 'r')
                render->LoadTexture(&this->pTextures[i].pName[0], this->pTextures[i].uTextureSize, (void **)&this->pHardwareSurfaces[i],
                    (void **)&this->pHardwareTextures[i]);
            else
            {
                v4 = strlen(&this->pTextures[i].pName[0]);
                v5 = (char *)malloc(v4 + 2);
                *v5 = 'h';
                strcpy(v5 + 1, &this->pTextures[i].pName[0]);
                render->LoadTexture(v5, this->pTextures[i].uTextureSize, (void **)&this->pHardwareSurfaces[i], (void **)&this->pHardwareTextures[i]);
                free(v5);
            }
        }
    }
    if (this->ptr_011BB4)
    {
        if (this->uNumLoadedFiles > 1)
            memset(this->ptr_011BB4, 0, this->uNumLoadedFiles - 1);
    }
}*/

//----- (004103BB) --------------------------------------------------------
void LODFile_IconsBitmaps::ReleaseHardwareTextures()
{
}

//----- (0041033D) --------------------------------------------------------
void LODFile_IconsBitmaps::ReleaseLostHardwareTextures()
{
}

//----- (004101B1) --------------------------------------------------------
int LODFile_IconsBitmaps::ReloadTexture(Texture_MM7 *pDst, const char *pContainer, int mode)
{
  Texture_MM7 *v6; // esi@2
  unsigned int v7; // ebx@6
  unsigned int v8; // ecx@6
  signed int result; // eax@7
  FILE *File; // [sp+Ch] [bp-8h]@1
  unsigned __int8 v15; // [sp+11h] [bp-3h]@13
  unsigned __int8 v16; // [sp+12h] [bp-2h]@13
  unsigned __int8 DstBuf; // [sp+13h] [bp-1h]@13
  void *DstBufa; // [sp+1Ch] [bp+8h]@10
  void *Sourcea; // [sp+20h] [bp+Ch]@10

  File = FindContainer(pContainer, 0);
  v6 = pDst;
  if ( File && pDst->paletted_pixels
    && mode == 2
    && pDst->pPalette16 && !pDst->pPalette24
    && (v7 = pDst->uTextureSize,
        fread(pDst, 1, 0x30u, File),
        strcpy_s(pDst->pName, pContainer),
        v8 = pDst->uTextureSize,
        (signed int)v8 <= (signed int)v7) )
  {
    if ( !pDst->uDecompressedSize || this->_011BA4_debug_paletted_pixels_uncompressed)
    {
      fread(pDst->paletted_pixels, 1, pDst->uTextureSize, File);
    }
    else
    {
      Sourcea = malloc(pDst->uDecompressedSize);
      DstBufa = malloc(pDst->uTextureSize);
      fread(DstBufa, 1, pDst->uTextureSize, File);
      zlib::Uncompress(Sourcea, &v6->uDecompressedSize, DstBufa, v6->uTextureSize);
      v6->uTextureSize = pDst->uDecompressedSize;
      free(DstBufa);
      memcpy(v6->paletted_pixels, Sourcea, pDst->uDecompressedSize);
      free(Sourcea);
    }
    for( uint i = 0; i < 256; ++i )
    {
      fread(&DstBuf, 1, 1, File);
      fread(&v16, 1, 1, File);
      fread(&v15, 1, 1, File);
      v6->pPalette16[i] = (unsigned __int8)(DstBuf >> (8 - (unsigned char)(this->uTextureRedBits)))
                        << ((unsigned char)(this->uTextureBlueBits) + (unsigned char)(this->uTextureGreenBits));
      v6->pPalette16[i] += (unsigned __int8)(v16 >> (8 - (unsigned char)(this->uTextureGreenBits)))
                        << this->uTextureBlueBits;
      v6->pPalette16[i] += (unsigned __int8)(v15 >> (8 - (unsigned char)(this->uTextureBlueBits)));
    }
    result = 1;
  }
  else
    result = -1;
  return result;
}

//----- (0040FC08) --------------------------------------------------------
int LODFile_IconsBitmaps::LoadTextureFromLOD(Texture_MM7 *pOutTex, const char *pContainer, enum TEXTURE_TYPE eTextureType)
{
    Texture_MM7 *v8; // esi@3
    enum TEXTURE_TYPE v12; // eax@14
    signed int result; // esi@14
    unsigned int v14; // eax@21
    void *v19; // ST3C_4@27
    size_t v22; // ST2C_4@29
    const void *v23; // ecx@29
    void *v30; // eax@30
    signed int v41; // ecx@43
    signed int v42; // ecx@48

    FILE* pFile = FindContainer(pContainer, false);
    if (!pFile)
        return -1;
    v8 = pOutTex;
    fread(pOutTex, 1, 0x30, pFile);
    strcpy(pOutTex->pName, pContainer);

    // BITMAPS
    if (/*render->pRenderD3D &&*/ (pOutTex->pBits & 2) && strcmp(v8->pName, "sptext01"))
    {
        if (!pHardwareSurfaces || !pHardwareTextures)
        {
            pHardwareSurfaces = new IDirectDrawSurface *[1000];
            memset(pHardwareSurfaces, 0, 1000 * sizeof(IDirectDrawSurface4 *));

            pHardwareTextures = new IDirect3DTexture2 *[1000];
            memset(pHardwareTextures, 0, 1000 * sizeof(IDirect3DTexture2 *));

            ptr_011BB4 = new char[1000];
            memset(ptr_011BB4, 0, 1000);
        }
        if (_strnicmp(pContainer, "wtrdr", 5))
        {
            if (_strnicmp(pContainer, "WtrTyl", 6))
                v14 = uNumLoadedFiles;
            else
            {
                render->hd_water_tile_id = uNumLoadedFiles;
                v14 = uNumLoadedFiles;
            }
            //result = render->LoadTexture(pContainer, pOutTex->palette_id1, (void **)&pHardwareSurfaces[v14], (void **)&pHardwareTextures[v14]);
            result = 1;
        }
        else
        {
            char *temp_container;
            temp_container = (char *)malloc(strlen(pContainer) + 2);
            *temp_container = 104;//'h'
            strcpy(temp_container + 1, pContainer);
            //result = render->LoadTexture((const char *)temp_container, pOutTex->palette_id1,
            //    (void **)&pHardwareSurfaces[uNumLoadedFiles], (void **)&pHardwareTextures[uNumLoadedFiles]);
            result = 1;
            free((void *)temp_container);
        }
        return result;
    }


    // ICONS
    if (!v8->uDecompressedSize || _011BA4_debug_paletted_pixels_uncompressed)
    {
        v8->paletted_pixels = (unsigned __int8 *)malloc(v8->uTextureSize);
        fread(v8->paletted_pixels, 1, (size_t)v8->uTextureSize, pFile);
    }
    else
    {
        pContainer = (const char *)malloc(v8->uDecompressedSize);
        v19 = malloc(v8->uTextureSize);
        fread(v19, 1, (size_t)v8->uTextureSize, pFile);
        zlib::Uncompress((void *)pContainer, &v8->uDecompressedSize, v19, v8->uTextureSize);
        v8->uTextureSize = v8->uDecompressedSize;
        free(v19);
        if ( /*bUseLoResSprites*/false && v8->pBits & 2)
        {
            pOutTex = (Texture_MM7 *)(((signed int)v8->uSizeOfMaxLevelOfDetail >> 2)
                + ((signed int)v8->uSizeOfMaxLevelOfDetail >> 4)
                + ((signed int)v8->uSizeOfMaxLevelOfDetail >> 6));
            v22 = (size_t)pOutTex;
            v23 = &pContainer[v8->uTextureWidth * v8->uTextureHeight];
            v8->paletted_pixels = (unsigned __int8 *)malloc((unsigned int)pOutTex);
            memcpy(v8->paletted_pixels, v23, v22);
            v8->uTextureWidth = (signed __int16)v8->uTextureWidth / 2;
            v8->uTextureHeight = (signed __int16)v8->uTextureHeight / 2;
            --v8->uWidthLn2;
            --v8->uHeightLn2;
            v8->uWidthMinus1 = v8->uTextureWidth - 1;
            v8->uHeightMinus1 = v8->uTextureHeight - 1;
            v8->uSizeOfMaxLevelOfDetail = (signed __int16)v8->uTextureWidth * (signed __int16)v8->uTextureHeight;
            v8->uTextureSize = (unsigned int)pOutTex;
        }
        else
        {
            v8->paletted_pixels = (unsigned __int8 *)malloc(v8->uDecompressedSize);
            memcpy(v8->paletted_pixels, pContainer, v8->uDecompressedSize);
        }
        free((void *)pContainer);
    }

    free(v8->pPalette16);
    v8->pPalette16 = NULL;

    free(v8->pPalette24);
    v8->pPalette24 = NULL;

    if (eTextureType == TEXTURE_24BIT_PALETTE)
    {
        v8->pPalette24 = (unsigned __int8 *)malloc(0x300);
        fread(v8->pPalette24, 1, 0x300, pFile);
    }
    else
    {
        if (eTextureType == TEXTURE_16BIT_PALETTE)
        {
            v8->pPalette16 = (unsigned __int16 *)malloc(0x200);
            for (uint i = 0; i < 256; ++i)
            {
                fread((char *)&eTextureType + 3, 1, 1, pFile);
                fread((char *)&pContainer + 3, 1, 1, pFile);
                fread((char *)&pOutTex + 3, 1, 1, pFile);
                v8->pPalette16[i] = (unsigned __int8)(BYTE3(eTextureType) >> (8 - (unsigned char)this->uTextureRedBits)) //Uninitialized memory access
                    << ((unsigned char)this->uTextureBlueBits + (unsigned char)this->uTextureGreenBits);
                v8->pPalette16[i] += (unsigned __int8)(BYTE3(pContainer) >> (8 - (unsigned char)this->uTextureGreenBits))
                    << this->uTextureBlueBits;
                v8->pPalette16[i] += (unsigned __int8)(BYTE3(pOutTex) >> (8 - (unsigned char)this->uTextureBlueBits));
            }
        }
    }

    if (v8->pBits & 2)
    {
        v8->pLevelOfDetail1 = &v8->paletted_pixels[v8->uSizeOfMaxLevelOfDetail];
        //v8->pLevelOfDetail2 = &v8->pLevelOfDetail1[v8->uSizeOfMaxLevelOfDetail >> 2];
        //v8->pLevelOfDetail3 = &v8->pLevelOfDetail2[v8->uSizeOfMaxLevelOfDetail >> 4];
    }
    else
    {
        v8->pLevelOfDetail1 = 0;
        //v8->pLevelOfDetail2 = 0;
        //v8->pLevelOfDetail3 = 0;
    }
    for (v41 = 1; v41 < 15; ++v41)
    {
        if (1 << v41 == v8->uTextureWidth)
            v8->uWidthLn2 = v41;
    }
    for (v42 = 1; v42 < 15; ++v42)
    {
        if (1 << v42 == v8->uTextureHeight)
            v8->uHeightLn2 = v42;
    }

    switch (v8->uWidthLn2)
    {
    case 2:
        v8->uWidthMinus1 = 3;
        break;
    case 3:
        v8->uWidthMinus1 = 7;
        break;
    case 4:
        v8->uWidthMinus1 = 15;
        break;
    case 5:
        v8->uWidthMinus1 = 31;
        break;
    case 6:
        v8->uWidthMinus1 = 63;
        break;
    case 7:
        v8->uWidthMinus1 = 127;
        break;
    case 8:
        v8->uWidthMinus1 = 255;
        break;
    case 9:
        v8->uWidthMinus1 = 511;
        break;
    case 10:
        v8->uWidthMinus1 = 1023;
        break;
    case 11:
        v8->uWidthMinus1 = 2047;
        break;
    case 12:
        v8->uWidthMinus1 = 4095;
        break;
    default:
        break;
    }
    switch (v8->uHeightLn2)
    {
    case 2:
        v8->uHeightMinus1 = 3;
        break;
    case 3:
        v8->uHeightMinus1 = 7;
        break;
    case 4:
        v8->uHeightMinus1 = 15;
        break;
    case 5:
        v8->uHeightMinus1 = 31;
        break;
    case 6:
        v8->uHeightMinus1 = 63;
        break;
    case 7:
        v8->uHeightMinus1 = 127;
        break;
    case 8:
        v8->uHeightMinus1 = 255;
        break;
    case 9:
        v8->uHeightMinus1 = 511;
        break;
    case 10:
        v8->uHeightMinus1 = 1023;
        break;
    case 11:
        v8->uHeightMinus1 = 2047;
        break;
    case 12:
        v8->uHeightMinus1 = 4095;
        break;
    default:
        return 1;
    }
    return 1;
}

Texture_MM7 *LODFile_IconsBitmaps::LoadTexturePtr(const char *pContainer, enum TEXTURE_TYPE uTextureType)
{
  uint id = LoadTexture(pContainer, uTextureType);

  Assert(id != -1 && L"Texture_MM7 not found");

  return &pTextures[id];
}

//----- (0040FB20) --------------------------------------------------------
unsigned int LODFile_IconsBitmaps::LoadTexture(const char *pContainer, enum TEXTURE_TYPE uTextureType)
{
    for (uint i = 0; i < uNumLoadedFiles; ++i)
    {
        if (!_stricmp(pContainer, pTextures[i].pName))
            return i;
    }

    Assert(uNumLoadedFiles < 1000);

    if (LoadTextureFromLOD(&pTextures[uNumLoadedFiles], pContainer, uTextureType) == -1)
    {
        for (uint i = 0; i < uNumLoadedFiles; ++i)
        {
            if (!_stricmp(pTextures[i].pName, "pending"))
                return i;
        }
        LoadTextureFromLOD(&pTextures[uNumLoadedFiles], "pending", uTextureType);
    }

    return uNumLoadedFiles++;
}

Texture_MM7 * LODFile_IconsBitmaps::GetTexture( int idx )
{
  Assert(idx < MAX_LOD_TEXTURES, "Texture_MM7 index out of bounds (%u)", idx);
  if (idx == -1) 
  {
    //logger->Warning(L"Texture_MM7 id = %d missing", idx);
    return pTextures + LoadDummyTexture();
  }
  return pTextures + idx;
}

//----- (0046082C) --------------------------------------------------------
bool Initialize_GamesLOD_NewLOD()
{
  pGames_LOD = new LODWriteableFile;
  pGames_LOD->AllocSubIndicesAndIO(300, 0);
  if (pGames_LOD->LoadFile("data\\games.lod", 1))
  {
    pNew_LOD = new LODWriteableFile;
    pNew_LOD->AllocSubIndicesAndIO(300, 100000);
    return true;
  }
  return false;
}
