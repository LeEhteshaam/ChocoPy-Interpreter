#pragma once
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <format>
#include <variant>
#include "lexer.hpp"

using Value = std::variant<std::string, int, bool, std::monostate>;

class Environment {
    private:
        std::shared_ptr<Environment> parent;
        std::unordered_map<std::string, Value> environment; 
        TokenType getTypeOfValue(const Value& val);

    public: 
        void define(const Token& nameToken, TokenType typeKw, Value val);
        void assign(const Token& nameToken, Value val);
        Value get(const Token& token);
};