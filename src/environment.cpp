#include "environment.hpp"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

TokenType Environment::getTypeOfValue(const Value& val) {
    return std::visit(overloaded{
        [](int) -> TokenType { return INT_TYPE; },
        [](const std::string&) -> TokenType { return STR_TYPE; },
        [](bool) -> TokenType { return BOOL_TYPE; },
        [](std::monostate) -> TokenType { return NONE; }
    }, val);
}

void Environment::define(const Token& nameToken, TokenType declaredType, Value val) {
    std::string name = std::string(nameToken.lexeme);

    if (environment.contains(name)) {
        throw std::runtime_error(std::format("RuntimeError: Variable '{}' is already defined on line {}", name, nameToken.line));
    }

    if (declaredType != getTypeOfValue(val)) {
        throw std::runtime_error(std::format("RuntimeError: Type mismatch on line {}", nameToken.line));
    }

    environment[name] = val;
}

void Environment::assign(const Token& nameToken, Value val) {
    std::string name = std::string(nameToken.lexeme);

    if (!environment.contains(name)) {
        throw std::runtime_error(std::format("RuntimeError: Undefined variable '{}' on line {}", name, nameToken.line));
    }

    TokenType existingType = getTypeOfValue(environment[name]);
    TokenType newType = getTypeOfValue(val);

    if (existingType != newType) {
        throw std::runtime_error(std::format("RuntimeError: Type mismatch on reassignment to '{}' on line {}", name, nameToken.line));
    }

    environment[name] = val;
}

Value Environment::get(const Token& nameToken) {
    std::string name = std::string(nameToken.lexeme);

    if (environment.contains(name)) {
        return environment[name];
    }

    throw std::runtime_error(std::format("RuntimeError: Undefined variable '{}' on line {}", name, nameToken.line));
}