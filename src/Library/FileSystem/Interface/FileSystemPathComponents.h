#pragma once

#include <cassert>
#include <string_view>

class FileSystemPathView;

class FileSystemPathComponents {
 public:
    inline FileSystemPathView prefix() const;

    std::string_view name() const {
        return _path.substr(_nameStart);
    }

    std::string_view extension() const {
        return _path.substr(_extStart);
    }

    std::string_view stem() const {
        return _path.substr(_nameStart, _extStart - _nameStart);
    }

 private:
    explicit FileSystemPathComponents(std::string_view path) : _path(path) {
        _prefixEnd = _path.find_last_of('/');
        if (_prefixEnd == std::string_view::npos) {
            _prefixEnd = 0;
            _nameStart = 0;
        } else {
            _nameStart = _prefixEnd + 1;
        }

        const char *nameStart = _path.data() + _nameStart;
        const char *nameEnd = _path.data() + _path.size();
        if (nameEnd == nameStart) {
            _extStart = _path.size();
        } else {
            assert(!(nameEnd - nameStart == 1 && *nameStart == '.')); // Should be normalized, no "."s.
            _extStart = std::string_view(nameStart + 1, nameEnd).find_last_of('.');
            if (_extStart == 0 && nameEnd - nameStart == 2 && *nameStart == '.') { // Check for "..".
                _extStart = _path.size();
            } else {
                _extStart = _extStart == std::string_view::npos ? _path.size() : _extStart + _nameStart + 1;
            }
        }
    }

    friend class FileSystemPathView;

 private:
    std::string_view _path;
    size_t _prefixEnd;
    size_t _nameStart;
    size_t _extStart;
};
