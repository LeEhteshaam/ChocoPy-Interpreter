#include "interpreter.hpp"
#include <variant>
#include <stdexcept>
#include <format>
#include <string>
#include <print>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void Interpreter::interpret(const std::vector<expr>& expressions, std::ostream& out, std::ostream& err) {
    try {
        for (const auto& expression : expressions) {
            Value res = eval(expression);
            out << stringify(res) << "\n";
        }
    } catch (const std::runtime_error& error) {
        err << error.what() << "\n";
    }
}

std::string Interpreter::stringify(const Value& val) {
    return std::visit(overloaded{
        [](std::monostate) -> std::string { return "None"; },
        [](bool b) -> std::string { return b ? "True" : "False"; },
        [](int n) -> std::string { return std::to_string(n); },
        [](const std::string& s) -> std::string { return s; }
    }, val);
}

Value Interpreter::eval(const expr& expression) {
    // match based on variant type 
    return std::visit(overloaded {
        [] (const literal& l) -> Value {
            return std::visit(overloaded{
                [](int val) -> Value { return val; },
                [](bool val) -> Value { return val; },
                [](std::string_view val) -> Value { return std::string(val); },
                [](std::monostate val) -> Value { return val; }
            }, l.val);
        }, 
        [this] (const unary& u) -> Value { return evalUnary(u); }, 
        [this] (const grouping& g) -> Value { return eval(*g.expression); },
        [this] (const binary& b) -> Value { return evalBinary(b); }
    }, expression.node);
}

bool Interpreter::isTruthy(const Value& val) {
    return std::visit(overloaded{
        [](std::monostate)       { return false; },
        [](bool b)               { return b; },
        [](int n)                { return n != 0; },
        [](const std::string& s) { return !s.empty(); }
    }, val);
}

Value Interpreter::evalUnary(const unary& u) {
    const Value right = eval(*u.right);
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

Value Interpreter::evalBinary(const binary& b) {
    TokenType opType = b.op.type;
    const Value left_val = eval(*b.left);
    int line = b.op.line;

    if (opType == AND) {
        if (!isTruthy(left_val)) return left_val;
        return eval(*b.right);

    } else if (opType == OR) {
        if (isTruthy(left_val)) return left_val; 
        return eval(*b.right);
    }

    const Value right_val = eval(*b.right);

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
