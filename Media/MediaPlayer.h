#pragma once
#include "OSWindow.h"

#include "Engine/Engine.h"

#include "Engine/Graphics/Image.h"

#pragma pack(push, 1)

#pragma pack(pop)

#pragma pack(push, 1)
#pragma pack(pop)



extern "C"
{
    #include "lib/libavcodec/avcodec.h"
    #include "lib/libavformat/avformat.h"
    #include "lib/libavutil/avutil.h"
    #include "lib/libavutil/imgutils.h"
    #include "lib/libswscale/swscale.h"
    #include "lib/libswresample/swresample.h"
    #include "lib/libavutil/opt.h"
}
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")

#include "lib/OpenAL/al.h"
#include "lib/OpenAL/alc.h"
#pragma comment(lib, "OpenAL32.lib")

#pragma pack(push, 1)

struct MovieHeader
{
  char pVideoName[40];
  unsigned int uFileOffset;
};
struct SoundHeader
{
  char pSoundName[40];
  unsigned int uFileOffset;
  unsigned int uCompressedSize;
  unsigned int uDecompressedSize;
};

enum MovieType
{
  MOVIE_Invalid = 0x0,
  MOVIE_3DOLogo = 0x1,
  MOVIE_NWCLogo = 0x2,
  MOVIE_JVC = 0x3,
  MOVIE_Emerald = 0x4,
  MOVIE_Intro = 0x5,
  MOVIE_Death = 0x6,
  MOVIE_Outro = 0x7,
};

namespace Media
{
    class ITrack
    {
    public:
        virtual ~ITrack() {}

        virtual void Play(bool loop = false) = 0;
        virtual void Release() = 0;
    };

    class IMovie
    {
    public:
        virtual ~IMovie() {}

        virtual void Play() = 0;
        virtual void GetNextFrame(double dt, void *target_surface) = 0;
        virtual void Release() = 0;

    };

    class MPlayer
    {
    public:
        MPlayer();
        virtual ~MPlayer();

        //for video/////////////////////////////////////////////////
        int field_44;//final video
        unsigned int bFirstFrame;
        unsigned int bLoopPlaying;
        unsigned int bStopBeforeSchedule;
        OSWindow *window;
        int uMovieType;//0 - null, 1 - bik, 2 - smk
        char pCurrentMovieName[64];
        char pVideoFrameTextureFilename[32];
        MovieHeader *pMightVideoHeaders;
        MovieHeader *pMagicVideoHeaders;
        HANDLE hMightVid;
        HANDLE hMagicVid;
        unsigned int uNumMightVideoHeaders;
        unsigned int uNumMagicVideoHeaders;
        bool bPlaying_Movie;
        bool loop_current_file;
        DWORD time_video_begin;
        int current_movie_width;
        int current_movie_height;
        HANDLE hVidFile;
        int uSize;
        int uOffset;

        void Initialize(OSWindow *window);

        void OpenFullscreenMovie(const char *pFilename, unsigned int bLoop);
        void OpenHouseMovie(const char *pMovieName, unsigned int a3_1);

        void LoadMovie(const char *);
        void SelectMovieType();

        inline void PlayFullscreenMovie(MovieType movie_type, bool bShowMouseAfterPlayback)
        {
            extern unsigned int bNoVideo;
            if (bNoVideo) return;

            switch (movie_type)
            {
            case MOVIE_3DOLogo: FullscreenMovieLoop("3dologo", 0);        break;
            case MOVIE_NWCLogo: FullscreenMovieLoop("new world logo", 0); break;
            case MOVIE_JVC:     FullscreenMovieLoop("jvc", 0);            break;
            case MOVIE_Intro:   FullscreenMovieLoop("Intro", 0);          break;
            case MOVIE_Emerald: FullscreenMovieLoop("Intro Post", 0);     break;
            case MOVIE_Death:   FullscreenMovieLoop("losegame", 2);       break;
            case MOVIE_Outro:   FullscreenMovieLoop("end_seq1", 20);      break;

            default:
                Error("Invalid movie requested: %u", movie_type);
                break;
            }
        }
        void FullscreenMovieLoop(const char *pMovieName, int a2);
        void HouseMovieLoop();

        void ShowMM7IntroVideo_and_LoadingScreen();
        void Unload();
        ///////////////////////////////////////////////

        IMovie *LoadMovie(const wchar_t *name, int width, int height, int cache_ms);
        IMovie *LoadMovieFromLOD(HANDLE h, int readFunction(void*, uint8_t*, int), int64_t seekFunction(void*, int64_t, int), int width, int height);

        //for audio////////////////////////////////////
        HANDLE hAudioSnd;
        unsigned int uNumSoundHeaders;
        struct SoundHeader *pSoundHeaders;

        void LoadAudioSnd();
        ///////////////////////////////////////////////


        ITrack *LoadTrack(const wchar_t *name);

    protected:
        static int readFunction(void *, uint8_t *, int);
        static int64_t seekFunction(void *, int64_t, int);
    };
};
#pragma pack(pop)
extern Media::MPlayer *pMediaPlayer;
extern Media::IMovie *pMovie_Track;
extern Media::ITrack *pAudio_Track;
extern class Movie *movie;

extern int mSourceID;

extern void PlayMovie(const wchar_t * pFilename);
extern void PlayAudio(const wchar_t * pFilename);
extern void MovieRelease();
