#pragma once

#include "ast.hpp"
#include <variant>
#include <string>
#include <vector>

using Value = std::variant<std::string, int, bool, std::monostate>;

class Interpreter {
private:
    std::string stringify(const Value& val);
    Value eval(const expr& expression);
    Value evalUnary(const unary& u);
    Value evalBinary(const binary& b);
    bool isTruthy(const Value& val);

public:
    void interpret(const std::vector<expr>& expressions);
};
