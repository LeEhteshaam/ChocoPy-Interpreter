#include "ast_printer.hpp"
#include <print>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void printAST(const expr& expression) {
    std::visit(overloaded {

        [] (const literal& l) {
            std::visit(overloaded {
                [] (int num) {std::print("{}", num);},
                [] (bool b) {std::print("{}", b);},
                [] (std::string_view s) {std::print("{}", s);}, 
                [] (std::monostate m) {std::print("null");}
            }, l.val);
        },

        [] (const binary& b) {
            printAST(*b.left);
            std::string_view op = b.op.lexeme;
            std::print(" {} ", op);
            printAST(*b.right);
        },

        [] (const unary& u) {
            std::string_view lexeme = u.op.lexeme;
            std::print("{}", lexeme);
            printAST(*u.right);
        },

        [] (const grouping& g) {
            std::print("(");
            printAST(*g.expression);
            std::print(")");
        }
    }
    , expression.node);
}