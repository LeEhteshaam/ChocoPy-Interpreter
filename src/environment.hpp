#pragma once
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <format>
#include <variant>
#include "lexer.hpp"
#include "ast.hpp"

using Value = std::variant<std::string, int, bool, std::monostate>;

class Environment;

struct closure {
    funcDef func;
    std::shared_ptr<Environment> closureEnv;
};

class Environment {
    private:
        std::shared_ptr<Environment> parent;
        std::unordered_map<std::string, Value> varMap; 
        std::unordered_map<std::string, closure> closures;
        TokenType getTypeOfValue(const Value& val);

    public: 
        void define(const Token& nameToken, TokenType typeKw, Value val);
        void assign(const Token& nameToken, Value val, int distance = 0);
        Value get(const Token& token, int distance = 0);
        struct closure getFunc(const Token& nameToken, int distance = 0);
        void addFunc(const Token& nameToken, funcDef func, std::shared_ptr<Environment> env);
        Environment* ancestor(int distance);
        Environment() = default;         
        Environment(std::shared_ptr<Environment> par) {
            parent = par;
        }
};