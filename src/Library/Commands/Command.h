#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <utility>

template<typename T>
class ParseCommandParameter {
 public:
    static bool parse(const std::string &str, T &value) {
        std::istringstream iss(str);
        iss >> value;
        return true;
    }
};

class ICommand {
 public:
    virtual ~ICommand() = default;
    virtual std::string run(const std::vector<std::string> &parameters) = 0;
};

template<typename ...Properties>
class TCommandN : public ICommand {
    std::string run(const std::vector<std::string> &parameters) override {
        constexpr auto NUM_ARGS{ sizeof...(Properties) };
        if (parameters.size() >= NUM_ARGS) {
            return unpackAndCallFunction<NUM_ARGS, 0>(parameters, Properties()...);
        }

        return "";
    }

    template<int SIZE, int INDEX, typename T, typename ...Args>
    std::string unpackAndCallFunction(const std::vector<std::string> &vec, T first, Args &&... args) {
        typename std::decay<T>::type value{};
        ParseCommandParameter<T>::parse(vec[INDEX], value);

        if constexpr (INDEX < SIZE - 1) {
            return unpackAndCallFunction<SIZE, INDEX + 1>(vec, args..., value);
        } else {
            return run(args..., value);
        }
    }

    template<int SIZE, int INDEX>
    std::string unpackAndCallFunction(const std::vector<std::string>& vec) {
        return run();
    }

    virtual std::string run(Properties...) = 0;
};

template<typename Func, typename ...Properties>
class TCommandFunc : public TCommandN<Properties...> {
 public:
    explicit TCommandFunc(Func &&func) : _func(std::forward<Func>(func)) {
    }

    std::string run(Properties... props) override {
        return _func(props...);
    }

 private:
    Func _func;
};

class ChangeMoneyCommand : public TCommandN<int, float, std::string> {
 public:
    std::string run(int money, float f, std::string t) override;
};
