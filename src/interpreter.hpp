#pragma once

#include "ast.hpp"
#include "environment.hpp"
#include <variant>
#include <string>
#include <vector>
#include <iostream>
#include <ostream>

using Value = std::variant<std::string, int, bool, std::monostate>;

class Interpreter {
private:
    Environment env;
    std::string stringify(const Value& val);
    Value eval(const expr& expression);
    Value evalUnary(const unary& u);
    Value evalBinary(const binary& b);
    bool isTruthy(const Value& val);

public:
    void interpret(const std::vector<stmt>& statements, std::ostream& out = std::cout, std::ostream& err = std::cerr);
};