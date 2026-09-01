#include "parser.hpp"
#include <variant>
#include <stdexcept>
#include <format>
#include <string>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using Value = std::variant<std::string, int, bool, std::monostate>;

Value evalUnary(const unary& u);
Value evalBinary(const binary& b);

Value eval(struct expr expression) {
    // match base on variant type 
    return std::visit(overloaded {
        [] (const literal& l) { return l.val; }, 
        [] (const unary& u) { return evalUnary(u); }, 
        [] (const grouping& g) { return eval(*g.expression); },
        [] (const binary& b) { return evalBinary(b); }
    }, expression.node);
}

bool isTruthy(const Value& val) {
    return std::visit(overloaded{
        [](std::monostate)     { return false; },
        [](bool b)             { return b; },
        [](int n)           { return n != 0; },
        [](const std::string& s) { return !s.empty(); }
    }, val);
}

Value evalUnary(const unary& u) {
    Value right = eval(*u.right);
    TokenType opType = u.op.type;
    int line = u.op.line;

    if (opType == NOT) {
        return (!isTruthy(right));
    } else {
        return std::visit(overloaded{
            [](bool b) -> Value    { return b * -1; },
            [](int n) -> Value     { return n * -1; },
            [line](std::monostate) -> Value { throw std::runtime_error(std::format("RuntimeError: bad operand type for unary -: 'NoneType' on line {}", line)); }, 
            [line](const std::string& s) -> Value { throw std::runtime_error(std::format("RuntimeError: bad operand type for unary -: 'str' on line {}", line)); } 
        }, right);
    }
}

Value evalBinary(const binary& b) {
    TokenType opType = b.op.type;
    Value left_val = eval(*b.left);
    int line = b.op.line;

    if (opType == AND) {
        if (!isTruthy(left_val)) return left_val;
        return eval(*b.right);

    } else if (opType == OR) {
        if (isTruthy(left_val)) return left_val; 
        return eval(*b.right);
    }

    Value right_val = eval(*b.right);

    if (opType == ADD) {
        return std::visit(overloaded{
            [](int l, int r) -> Value { return l + r; },
            [](const std::string& l, const std::string& r) -> Value { return l + r; },
            [line](auto&& l, auto&& r) -> Value {
                throw std::runtime_error(std::format("RuntimeError: unsupported operand type(s) for + on line {}", line));
            }
        }, left_val, right_val);

    } else if (opType == MINUS) {
        return std::visit(overloaded{
            [](int l, int r) -> Value { return l - r; },
            [line](auto&& l, auto&& r) -> Value {
                throw std::runtime_error(std::format("RuntimeError: unsupported operand type(s) for - on line {}", line));
            }
        }, left_val, right_val);

    } else if (opType == MULTIPLY) {
        return std::visit(overloaded{
            [](int l, int r) -> Value { return l * r; },
            [](const std::string& l, int r) -> Value {
                std::string res;
                for (int i = 0; i < r; ++i) res += l;
                return res;
            },
            [line](auto&& l, auto&& r) -> Value {
                throw std::runtime_error(std::format("RuntimeError: unsupported operand type(s) for * on line {}", line));
            }
        }, left_val, right_val);

    } else if (opType == INT_DIVIDE) {
        return std::visit(overloaded{
            [line](int l, int r) -> Value {
                if (r == 0) {
                    throw std::runtime_error(std::format("RuntimeError: division by zero on line {}", line));
                }
                return l / r; 
            },
            [line](auto&& l, auto&& r) -> Value {
                throw std::runtime_error(std::format("RuntimeError: unsupported operand type(s) for // on line {}", line));
            }
        }, left_val, right_val);

    } else if (opType == EQUAL) {
        return left_val == right_val;

    } else if (opType == NOT_EQUAL) {
        return left_val != right_val;

    } else if (opType == GREATER) {
        return std::visit(overloaded{
            [](int l, int r) -> Value { return l > r; },
            [](const std::string& l, const std::string& r) -> Value { return l > r; },
            [line](auto&& l, auto&& r) -> Value {
                throw std::runtime_error(std::format("RuntimeError: unsupported operand type(s) for > on line {}", line));
            }
        }, left_val, right_val);

    } else if (opType == GREATER_EQUAL) {
        return std::visit(overloaded{
            [](int l, int r) -> Value { return l >= r; },
            [](const std::string& l, const std::string& r) -> Value { return l >= r; },
            [line](auto&& l, auto&& r) -> Value {
                throw std::runtime_error(std::format("RuntimeError: unsupported operand type(s) for >= on line {}", line));
            }
        }, left_val, right_val);

    } else if (opType == LESS) {
        return std::visit(overloaded{
            [](int l, int r) -> Value { return l < r; },
            [](const std::string& l, const std::string& r) -> Value { return l < r; },
            [line](auto&& l, auto&& r) -> Value {
                throw std::runtime_error(std::format("RuntimeError: unsupported operand type(s) for < on line {}", line));
            }
        }, left_val, right_val);

    } else if (opType == LESS_EQUAL) {
        return std::visit(overloaded{
            [](int l, int r) -> Value { return l <= r; },
            [](const std::string& l, const std::string& r) -> Value { return l <= r; },
            [line](auto&& l, auto&& r) -> Value {
                throw std::runtime_error(std::format("RuntimeError: unsupported operand type(s) for <= on line {}", line));
            }
        }, left_val, right_val);
    }

    throw std::runtime_error(std::format("RuntimeError: Invalid op type on line {}", line));
}