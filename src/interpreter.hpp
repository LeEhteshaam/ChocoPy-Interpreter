#pragma once

#include "ast.hpp"
#include "environment.hpp"
#include <variant>
#include <string>
#include <vector>
#include <iostream>
#include <ostream>

using Value = std::variant<std::string, int, bool, std::monostate>;

class ReturnException {
public:
    Value returnValue;    
    ReturnException(Value val) : returnValue(std::move(val)) {}
};

class Interpreter {
private:
    std::shared_ptr<Environment> env = std::make_shared<Environment>(); 
    std::string stringify(const Value& val);
    Value eval(const expr& expression, std::ostream& out = std::cout, std::ostream& err = std::cerr);
    Value evalUnary(const unary& u, std::ostream& out = std::cout, std::ostream& err = std::cerr);
    Value evalBinary(const binary& b, std::ostream& out = std::cout, std::ostream& err = std::cerr);
    bool isTruthy(const Value& val);

public:
    void interpret(const std::vector<stmt>& statements, std::ostream& out = std::cout, std::ostream& err = std::cerr);
};