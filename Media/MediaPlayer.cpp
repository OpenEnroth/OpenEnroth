#include "Media/MediaPlayer.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/mem.h>
    #include <libavutil/opt.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
}

#include <algorithm>
#include <cassert>
#include <chrono>
#include <deque>
#include <map>
#include <memory>
#include <queue>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/ErrorHandling.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Log.h"

#include "GUI/GUIWindow.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/Audio/OpenALSoundProvider.h"

#include "Platform/Api.h"
#include "Platform/OSWindow.h"


OpenALSoundProvider *provider = nullptr;

MPlayer *pMediaPlayer = nullptr;
PMovie pMovie_Track;

class AVStreamWrapper {
 public:
    AVStreamWrapper() {
        type = AVMEDIA_TYPE_UNKNOWN;
        stream_idx = -1;
        stream = nullptr;
        dec = nullptr;
        dec_ctx = nullptr;
    }

    virtual ~AVStreamWrapper() {
        close();
    }

    virtual void reset() {
        if (dec_ctx != nullptr) {
            avcodec_flush_buffers(dec_ctx);
        }
    }

    virtual void close() {
        type = AVMEDIA_TYPE_UNKNOWN;
        stream_idx = -1;
        stream = nullptr;
        dec = nullptr;
        if (dec_ctx != nullptr) {
            // Close the codec
            avcodec_close(dec_ctx);
            logger->Warning("close decoder context file\n");
            dec_ctx = nullptr;
        }
    }

    virtual bool open(AVFormatContext *format_ctx) = 0;

    virtual bool open(AVFormatContext *format_ctx, AVMediaType type_) {
        stream_idx = av_find_best_stream(format_ctx, type_, -1, -1, &dec, 0);
        if (stream_idx < 0) {
            close();
            fprintf(stderr, "ffmpeg: Unable to find audio stream\n");
            return false;
        }

        stream = format_ctx->streams[stream_idx];
        dec_ctx = avcodec_alloc_context3(dec);
        if (dec_ctx == nullptr) {
            close();
            return false;
        }

        if (avcodec_parameters_to_context(dec_ctx, stream->codecpar) < 0) {
            close();
            return false;
        }
        if (avcodec_open2(dec_ctx, dec, nullptr) < 0) {
            close();
            return false;
        }

        return true;
    }

    AVMediaType type;
    int stream_idx;
    AVStream *stream;
    AVCodec *dec;
    AVCodecContext *dec_ctx;
    std::queue<PMemBuffer, std::deque<PMemBuffer>> queue;
};

class AVAudioStream : public AVStreamWrapper {
 public:
    AVAudioStream() : AVStreamWrapper() { converter = nullptr; }

    virtual bool open(AVFormatContext *format_ctx) {
        if (!AVStreamWrapper::open(format_ctx, AVMEDIA_TYPE_AUDIO)) {
            return false;
        }

        converter = swr_alloc_set_opts(
            converter, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
            dec_ctx->sample_rate, dec_ctx->channel_layout, dec_ctx->sample_fmt,
            dec_ctx->sample_rate, 0, nullptr);
        if (swr_init(converter) < 0) {
            logger->Warning("swr_init: failed");
            swr_free(&converter);
            converter = nullptr;
            return false;
        }

        return true;
    }

    PMemBuffer decode_frame(AVPacket *avpacket) {
        PMemBuffer result;
        AVFrame *frame = av_frame_alloc();

        if (!queue.empty()) {
            result = queue.front();
            queue.pop();
        }

        if (avcodec_send_packet(dec_ctx, avpacket) >= 0) {
            int res = 0;
            while (res >= 0) {
                res = avcodec_receive_frame(dec_ctx, frame);
                if (res == AVERROR(EAGAIN) || res == AVERROR_EOF) {
                    break;
                }
                if (res < 0) {
                    av_frame_free(&frame);
                    return result;
                }
                PMemBuffer tmp_buf = AllocMemBuffer(
                    frame->nb_samples * 2 * 2);
                uint8_t *dst_channels[8] = { 0 };
                dst_channels[0] = (uint8_t*)tmp_buf->GetData();
                int got_samples = swr_convert(
                    converter, dst_channels, frame->nb_samples,
                    (const uint8_t**)frame->data, frame->nb_samples);
                if (got_samples > 0) {
                    if (!result) {
                        result = tmp_buf;
                    } else {
                        queue.push(tmp_buf);
                    }
                }
            }
        }

        av_frame_free(&frame);

        return result;
    }

 protected:
    SwrContext *converter;
};

class AVVideoStream : public AVStreamWrapper {
 public:
    AVVideoStream() : AVStreamWrapper() {
        frame_len = 0.;
        frames_per_second = 0.;
        converter = nullptr;
        height = 0;
        width = 0;
    }

    virtual bool open(AVFormatContext *format_ctx) {
        if (!AVStreamWrapper::open(format_ctx, AVMEDIA_TYPE_VIDEO)) {
            return false;
        }

        width = dec_ctx->width;
        height = dec_ctx->height;

        frame_len = av_q2d(stream->time_base) * 1000.;
        frames_per_second = 1. / av_q2d(stream->time_base);

        converter = sws_getContext(
            dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt, width, height,
            AV_PIX_FMT_RGB32, SWS_BICUBIC, nullptr, nullptr, nullptr);

        return true;
    }

    PMemBuffer decode_frame(AVPacket *avpacket) {
        PMemBuffer result;
        AVFrame *frame = av_frame_alloc();

        if (!queue.empty()) {
            result = queue.front();
            queue.pop();
        }

        if (avcodec_send_packet(dec_ctx, avpacket) >= 0) {
            int res = 0;
            while (res >= 0) {
                res = avcodec_receive_frame(dec_ctx, frame);
                if (res == AVERROR(EAGAIN) || res == AVERROR_EOF) {
                    break;
                }
                if (res < 0) {
                    av_frame_free(&frame);
                    return result;
                }
                int linesizes[4] = { 0, 0, 0, 0 };
                if (av_image_fill_linesizes(linesizes, AV_PIX_FMT_RGB32, width) < 0) {
                    assert(false);
                }
                uint8_t *data[4] = { nullptr, nullptr, nullptr, nullptr };
                PMemBuffer tmp_buf = AllocMemBuffer(frame->height * linesizes[0] * 2);
                data[0] = (uint8_t*)tmp_buf->GetData();

                if (sws_scale(converter, frame->data, frame->linesize, 0, frame->height,
                    data, linesizes) < 0) {
                    assert(false);
                }

                if (!result) {
                    result = tmp_buf;
                } else {
                    queue.push(tmp_buf);
                }
            }
        }

        av_frame_free(&frame);

        last_frame = result;

        return result;
    }

    PMemBuffer last_frame;
    double frames_per_second;
    double frame_len;
    SwsContext *converter;
    int width;
    int height;
};

class Movie : public IMovie {
 public:
    Movie() {
        width = 0;
        height = 0;
        format_ctx = nullptr;
        playback_time = 0.0;

        last_resampled_frame_num = -1;

        audio_data_in_device = nullptr;
        ioBuffer = nullptr;
        format_ctx = nullptr;
        avioContext = nullptr;

        looping = false;
        playing = false;

        uFileSize = 0;
        uFilePos = 0;
        uFileOffset = 0;
        hFile = nullptr;
    }

    virtual ~Movie() { Close(); }

    void Close() {
        ReleaseAVCodec();

        if (audio_data_in_device) {
            provider->DeleteStreamingTrack(&audio_data_in_device);
        }
    }

    inline void ReleaseAVCodec() {
        audio.close();
        video.close();

        if (format_ctx) {
            // Close the video file
            avformat_close_input(&format_ctx);
            logger->Warning("close video format context file\n");
            format_ctx = nullptr;
        }
        if (avioContext) {
            av_free(avioContext);
            avioContext = nullptr;
        }
        if (ioBuffer) {
            // av_free(ioBuffer);
            ioBuffer = nullptr;
        }
    }

    bool Load(const char *filename) {  // Загрузка
        // Open video file
        if (avformat_open_input(&format_ctx, filename, nullptr, nullptr) < 0) {
            logger->Warning("ffmpeg: Unable to open input file");
            Close();
            return false;
        }

        // Retrieve stream information
        if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
            fprintf(stderr, "ffmpeg: Unable to find stream info\n");
            Close();
            return false;
        }

        // Dump information about file onto standard error
        av_dump_format(format_ctx, 0, filename, 0);

        audio.open(format_ctx);

        if (!video.open(format_ctx)) {
            Error("Cannot open video stream: %s", filename);
            Close();
            return false;
        }

        width = video.width;
        height = video.height;

        if (audio.stream_idx >= 0) {
            audio_data_in_device = provider->CreateStreamingTrack16(2, audio.dec_ctx->sample_rate, 2);
        }

        return true;
    }

    bool LoadFromLOD(FILE *f, size_t size, size_t offset) {
        hFile = f;
        uFileSize = size;
        uFileOffset = offset;
        uFilePos = 0;

        if (!ioBuffer) {
            ioBuffer = (unsigned char *)av_malloc(AV_INPUT_BUFFER_MIN_SIZE);  // can get av_free()ed by libav
        }

        if (!avioContext) {
            avioContext = avio_alloc_context(ioBuffer, 0x4000, 0, this, s_read, NULL, s_seek);
        }
        if (!format_ctx) {
            format_ctx = avformat_alloc_context();
        }
        format_ctx->pb = avioContext;
        return Load("dummyFilename");
    }

    virtual PMemBuffer GetFrame() {
        if (!playing) {
            return nullptr;
        }

        auto current_time = std::chrono::system_clock::now();
        std::chrono::duration<double, std::ratio<1>> diff = current_time - start_time;
        std::chrono::milliseconds diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
        playback_time += diff_ms.count();
        start_time = std::chrono::system_clock::now();

        int desired_frame_number = (int)((playback_time / video.frame_len) + 0.5);
        if (last_resampled_frame_num == desired_frame_number) {
            return video.last_frame;
        }
        last_resampled_frame_num++;
        if (last_resampled_frame_num == video.stream->duration) {
            if (looping) {
                video.reset();
                audio.reset();
                int err = av_seek_frame(format_ctx, -1, 0,
                                        AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);
                char err_buf[2048];
                av_strerror(err, err_buf, 2048);
                if (err < 0) {
                    Close();
                    return nullptr;
                }
                last_resampled_frame_num = 0;
                playback_time = 0;
                desired_frame_number = 0;
            } else {
                playing = false;
                return nullptr;
            }
        }

        AVPacket *avpacket = av_packet_alloc();

        // keep reading packets until we hit the end or find a video packet
        do {
            if (av_read_frame(format_ctx, avpacket) < 0) {
                // probably movie is finished
                playing = false;
                av_packet_free(&avpacket);
                return nullptr;
            }

            // Is this a packet from the video stream?
            // audio packet - queue into playing
            if (avpacket->stream_index == audio.stream_idx) {
                PMemBuffer buffer = audio.decode_frame(avpacket);
                if (buffer) {
                    provider->Stream16(audio_data_in_device,
                                       buffer->GetSize() / 2,
                                       buffer->GetData());
                }
            } else if (avpacket->stream_index == video.stream_idx) {
              // Decode video frame
              // video packet - decode & maybe show
              video.decode_frame(avpacket);
            } else {
                assert(false);  // unknown stream
            }
        } while (avpacket->stream_index != video.stream_idx ||
                 avpacket->pts != desired_frame_number);

        av_packet_free(&avpacket);

        return video.last_frame;
    }

    virtual void PlayBink() {
        // fix for #39 - choppy sound with bink

        AVPacket packet;

        Rect rect;
        rect.x = 0;
        rect.y = 0;
        rect.z = rect.x + window->GetWidth();
        rect.w = rect.y + window->GetHeight();

        // create texture
        Texture* tex = render->CreateTexture_Blank(pMovie_Track->GetWidth(), pMovie_Track->GetHeight(), IMAGE_FORMAT_A8R8G8B8);

        // holds decoded audio
        std::queue<PMemBuffer, std::deque<PMemBuffer>> buffq;

        // loop through once and add all audio packets to queue
        while (av_read_frame(format_ctx, &packet) >= 0) {
            if (packet.stream_index == audio.stream_idx) {
                PMemBuffer buffer = audio.decode_frame(&packet);
                if (buffer) buffq.push(buffer);
            }
        }
        logger->Info("Audio Packets Queued");

        // reset video to start
        int err = av_seek_frame(format_ctx, -1, 0, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);
        if (err < 0) {
            logger->Info("Seek to start failed! - Exit Movie");
            return;
        }
        start_time = std::chrono::system_clock::now();
        logger->Info("Video stream reset");

        int lastvideopts = -1;
        int desired_frame_number;
        auto current_time = std::chrono::system_clock::now();
        int audioupdaterate = (30.0f * video.frame_len) / 1000.0f;

        // loop through and do video
        while (av_read_frame(format_ctx, &packet) >= 0) {
            do {
                // get playback time - and wait till we need the next frame
                current_time = std::chrono::system_clock::now();
                playback_time = (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time)).count();
                desired_frame_number = (int)((playback_time / video.frame_len)/* + 0.25*/);
                OS_Sleep(5);
            } while (lastvideopts == desired_frame_number);

            // ignore audio packets
            if (packet.stream_index == audio.stream_idx) { continue; }

            if (packet.stream_index == video.stream_idx) {
                // check if anymore sound frames still in decoder
                PMemBuffer buffer = audio.decode_frame(NULL);
                if (buffer) buffq.push(buffer);

                // stream required sound frames
                // nwc and intro are 15fps vid but need 30fps sound
                // jvc is 10fps video but need 30 fps sound
                for (int i = 0; i < audioupdaterate; i++) {
                    if (!buffq.empty()) {
                        provider->Stream16(audio_data_in_device,
                                            buffq.front()->GetSize() / 2,
                                            buffq.front()->GetData());
                        buffq.pop();
                    }
                }

                // Decode video frame and show
                lastvideopts = packet.pts;
                video.decode_frame(&packet);
                PMemBuffer tmp_buf = video.last_frame;

                render->BeginScene();
                // update pixels from buffer
                uint32_t* pix = (uint32_t*)tex->GetPixels(IMAGE_FORMAT_A8R8G8B8);
                unsigned int num_pixels = tex->GetWidth() * tex->GetHeight();
                unsigned int num_pixels_bytes = num_pixels * IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT_A8R8G8B8);
                memcpy(pix, tmp_buf->GetData(), num_pixels_bytes);

                // update texture
                render->Update_Texture(tex);
                render->DrawImage(tex, rect);
                render->EndScene();
                render->Present();
            }

            MessageLoopWithWait();

            av_packet_unref(&packet);

            // exit movie
            if (!playing) break;
        }
        // hold for frame length at end of packets
        OS_Sleep(video.frame_len);

        // clean up
        while (!buffq.empty()) buffq.pop();
        tex->Release();

        return;
    }

    virtual String GetFormat() {
        return format_ctx->iformat->name;
    }

    virtual unsigned int GetWidth() const { return width; }

    virtual unsigned int GetHeight() const { return height; }

    virtual bool Play(bool loop = false) {
        start_time = std::chrono::system_clock::now();
        looping = loop;
        playing = true;
        return false;
    }

    virtual bool Stop() {
        playing = false;
        return false;
    }

    virtual bool IsPlaying() const { return playing; }

 protected:
    static int s_read(void *opaque, uint8_t *buf, int buf_size) {
        Movie *_this = (Movie *)opaque;
        return _this->read(opaque, buf, buf_size);
    }

    static int64_t s_seek(void *opaque, int64_t offset, int whence) {
        Movie *_this = (Movie *)opaque;
        return _this->seek(opaque, offset, whence);
    }

    int read(void *opaque, uint8_t *buf, int buf_size) {
        fseek(hFile, uFileOffset + uFilePos, SEEK_SET);
        buf_size = std::min(buf_size, (int)uFileSize - (int)uFilePos);
        buf_size = fread(buf, 1, buf_size, hFile);
        uFilePos += buf_size;
        return buf_size;
    }

    int64_t seek(void *opaque, int64_t offset, int whence) {
        if (whence == AVSEEK_SIZE) {
            return uFileSize;
        }

        switch (whence) {
            case SEEK_SET:
                uFilePos = (size_t)offset;
                break;
            case SEEK_CUR:
                uFilePos += (size_t)offset;
                break;
            case SEEK_END:
                uFilePos = uFileSize - (size_t)offset;
                break;
            default:
                assert(false);
                break;
        }
        fseek(hFile, uFileOffset + uFilePos, SEEK_SET);
        return uFilePos;
    }

 protected:
    unsigned int width;
    unsigned int height;
    AVFormatContext *format_ctx;
    double playback_time;

    AVAudioStream audio;
    unsigned char *ioBuffer;
    AVIOContext *avioContext;
    OpenALSoundProvider::StreamingTrackBuffer *audio_data_in_device;

    AVVideoStream video;
    int last_resampled_frame_num;

    std::chrono::time_point<std::chrono::system_clock> start_time;
    bool looping;
    bool playing;

    FILE *hFile;
    size_t uFileSize;
    size_t uFileOffset;
    size_t uFilePos;
};

// for video/////////////////////////////////////////////////////////////////

class VideoList {
 protected:
#pragma pack(push, 1)
    struct MovieHeader {
        char pVideoName[40];
        unsigned int uFileOffset;
    };
#pragma pack(pop)

    struct Node {
        size_t offset;
        size_t size;
    };
    typedef std::map<std::string, Node> Nodes;

 public:
    VideoList() { file = nullptr; }

    virtual ~VideoList() {
        if (file != nullptr) {
            fclose(file);
            file = nullptr;
        }
    }

    void Initialize(const std::string &file_path) {
        static_assert(sizeof(MovieHeader) == 44, "Wrong type size");

        if (engine->config->NoVideo()) {
            return;
        }

        file = fcaseopen(file_path.c_str(), "rb");
        if (file == nullptr) {
            logger->Warning("Can't open video file: %s", file_path.c_str());
            return;
        }
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        uint32_t uNumVideoHeaders = 0;
        fread(&uNumVideoHeaders, 1, 4, file);

        std::vector<MovieHeader> headers;
        headers.resize(uNumVideoHeaders);
        if (fread(&headers[0], sizeof(MovieHeader), uNumVideoHeaders, file) !=
            uNumVideoHeaders) {
            return;
        }
        std::sort(headers.begin(), headers.end(),
                  [](MovieHeader &a, MovieHeader &b) {
                      return a.uFileOffset < b.uFileOffset;
                  });

        for (size_t i = 0; i < headers.size(); i++) {
            std::string name = headers[i].pVideoName;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            Node node;
            node.offset = headers[i].uFileOffset;
            if (i < headers.size() - 1) {
                node.size = headers[i + 1].uFileOffset - headers[i].uFileOffset;
            } else {
                node.size = file_size - headers[i].uFileOffset;
            }
            nodes[name] = node;
        }
    }

    bool find(const std::string &video_name, FILE *&file_, size_t &offset,
              size_t &size) {
        file_ = nullptr;
        offset = 0;
        size = 0;

        std::string name = video_name;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        Nodes::iterator it = nodes.find(name);
        if (it != nodes.end()) {
            file_ = file;
            offset = it->second.offset;
            size = it->second.size;
            return true;
        }

        return false;
    }

 protected:
    Nodes nodes;
    FILE *file;
};

void MPlayer::Initialize() {
    might_list = new VideoList();
    std::string filename = MakeDataPath("anims/might7.vid");
    might_list->Initialize(filename);

    magic_list = new VideoList();
    filename = MakeDataPath("anims/magic7.vid");
    magic_list->Initialize(filename);
}

void MPlayer::OpenHouseMovie(const std::string &pMovieName, bool bLoop) {
    if (IsMoviePlaying()) {
        return;
    }

    pEventTimer->Pause();
    pAudioPlayer->MusicPause();
    size_t size = 0;
    size_t offset = 0;
    FILE *file = LoadMovie(pMovieName, size, offset);
    if (file == nullptr) {
        return;
    }

    std::shared_ptr<Movie> pMovie = std::make_shared<Movie>();
    pMovie->LoadFromLOD(file, size, offset);
    pMovie_Track = std::dynamic_pointer_cast<IMovie>(pMovie);
    sInHouseMovie = pMovieName;
}

void MPlayer::HouseMovieLoop() {
    if (!pMovie_Track || engine->config->NoVideo()) {
        return;
    }

    if (!pMovie_Track->IsPlaying()) {
        pMovie_Track->Play(false);
    }

    render->BeginScene();

    static Texture *tex;
    if (!tex) {
        tex = render->CreateTexture_Blank(pMovie_Track->GetWidth(), pMovie_Track->GetHeight(), IMAGE_FORMAT_A8R8G8B8);
    }

    PMemBuffer buffer = pMovie_Track->GetFrame();
    if (buffer) {
        Rect rect;
        rect.x = 8;
        rect.y = 8;
        rect.z = rect.x + game_viewport_width;
        rect.w = rect.y + game_viewport_height;

        // update pixels from buffer
        uint32_t *pix = (uint32_t*)tex->GetPixels(IMAGE_FORMAT_A8R8G8B8);
        unsigned int num_pixels = tex->GetWidth() * tex->GetHeight();
        unsigned int num_pixels_bytes = num_pixels * IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT_A8R8G8B8);
        memcpy(pix, buffer->GetData(), num_pixels_bytes);

        // update texture
        render->Update_Texture(tex);
        render->DrawImage(tex, rect);

    } else {
        pMovie_Track = nullptr;
        size_t size = 0;
        size_t offset = 0;
        FILE *file = LoadMovie(sInHouseMovie, size, offset);
        if (file != nullptr) {
            std::shared_ptr<Movie> pMovie = std::make_shared<Movie>();
            pMovie->LoadFromLOD(file, size, offset);
            pMovie_Track = std::dynamic_pointer_cast<IMovie>(pMovie);
            pMovie_Track->Play();
        }
        // callback to prevent skipped frame draw
        HouseMovieLoop();
    }

    render->EndScene();
}

void MPlayer::PlayFullscreenMovie(const std::string &pFilename) {
    if (engine->config->NoVideo()) {
        return;
    }

    size_t size = 0;
    size_t offset = 0;
    FILE *file = LoadMovie(pFilename, size, offset);
    if (file == nullptr) {
        return;
    }

    std::shared_ptr<Movie> pMovie = std::make_shared<Movie>();
    if (!pMovie->LoadFromLOD(file, size, offset)) {
        return;
    }
    pMovie_Track = std::dynamic_pointer_cast<IMovie>(pMovie);

    pEventTimer->Pause();
    pAudioPlayer->MusicPause();
    OS_ShowCursor(false);
    current_screen_type = CURRENT_SCREEN::SCREEN_VIDEO;

    pMovie_Track->Play();

    Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.z = rect.x + window->GetWidth();
    rect.w = rect.y + window->GetHeight();

    // create texture
    Texture *tex = render->CreateTexture_Blank(pMovie_Track->GetWidth(), pMovie_Track->GetHeight(), IMAGE_FORMAT_A8R8G8B8);

    if (pMovie->GetFormat() == "bink") {
        logger->Info("bink file");
        pMovie->PlayBink();
    } else {
        while (true) {
            MessageLoopWithWait();

            render->BeginScene();

            OS_Sleep(30);

            PMemBuffer buffer = pMovie_Track->GetFrame();
            if (!buffer) {
                break;
            }

            // update pixels from buffer
            uint32_t* pix = (uint32_t*)tex->GetPixels(IMAGE_FORMAT_A8R8G8B8);
            unsigned int num_pixels = tex->GetWidth() * tex->GetHeight();
            unsigned int num_pixels_bytes = num_pixels * IMAGE_FORMAT_BytesPerPixel(IMAGE_FORMAT_A8R8G8B8);
            memcpy(pix, buffer->GetData(), num_pixels_bytes);

            // update texture
            render->Update_Texture(tex);
            render->DrawImage(tex, rect);

            render->EndScene();
            render->Present();
        }
    }

    // release texture
    tex->Release();

    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
    pMovie_Track = nullptr;

    OS_ShowCursor(true);
}

bool MPlayer::IsMoviePlaying() const {
    if (!pMovie_Track) {
        return false;
    }

    return pMovie_Track->IsPlaying();
}

bool MPlayer::StopMovie() {
    if (pMovie_Track) {
        return pMovie_Track->Stop();
    }
    return false;
}

FILE *MPlayer::LoadMovie(const std::string &video_name, size_t &size, size_t &offset) {
    std::string pVideoNameBik = video_name + ".bik";
    std::transform(pVideoNameBik.begin(), pVideoNameBik.end(),
                   pVideoNameBik.begin(), ::tolower);
    std::string pVideoNameSmk = video_name + ".smk";
    std::transform(pVideoNameSmk.begin(), pVideoNameSmk.end(),
                   pVideoNameSmk.begin(), ::tolower);

    FILE *file = nullptr;
    offset = 0;
    size = 0;

    if (might_list != nullptr) {
        if (might_list->find(pVideoNameBik, file, offset, size)) {
            return file;
        }
        if (might_list->find(pVideoNameSmk, file, offset, size)) {
            return file;
        }
    }

    if (magic_list != nullptr) {
        if (magic_list->find(pVideoNameBik, file, offset, size)) {
            return file;
        }
        if (magic_list->find(pVideoNameSmk, file, offset, size)) {
            return file;
        }
    }

    return nullptr;
}

void MPlayer::Unload() {
    if (pMovie_Track) {
        pMovie_Track->Stop();
    }
    pMovie_Track = nullptr;
    if (!bGameoverLoop) {
        pAudioPlayer->MusicResume();
    }
    pEventTimer->Resume();
}

// for video//////////////////////////////////////////////////////////////////

void av_logger(void *ptr, int level, const char *format, va_list args) {
    char buf[2048];
    int prefix = 1;
    av_log_format_line(ptr, level, format, args, buf, 2048, &prefix);
    log("av: %s", buf);
}

MPlayer::MPlayer() {
    static int libavcodec_initialized = false;

    if (!libavcodec_initialized) {
        av_log_set_callback(av_logger);
        // Register all available file formats and codecs
#ifndef FF_API_NEXT
        avcodec_register_all();
        av_register_all();
#endif
        libavcodec_initialized = true;
    }

    pMovie_Track = nullptr;
    might_list = nullptr;
    magic_list = nullptr;

    if (!provider) {
        provider = new OpenALSoundProvider;
        // logger->Warning("allocation dynamic memory for provider\n");
        provider->Initialize();
    }
}

MPlayer::~MPlayer() {
    if (might_list != nullptr) {
        delete might_list;
        might_list = nullptr;
    }

    if (magic_list != nullptr) {
        delete magic_list;
        magic_list = nullptr;
    }

    delete provider;
}

// AudioBaseDataSource

class AudioBaseDataSource : public IAudioDataSource {
 public:
    AudioBaseDataSource();
    virtual ~AudioBaseDataSource() { Close(); }

    virtual bool Open();
    virtual void Close();

    virtual size_t GetSampleRate();
    virtual size_t GetChannelCount();
    virtual PMemBuffer GetNextBuffer();

 protected:
    AVFormatContext *pFormatContext;
    int iStreamIndex;
    AVCodecContext *pCodecContext;
    SwrContext *pConverter;
    bool bOpened;
    std::queue<PMemBuffer, std::deque<PMemBuffer>> queue;
};

AudioBaseDataSource::AudioBaseDataSource() {
    pFormatContext = nullptr;
    iStreamIndex = -1;
    pCodecContext = nullptr;
    pConverter = nullptr;
    bOpened = false;
}

bool AudioBaseDataSource::Open() {
    // Retrieve stream information
    if (avformat_find_stream_info(pFormatContext, nullptr) < 0) {
        Close();
        fprintf(stderr, "ffmpeg: Unable to find stream info\n");
        return false;
    }

    AVCodec *codec = nullptr;
    iStreamIndex = av_find_best_stream(pFormatContext, AVMEDIA_TYPE_AUDIO, -1,
                                       -1, &codec, 0);
    if (iStreamIndex < 0) {
        Close();
        fprintf(stderr, "ffmpeg: Unable to find audio stream\n");
        return false;
    }

    AVStream *stream = pFormatContext->streams[iStreamIndex];
    pCodecContext = avcodec_alloc_context3(codec);
    if (pCodecContext == nullptr) {
        Close();
        return false;
    }

    if (avcodec_parameters_to_context(pCodecContext, stream->codecpar) < 0) {
        Close();
        return false;
    }
    if (avcodec_open2(pCodecContext, codec, nullptr) < 0) {
        Close();
        return false;
    }

    pConverter = swr_alloc_set_opts(
        pConverter,
        pCodecContext->channel_layout ? pCodecContext->channel_layout
                                      : AV_CH_LAYOUT_MONO,
        AV_SAMPLE_FMT_S16, pCodecContext->sample_rate,
        pCodecContext->channel_layout ? pCodecContext->channel_layout
                                      : AV_CH_LAYOUT_MONO,
        pCodecContext->sample_fmt, pCodecContext->sample_rate, 0, nullptr);
    if (swr_init(pConverter) < 0) {
        Close();
        fprintf(stderr, "ffmpeg: Failed to create converter\n");
        return false;
    }

    bOpened = true;

    return true;
}

void AudioBaseDataSource::Close() {
    if (pConverter != nullptr) {
        swr_free(&pConverter);
        pConverter = nullptr;
    }

    if (pCodecContext) {
        avcodec_close(pCodecContext);
        pCodecContext = nullptr;
    }

    iStreamIndex = -1;

    if (pFormatContext != nullptr) {
        avformat_close_input(&pFormatContext);
        pFormatContext = nullptr;
    }
}

size_t AudioBaseDataSource::GetSampleRate() {
    if (pCodecContext == nullptr) {
        return 0;
    }

    return pCodecContext->sample_rate;
}

size_t AudioBaseDataSource::GetChannelCount() {
    if (pCodecContext == nullptr) {
        return 0;
    }

    return pCodecContext->channels;
}

PMemBuffer AudioBaseDataSource::GetNextBuffer() {
    PMemBuffer buffer;

    if (!queue.empty()) {
        buffer = queue.front();
        queue.pop();
    }

    AVPacket *packet = av_packet_alloc();

    if (av_read_frame(pFormatContext, packet) >= 0) {
        if (avcodec_send_packet(pCodecContext, packet) >= 0) {
            PMemBuffer result;
            AVFrame *frame = av_frame_alloc();
            int res = 0;
            while (res >= 0) {
                res = avcodec_receive_frame(pCodecContext, frame);
                if (res == AVERROR(EAGAIN) || res == AVERROR_EOF) {
                    break;
                }
                if (res < 0) {
                    return buffer;
                }
                PMemBuffer tmp_buf = AllocMemBuffer(
                    frame->nb_samples * pCodecContext->channels * 2);
                uint8_t *dst_channels[8] = {0};
                dst_channels[0] = (uint8_t *)tmp_buf->GetData();
                int got_samples = swr_convert(
                    pConverter, dst_channels, frame->nb_samples,
                    (const uint8_t **)frame->data, frame->nb_samples);
                if (got_samples > 0) {
                    if (!buffer) {
                        buffer = tmp_buf;
                    } else {
                        queue.push(tmp_buf);
                    }
                }
            }
            av_frame_free(&frame);
        }
    }

    av_packet_free(&packet);

    return buffer;
}

// AudioFileDataSource

class AudioFileDataSource : public AudioBaseDataSource {
 public:
    explicit AudioFileDataSource(const std::string &file_name);
    virtual ~AudioFileDataSource() {}

    virtual bool Open();

 protected:
    std::string sFileName;
};

AudioFileDataSource::AudioFileDataSource(const std::string &file_name) {
    sFileName = file_name;
}

bool AudioFileDataSource::Open() {
    if (bOpened) {
        return true;
    }

    // Open audio file
    if (avformat_open_input(&pFormatContext, sFileName.c_str(), nullptr,
                            nullptr) < 0) {
        fprintf(stderr, "ffmpeg: Unable to open input file\n");
        return false;
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatContext, 0, sFileName.c_str(), 0);

    return AudioBaseDataSource::Open();
}

// AudioBufferDataSource

class AudioBufferDataSource : public AudioBaseDataSource {
 public:
    explicit AudioBufferDataSource(PMemBuffer buffer);
    virtual ~AudioBufferDataSource() {}

    bool Open();

 protected:
    PMemBuffer buffer;
    uint8_t *buf_pos;
    uint8_t *buf_end;
    uint8_t *avio_ctx_buffer;
    size_t avio_ctx_buffer_size;
    AVIOContext *avio_ctx;

    static int read_packet(void *opaque, uint8_t *buf, int buf_size);
    int ReadPacket(uint8_t *buf, int buf_size);
};

AudioBufferDataSource::AudioBufferDataSource(PMemBuffer buffer)
    : buffer(buffer) {
    buf_pos = nullptr;
    buf_end = nullptr;
    avio_ctx_buffer = nullptr;
    avio_ctx_buffer_size = 4096;
    avio_ctx = nullptr;
}

bool AudioBufferDataSource::Open() {
    if (bOpened) {
        return true;
    }

    pFormatContext = avformat_alloc_context();
    if (pFormatContext == nullptr) {
        return false;
    }

    avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
    if (avio_ctx_buffer == nullptr) {
        Close();
        return false;
    }

    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0,
                                  this, &read_packet, nullptr, nullptr);
    if (!avio_ctx) {
        Close();
        return false;
    }

    pFormatContext->pb = avio_ctx;

    buf_pos = (uint8_t*)buffer->GetData();
    buf_end = buf_pos + buffer->GetSize();

    // Open audio file
    if (avformat_open_input(&pFormatContext, nullptr, nullptr, nullptr) < 0) {
        fprintf(stderr, "ffmpeg: Unable to open input buffer\n");
        return false;
    }

    // Dump information about buffer onto standard error
    av_dump_format(pFormatContext, 0, nullptr, 0);

    return AudioBaseDataSource::Open();
}

int AudioBufferDataSource::read_packet(void *opaque, uint8_t *buf,
                                       int buf_size) {
    AudioBufferDataSource *pThis = (AudioBufferDataSource *)opaque;
    return pThis->ReadPacket(buf, buf_size);
}

int AudioBufferDataSource::ReadPacket(uint8_t *buf, int buf_size) {
    int size = buf_end - buf_pos;
    if (size <= 0) {
        return 0;
    }
    size = std::min(buf_size, size);
    memcpy(buf, buf_pos, size);
    buf_pos += size;
    return size;
}

PAudioDataSource CreateAudioFileDataSource(const std::string &file_name) {
    std::shared_ptr<AudioFileDataSource> source =
        std::make_shared<AudioFileDataSource>(file_name);
    return std::dynamic_pointer_cast<IAudioDataSource, AudioFileDataSource>(
        source);
}

PAudioDataSource CreateAudioBufferDataSource(PMemBuffer buffer) {
    std::shared_ptr<AudioBufferDataSource> source =
        std::make_shared<AudioBufferDataSource>(buffer);
    return std::dynamic_pointer_cast<IAudioDataSource, AudioBufferDataSource>(
        source);
}
