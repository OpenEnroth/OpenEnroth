#include "Engine/Graphics/Image.h"

#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/OurMath.h"
#include "Engine/ZlibWrapper.h"

#include "Engine/Graphics/ImageFormatConverter.h"
#include "Engine/Graphics/ImageLoader.h"
#include "Engine/Graphics/Texture.h"

#include "Engine/Serialization/LegacyImages.h"

#include "Engine/Tables/FrameTableInc.h"

#include "PaletteManager.h"

struct TextureFrameTable *pTextureFrameTable;

stru355 stru_4E82A4 = {0x20, 0x41, 0, 0x20, 0xFF0000, 0xFF00, 0xFF, static_cast<int>(0xFF000000)};
stru355 stru_4EFCBC = {0x20, 0x41, 0, 0x10, 0x7C00, 0x3E0, 0x1F, 0x8000};

// Texture_MM7 pTex_F7CE30;

const wchar_t *IMAGE_FORMAT_ToString(IMAGE_FORMAT format) {
    switch (format) {
        case IMAGE_FORMAT_R5G6B5:
            return L"IMAGE_FORMAT_R5G6B5";
        case IMAGE_FORMAT_A1R5G5B5:
            return L"IMAGE_FORMAT_A1R5G5B5";
        case IMAGE_FORMAT_A8R8G8B8:
            return L"IMAGE_FORMAT_A8R8G8B8";
        case IMAGE_FORMAT_R8G8B8:
            return L"IMAGE_FORMAT_R8G8B8";
        case IMAGE_FORMAT_R8G8B8A8:
            return L"IMAGE_FORMAT_R8G8B8A8";

        default:
            Error("Invalid format: %d", format);
            return L"Invalid format";
    }
}

unsigned int IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT format) {
    switch (format) {
        case IMAGE_FORMAT_R5G6B5:
            return 2;
        case IMAGE_FORMAT_A1R5G5B5:
            return 2;
        case IMAGE_FORMAT_A8R8G8B8:
            return 4;
        case IMAGE_FORMAT_R8G8B8:
            return 3;
        case IMAGE_FORMAT_R8G8B8A8:
            return 4;

        default:
            Error("Invalid format: %d", format);
            return 0;
    }
}

Texture *TextureFrame::GetTexture() {
    if (!this->tex) {
        this->tex = assets->GetBitmap(this->name);
    }
    return this->tex;
}

void TextureFrameTable::ToFile() {
    TextureFrameTable *v1 = this;
    FILE *file = fopen("data\\dtft.bin", "wb");
    if (file == nullptr) {
        Error("Unable to save dtft.bin!", 0);
    }
    fwrite(v1, 4u, 1u, file);
    fwrite(v1->pTextures, 0x14u, v1->sNumTextures, file);
    fclose(file);
}

void TextureFrameTable::FromFile(void *data_mm6, void *data_mm7, void *data_mm8) {
    uint num_mm6_frames = data_mm6 ? *(int *)data_mm6 : 0,
         num_mm7_frames = data_mm7 ? *(int *)data_mm7 : 0,
         num_mm8_frames = data_mm8 ? *(int *)data_mm8 : 0;

    this->sNumTextures =
        /*num_mm6_frames + */ num_mm7_frames /*+ num_mm8_frames*/;
    Assert(sNumTextures, 0);
    // Assert(!num_mm8_frames);

    TextureFrame_MM7 *frame_data =
        (TextureFrame_MM7 *)((unsigned char *)data_mm7 + 4);
    auto frames = new TextureFrame[this->sNumTextures];
    for (unsigned int i = 0; i < this->sNumTextures; ++i) {
        frames[i].name = frame_data->pTextureName;
        std::transform(frames[i].name.begin(), frames[i].name.end(),
                       frames[i].name.begin(), ::tolower);

        frames[i].uAnimLength = frame_data->uAnimLength;
        frames[i].uAnimTime = frame_data->uAnimTime;
        frames[i].uFlags = frame_data->uFlags;

        ++frame_data;
    }

    // pTextures = (TextureFrame *)malloc(sNumTextures * sizeof(TextureFrame));
    // memcpy(pTextures,                                   (char *)data_mm7 + 4,
    // num_mm7_frames * sizeof(TextureFrame)); memcpy(pTextures + num_mm7_frames,
    // (char *)data_mm6 + 4, num_mm6_frames * sizeof(TextureFrame));
    // memcpy(pTextures + num_mm6_frames + num_mm7_frames, (char *)data_mm8 + 4,
    // num_mm8_frames * sizeof(TextureFrame));

    this->pTextures = frames;
}

void TextureFrameTable::LoadAnimationSequenceAndPalettes(int uFrameID) {
    if (uFrameID <= this->sNumTextures && uFrameID >= 0) {
        for (unsigned int i = uFrameID;; ++i) {
            // this->pTextures[i].uTextureID =
            // pBitmaps_LOD->LoadTexture(this->pTextures[i].pTextureName,
            // TEXTURE_DEFAULT);

            // if (this->pTextures[i].uTextureID != -1)
            //    pBitmaps_LOD->pTextures[this->pTextures[i].uTextureID].palette_id2
            //    =
            //    pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[this->pTextures[i].uTextureID].palette_id1);

            if (this->pTextures[i].uFlags & 1) break;
        }
    }
    return;
}

int TextureFrameTable::FindTextureByName(const char *Str2) {
    String name = Str2;
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    for (unsigned int i = 0; i < this->sNumTextures; ++i) {
        if (this->pTextures[i].name == name) return i;
    }
    return -1;
}

Texture *TextureFrameTable::GetFrameTexture(int uFrameID, signed int a3) {
    int v3 = uFrameID;
    TextureFrame *v4 = this->pTextures;
    TextureFrame *v5 = &v4[uFrameID];
    int v6 = v5->uAnimLength;
    if (v5->uFlags & 1 && (v6 != 0)) {
        int v7 = (a3 >> 3) % v6;
        for (char *i = (char *)&v5->uAnimTime;; i += 20) {
            int v9 = *(short *)i;
            if (v7 <= v9) break;
            v7 -= v9;
            ++v3;
        }
        return v4[v3].GetTexture();
    } else {
        return v5->GetTexture();
    }
}

void Texture_MM7::Release() {
    header.pName[0] = 0;

    if (header.pBits & 0x0400) {
        __debugbreak();
    }

    if (paletted_pixels != nullptr) {
        free(paletted_pixels);
        paletted_pixels = nullptr;
    }

    if (pPalette24 != nullptr) {
        free(pPalette24);
        pPalette24 = nullptr;
    }

    pLevelOfDetail1 = nullptr;

    memset(&header, 0, sizeof(header));
}

Texture_MM7::Texture_MM7() {
    memset(&header, 0, sizeof(header));
    paletted_pixels = nullptr;
    pLevelOfDetail1 = nullptr;
    pPalette24 = nullptr;
}

int TextureFrameTable::FromFileTxt(const char *Args) {
    TextureFrameTable *v2;  // ebx@1
    FILE *v3;               // eax@1
    int v4;                 // esi@3
    const void *v5;         // ST0C_4@10
    void *v6;               // eax@10
    FILE *v7;               // ST0C_4@12
    char *i;                // eax@12
    signed int v9;          // esi@15
                            //    int v10; // eax@17
    int v11;                // edx@22
    int v12;                // ecx@23
    int v13;                // eax@24
    signed int j;           // eax@27
    TextureFrame *v15;      // edx@28
    int v16;                // esi@28
    int k;                  // ecx@29
    char Buf;               // [sp+Ch] [bp-2F8h]@3
    FrameTableTxtLine v20;  // [sp+200h] [bp-104h]@4
    int v21;                // [sp+27Ch] [bp-88h]@4
    char *Str1;             // [sp+280h] [bp-84h]@5
    char *Str;              // [sp+284h] [bp-80h]@15
    int v24;                // [sp+2F8h] [bp-Ch]@3
    int v25;                // [sp+2FCh] [bp-8h]@3
    FILE *File;             // [sp+300h] [bp-4h]@1
    int Argsa;              // [sp+30Ch] [bp+8h]@28

    v2 = this;
    v3 = fopen(Args, "r");
    File = v3;
    if (!v3) Error("CTextureFrameTable::load - Unable to open file: %s.", Args);

    v4 = 0;
    v24 = 0;
    v25 = 1;
    if (fgets(&Buf, 490, v3)) {
        do {
            *strchr(&Buf, 10) = 0;
            memcpy(&v21, txt_file_frametable_parser(&Buf, &v20), 0x7Cu);
            __debugbreak();  // warning C4700: uninitialized local variable
                             // 'Str1' used
            if (v21 && *Str1 != 47) {
                if (v21 < 2)
                    Error(
                        "CTextureFrameTable::load, too few arguments, %s line "
                        "%i.",
                        Args, v25);
                ++v24;
            }
            ++v25;
        } while (fgets(&Buf, 490, File));
        v4 = v24;
    }
    v5 = v2->pTextures;
    v2->sNumTextures = v4;
    v6 = malloc(sizeof(TextureFrame) * v4);
    v2->pTextures = (TextureFrame *)v6;
    if (!v6) Error("CTextureFrameTable::load - Out of Memory!", 0);
    v7 = File;
    v2->sNumTextures = 0;
    fseek(v7, 0, 0);
    for (i = fgets(&Buf, 490, File); i; i = fgets(&Buf, 490, File)) {
        *strchr(&Buf, 10) = 0;
        memcpy(&v21, txt_file_frametable_parser(&Buf, &v20), 0x7Cu);
        if (v21 && *Str1 != 47) {
            // strcpy(v2->pTextures[v2->sNumTextures].pTextureName, Str1);
            __debugbreak();  // warning C4700: uninitialized local variable
                             // 'Str' used
            v2->pTextures[v2->sNumTextures].uAnimTime = atoi(Str);
            v9 = 2;
            for (v2->pTextures[v2->sNumTextures].uFlags = 0; v9 < v21; ++v9) {
                if (!_stricmp((&Str1)[4 * v9], "New")) {
                    // v10 = (int)&v2->pTextures[v2->sNumTextures].uFlags;
                    v2->pTextures[v2->sNumTextures].uFlags |= 2;
                }
            }
            ++v2->sNumTextures;
        }
    }
    fclose(File);
    v11 = 0;
    if ((signed int)(v2->sNumTextures - 1) > 0) {
        v12 = 0;
        do {
            v13 = (int)&v2->pTextures[v12];
            if (!(*(char *)(v13 + 38) & 2)) *(char *)(v13 + 18) |= 1u;
            ++v11;
            ++v12;
        } while (v11 < (signed int)(v2->sNumTextures - 1));
    }
    for (j = 0; j < (signed int)v2->sNumTextures;
         *(short *)(Argsa + 16) = v16) {
        v15 = v2->pTextures;
        Argsa = (int)&v15[j];
        v16 = *(short *)(Argsa + 14);
        if (*(char *)(Argsa + 18) & 1) {
            ++j;
            for (k = (int)&v15[j]; *(char *)(k + 18) & 1; k += 20) {
                v16 += *(short *)(k + 14);
                ++j;
            }
            HEXRAYS_LOWORD(v16) = v15[j].uAnimTime + v16;
        }
        ++j;
    }
    return 1;
}

Image *Image::Create(ImageLoader *loader) {
    Image *img = new Image();
    if (img) {
        img->loader = loader;
    }

    return img;
}

bool Image::LoadImageData() {
    if (!initialized) {
        void *data = nullptr;
        initialized = loader->Load(&width, &height, &data, &native_format);
        if (initialized && native_format != IMAGE_INVALID_FORMAT) {
            pixels[native_format] = data;
        }
    }

    return initialized;
}

unsigned int Image::GetWidth() {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        return width;
    }

    return 0;
}

unsigned int Image::GetHeight() {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        return height;
    }

    return 0;
}

Image *Image::Create(unsigned int width, unsigned int height,
                     IMAGE_FORMAT format, const void *pixels) {
    Image *img = new Image(false);
    if (img) {
        img->initialized = true;
        img->width = width;
        img->height = height;
        img->native_format = format;
        unsigned int num_pixels = img->GetWidth() * img->GetHeight();
        unsigned int num_pixels_bytes =
            num_pixels * IMAGE_FORMAT_BytesPerPixel(format);
        img->pixels[format] = new unsigned char[num_pixels_bytes];
        if (pixels) {
            memcpy(img->pixels[format], pixels, num_pixels_bytes);
        } else {
            memset(img->pixels[format], 0, num_pixels_bytes);
        }
    }

    return img;
}

const void *Image::GetPixels(IMAGE_FORMAT format) {
    if (!initialized) {
        LoadImageData();
    }

    if (initialized) {
        if (this->pixels[format]) {
            return this->pixels[format];
        }

        auto native_pixels = this->pixels[this->native_format];
        if (native_pixels) {
            static ImageFormatConverter
                converters[IMAGE_NUM_FORMATS][IMAGE_NUM_FORMATS] = {
                    // IMAGE_FORMAT_R5G6B5 ->
                    {
                        nullptr,                   // IMAGE_FORMAT_R5G6B5
                        nullptr,                   // IMAGE_FORMAT_A1R5G5B5
                        Image_R5G6B5_to_A8R8G8B8,  // IMAGE_FORMAT_A8R8G8B8
                        Image_R5G6B5_to_R8G8B8,    // IMAGE_FORMAT_R8G8B8
                        nullptr,                   // IMAGE_FORMAT_R8G8B8A8
                    },

                    // IMAGE_FORMAT_A1R5G5B5 ->
                    {
                        nullptr,                     // IMAGE_FORMAT_R5G6B5
                        nullptr,                     // IMAGE_FORMAT_A1R5G5B5
                        nullptr,                     // IMAGE_FORMAT_A8R8G8B8
                        nullptr,                     // IMAGE_FORMAT_R8G8B8
                        Image_A1R5G5B5_to_R8G8B8A8,  // IMAGE_FORMAT_R8G8B8A8
                    },

                    // IMAGE_FORMAT_A8R8G8B8 ->
                    {
                        Image_A8R8G8B8_to_R5G6B5,  // IMAGE_FORMAT_R5G6B5
                        nullptr,                   // IMAGE_FORMAT_A1R5G5B5
                        nullptr,                   // IMAGE_FORMAT_A8R8G8B8
                        nullptr,                   // IMAGE_FORMAT_R8G8B8
                        nullptr,                   // IMAGE_FORMAT_R8G8B8A8
                    },
                };

            ImageFormatConverter cvt = converters[this->native_format][format];
            if (cvt) {
                unsigned int num_pixels = this->GetWidth() * this->GetHeight();

                void *cvt_pixels =
                    new unsigned char[num_pixels *
                                      IMAGE_FORMAT_BytesPerPixel(format)];
                if (cvt(width * height, native_pixels, cvt_pixels)) {
                    return this->pixels[format] = cvt_pixels;
                } else {
                    delete[] cvt_pixels;
                    cvt_pixels = nullptr;
                }
            } else {
                logger->Warning(L"No ImageConverter defined from %s to %s",
                                IMAGE_FORMAT_ToString(this->native_format),
                                IMAGE_FORMAT_ToString(format));
            }
        }
    }
    return nullptr;
}

bool Image::Release() {
    if (loader) {
        assets->ReleaseImage(
            loader->GetResourceName());  // exception this nullptr
    }

    if (initialized) {
        if (loader) {
            delete loader;
            loader = nullptr;
        }

        for (unsigned int i = 0; i < IMAGE_NUM_FORMATS; ++i) {
            if (pixels[i]) {
                delete[] pixels[i];
                pixels[i] = nullptr;
            }
        }

        native_format = IMAGE_INVALID_FORMAT;
        width = 0;
        height = 0;
    }

    delete this;
    return true;
}
