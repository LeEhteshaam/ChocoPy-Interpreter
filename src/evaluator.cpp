#include "parser.hpp"
#include <variant>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using Value = std::variant<std::string, int, bool, std::monostate>;

Value eval(struct expr expression) {
    // match base on variant type 
    std::visit(overloaded {
        [] (const literal& l) { return l.val; }, 
        [] (const unary& u) { return evalUnary(u); }, 
        [] (const grouping& g) { return eval(*g.expression); },
        [] (const binary& b) { return evalBinary(b); }
    }
    , expression.node)
};

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

    if (opType == NOT) {
        return (!isTruthy(right));
    } else {
        return std::visit(overloaded{
            [](bool b)             { return b * -1; },
            [](int n)           { return n * -1; },
            [](std::monostate)     { return std::monostate{}; }, // log error: invalid type for -
            [](const std::string& s) { return std::monostate{}; } // log error: invalid type for - 
        }, right);
    }
}

Value evalBinary(const binary& b) {
    TokenType opType = b.op.type;
    Value left_val = eval(*b.left);

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
            [](auto&& l, auto&& r) -> Value {
                // log error: invalid types for ADD
                return std::monostate{};
            }
        }, left_val, right_val);

    } else if (opType == MINUS) {
        return std::visit(overloaded{
            [](int l, int r) -> Value { return l - r; },
            [](auto&& l, auto&& r) -> Value {
                // log error: invalid types for MINUS
                return std::monostate{};
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
            [](auto&& l, auto&& r) -> Value {
                // log error: invalid types for MULTIPLY
                return std::monostate{};
            }
        }, left_val, right_val);

    } else if (opType == INT_DIVIDE) {
        return std::visit(overloaded{
            [](int l, int r) -> Value {
                if (r == 0) {
                    // log error: division by zero
                    return std::monostate{};
                }
                return l / r; 
            },
            [](auto&& l, auto&& r) -> Value {
                // log error: invalid types for INT_DIVIDE
                return std::monostate{};
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
            [](auto&& l, auto&& r) -> Value {
                // log error: invalid types for GREATER
                return std::monostate{};
            }
        }, left_val, right_val);

    } else if (opType == GREATER_EQUAL) {
        return std::visit(overloaded{
            [](int l, int r) -> Value { return l >= r; },
            [](const std::string& l, const std::string& r) -> Value { return l >= r; },
            [](auto&& l, auto&& r) -> Value {
                // log error: invalid types for GREATER_EQUAL
                return std::monostate{};
            }
        }, left_val, right_val);

    } else if (opType == LESS) {
        return std::visit(overloaded{
            [](int l, int r) -> Value { return l < r; },
            [](const std::string& l, const std::string& r) -> Value { return l < r; },
            [](auto&& l, auto&& r) -> Value {
                // log error: invalid types for LESS
                return std::monostate{};
            }
        }, left_val, right_val);

    } else if (opType == LESS_EQUAL) {
        return std::visit(overloaded{
            [](int l, int r) -> Value { return l <= r; },
            [](const std::string& l, const std::string& r) -> Value { return l <= r; },
            [](auto&& l, auto&& r) -> Value {
                // log error: invalid types for LESS_EQUAL
                return std::monostate{};
            }
        }, left_val, right_val);
    }

    // Fallback for an unknown operator, return a error 
    return std::monostate{};
}
