#include "ast_printer.hpp"
#include <print>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void printAST(const expr& expression, std::ostream& out) {
    std::visit(overloaded {

        [&out] (const literal& l) {
            std::visit(overloaded {
                [&out] (int num) { std::print(out, "{}", num); },
                [&out] (bool b) { std::print(out, "{}", b ? "True" : "False"); },
                [&out] (std::string_view s) { std::print(out, "{}", s); }, 
                [&out] (std::monostate) { std::print(out, "null"); }
            }, l.val);
        },

        [&out] (const varExpr& v) {
            std::print(out, "{}", v.name.lexeme);
        },

        [&out] (const callExpr& c) {
            std::print(out, "{}(", c.name.lexeme);
            for (size_t i = 0; i < c.arguments.size(); ++i) {
                if (i > 0) std::print(out, ", ");
                printAST(c.arguments[i], out);
            }
            std::print(out, ")");
        },

        [&out] (const binary& b) {
            printAST(*b.left, out);
            std::string_view op = b.op.lexeme;
            std::print(out, " {} ", op);
            printAST(*b.right, out);
        },

        [&out] (const unary& u) {
            std::string_view lexeme = u.op.lexeme;
            std::print(out, "{}", lexeme);
            printAST(*u.right, out);
        },

        [&out] (const grouping& g) {
            std::print(out, "(");
            printAST(*g.expression, out);
            std::print(out, ")");
        }
    }
    , expression.node);
}