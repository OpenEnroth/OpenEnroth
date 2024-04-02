#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <tuple>

typedef std::tuple<std::string, bool> ExecuteResult;

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
    virtual ExecuteResult run(const std::vector<std::string> &parameters) = 0;
};

template<typename ...Properties>
class TCommandN : public ICommand {
    ExecuteResult run(const std::vector<std::string> &parameters) override {
        constexpr auto NUM_ARGS{ sizeof...(Properties) };
        if (parameters.size() >= NUM_ARGS) {
            return unpackAndCallFunction<NUM_ARGS, 0>(parameters, Properties()...);
        } else {
            return { "Invalid arguments number. Expected: " + std::to_string(NUM_ARGS) + " - Provided: " + std::to_string(parameters.size()), false };
        }
    }

    template<int SIZE, int INDEX, typename T, typename ...Args>
    ExecuteResult unpackAndCallFunction(const std::vector<std::string> &vec, T first, Args &&... args) {
        typename std::decay<T>::type value{};
        if (INDEX < vec.size()) {
            ParseCommandParameter<T>::parse(vec[INDEX], value);
        }

        if constexpr (INDEX < SIZE - 1) {
            return unpackAndCallFunction<SIZE, INDEX + 1>(vec, args..., value);
        } else {
            return run(args..., value);
        }
    }

    template<int SIZE, int INDEX>
    ExecuteResult unpackAndCallFunction(const std::vector<std::string>& vec) {
        return run();
    }

    virtual ExecuteResult run(Properties...) = 0;
};

template<typename Func, typename ...Properties>
class TCommandFunc : public TCommandN<Properties...> {
 public:
    explicit TCommandFunc(Func &&func) : _func(std::forward<Func>(func)) {
    }

    ExecuteResult run(Properties... props) override {
        return _func(props...);
    }

 private:
    Func _func;
};

class ChangeMoneyCommand : public TCommandN<int, float, std::string> {
 public:
     ExecuteResult run(int money, float f, std::string t) override;
};
