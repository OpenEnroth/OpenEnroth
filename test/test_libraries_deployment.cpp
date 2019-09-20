#include <zlib.h>

#include <al/al.h>
#include <al/alc.h>

#include <SDL.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

SDLMAIN_DECLSPEC int main(int argc, char* argv[]) {
    // ensure zlib was installed successfully
    deflateInit(nullptr, 0);

    // ensure openal was installed successfully
    auto a = alcOpenDevice(nullptr);

    // ensure sdl2 was installed successfully
    SDL_Delay(1000);

    // ensure ffmpeg was installed successfully
    auto b = avcodec_find_decoder(AV_CODEC_ID_H264);

    return 0;
}
