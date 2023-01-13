#include "DataPath.h"

#include <vector>
#include <set>

#include "FileSystem.h"

static std::filesystem::path s_data_path;
static std::vector<std::vector<std::string_view>> validateList = {
    {"anims", "magic7.vid" },
    {"anims", "might7.vid" },
    {"data", "bitmaps.lod" },
    {"data", "d3dbitmap.hwl" },
    {"data", "d3dsprite.hwl" },
    {"data", "events.lod" },
    {"data", "games.lod" },
    {"data", "icons.lod" },
    {"data", "sprites.lod" },
    {"sounds", "audio.snd" },
    {"shaders", "glbillbshader.frag"},
    {"shaders", "glbillbshader.vert"},
    {"shaders", "glbspshader.frag"},
    {"shaders", "glbspshader.vert"},
    {"shaders", "gldecalshader.frag"},
    {"shaders", "gldecalshader.vert"},
    {"shaders", "glforcepershader.frag"},
    {"shaders", "glforcepershader.vert"},
    {"shaders", "gllinesshader.frag"},
    {"shaders", "gllinesshader.vert"},
    {"shaders", "glnuklear.frag"},
    {"shaders", "glnuklear.vert"},
    {"shaders", "gloutbuild.frag"},
    {"shaders", "gloutbuild.vert"},
    {"shaders", "glterrain.frag"},
    {"shaders", "glterrain.vert"},
    {"shaders", "gltextshader.frag"},
    {"shaders", "gltextshader.vert"},
    {"shaders", "gltwodshader.frag"},
    {"shaders", "gltwodshader.vert"},
};

void SetDataPath(const std::string &data_path) {
    s_data_path = ExpandUserPath(data_path);
}

std::string MakeDataPath(std::initializer_list<std::string_view> paths) {
    std::filesystem::path result = s_data_path;

    for (auto p : paths)
        if (!p.empty())
            result /= p;

    return MakeCaseInsensitivePath(result).string();
}

bool validateDataPath(const std::string &data_path) {
    bool isGood = true;
    for (auto v : validateList) {
        std::filesystem::path path = data_path;
        for (auto p : v) {
            if (!p.empty())
                path /= p;
        }

        if (!std::filesystem::exists(MakeCaseInsensitivePath(path))) {
            isGood = false;
            break;
        }
    }

    return isGood;
}

std::string MakeTempPath(const char *file_rel_path) {
    return (std::filesystem::temp_directory_path() / file_rel_path).string();
}
