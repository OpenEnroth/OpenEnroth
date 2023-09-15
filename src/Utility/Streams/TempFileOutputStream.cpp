#include "TempFileOutputStream.h"

#include <random>
#include <filesystem>

TempFileOutputStream::TempFileOutputStream(std::string_view path) {
    open(path);
}

TempFileOutputStream::~TempFileOutputStream() {
    closeInternal();
}

void TempFileOutputStream::open(std::string_view path) {
    close();

    _targetPath = path;
    do {
        _tmpPath = _targetPath + "." + std::to_string(std::random_device()()) + ".tmp";
    } while (std::filesystem::exists(_tmpPath));
    base_type::open(_tmpPath);
}

void TempFileOutputStream::close() {
    closeInternal();
}

void TempFileOutputStream::closeInternal() {
    if (!isOpen())
        return;

    base_type::close();
    std::filesystem::rename(_tmpPath, _targetPath);
}
