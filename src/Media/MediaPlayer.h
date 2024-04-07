#pragma once

#include <string>
#include <memory>

#include "Library/Vid/VidReader.h"

#include "Media/Movie.h"

// MOVIE_3DOLogo  "3dologo"
// MOVIE_NWCLogo  "new world logo"
// MOVIE_JVC      "jvc"
// MOVIE_Intro    "Intro"
// MOVIE_Emerald  "Intro Post"
// MOVIE_Death    "losegame"
// MOVIE_Outro    "end_seq1"

class VideoList;
class FFmpegLogProxy;

class MPlayer {
 public:
    MPlayer();
    virtual ~MPlayer();

    void Initialize();
    void Unload();

    void PlayFullscreenMovie(std::string_view pMovieName);

    void OpenHouseMovie(std::string_view pMovieName, bool bLoop);
    void HouseMovieLoop();

    bool IsMoviePlaying() const;
    bool StopMovie();

 protected:
    std::unique_ptr<FFmpegLogProxy> logProxy;
    VidReader might_list;
    VidReader magic_list;
    std::string sInHouseMovie;

    Blob LoadMovie(std::string_view video_name);
};

extern MPlayer *pMediaPlayer;
extern PMovie pMovie_Track;
