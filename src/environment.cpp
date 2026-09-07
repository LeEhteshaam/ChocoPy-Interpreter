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

Environment* Environment::ancestor(int distance) {
    Environment* environment = this;
    for (int i = 0; i < distance; ++i) {
        if (environment->parent == nullptr) {
            return nullptr;
        }
        environment = environment->parent.get();
    }
    return environment;
}

void Environment::define(const Token& nameToken, TokenType declaredType, Value val) {
    std::string name = std::string(nameToken.lexeme);

    if (varMap.contains(name)) {
        throw std::runtime_error(std::format("RuntimeError: Variable '{}' is already defined on line {}", name, nameToken.line));
    }
    
    if (declaredType != getTypeOfValue(val)) {
        throw std::runtime_error(std::format("RuntimeError: Type mismatch on line {}", nameToken.line));
    }

    varMap[name] = val;
}

void Environment::assign(const Token& nameToken, Value val, int distance) {
    Environment* target = ancestor(distance);
    if (target == nullptr) {
        throw std::runtime_error(std::format("RuntimeError: Undefined variable '{}' on line {}", nameToken.lexeme, nameToken.line));
    }

    std::string name = std::string(nameToken.lexeme);

    if (!target->varMap.contains(name)) {
        throw std::runtime_error(std::format("RuntimeError: Undefined variable '{}' on line {}", name, nameToken.line));
    }

    TokenType existingType = getTypeOfValue(target->varMap[name]);
    TokenType newType = getTypeOfValue(val);

    if (existingType != newType) {
        throw std::runtime_error(std::format("RuntimeError: Type mismatch on reassignment to '{}' on line {}", name, nameToken.line));
    }

    target->varMap[name] = val;
}

Value Environment::get(const Token& nameToken, int distance) {
    Environment* target = ancestor(distance);
    if (target == nullptr) {
        throw std::runtime_error(std::format("RuntimeError: Undefined variable '{}' on line {}", nameToken.lexeme, nameToken.line));
    }

    std::string name = std::string(nameToken.lexeme);

    if (target->varMap.contains(name)) {
        return target->varMap[name];
    }

    throw std::runtime_error(std::format("RuntimeError: Undefined variable '{}' on line {}", name, nameToken.line));
}

struct closure Environment::getFunc(const Token& nameToken, int distance) {
    Environment* target = ancestor(distance);
    if (target == nullptr) {
        throw std::runtime_error(std::format("RuntimeError: Undefined variable '{}' on line {}", nameToken.lexeme, nameToken.line));
    }

    std::string name = std::string(nameToken.lexeme);

    if (target->closures.contains(name)) {
        return target->closures[name];
    }

    throw std::runtime_error(std::format("RuntimeError: Undefined variable '{}' on line {}", name, nameToken.line));
}

void Environment::addFunc(const Token& nameToken, funcDef func, std::shared_ptr<Environment> env) {
    std::string name = std::string(nameToken.lexeme);

    if (closures.contains(name)) {
        throw std::runtime_error(std::format("RuntimeError: Function '{}' is already defined on line {}", name, nameToken.line));
    }

    closures[name] = closure { 
        std::move(func),
        env
    };
}