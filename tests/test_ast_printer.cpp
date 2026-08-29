#include "../src/ast_printer.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <sstream>

// Helper functions to construct AST nodes
expr make_literal_int(int val) {
    return expr{literal{Token{INT, 0, 0, "", 0, val}, val}};
}

expr make_literal_bool(bool val) {
    return expr{literal{Token{val ? TRUE : FALSE, 0, 0, "", 0, std::monostate{}}, val}};
}

expr make_literal_str(std::string_view val) {
    return expr{literal{Token{STR, 0, 0, "", 0, val}, val}};
}

expr make_literal_none() {
    return expr{literal{Token{NONE, 0, 0, "", 0, std::monostate{}}, std::monostate{}}};
}

expr make_unary(Token op, expr right) {
    return expr{unary{op, std::make_unique<expr>(std::move(right))}};
}

expr make_binary(expr left, Token op, expr right) {
    return expr{binary{
        std::make_unique<expr>(std::move(left)),
        op,
        std::make_unique<expr>(std::move(right))
    }};
}

expr make_grouping(expr expression) {
    return expr{grouping{std::make_unique<expr>(std::move(expression))}};
}

// Helper to capture output of printAST
std::string capture_ast_output(const expr& expression) {
    std::ostringstream oss;
    printAST(expression, oss);
    return oss.str();
}

// Unit tests
void test_literal_exprs() {
    // Int literal
    {
        expr e = make_literal_int(42);
        std::string out = capture_ast_output(e);
        assert(out == "42");
    }

    // Bool literal - True
    {
        expr e = make_literal_bool(true);
        std::string out = capture_ast_output(e);
        assert(out == "True");
    }

    // Bool literal - False
    {
        expr e = make_literal_bool(false);
        std::string out = capture_ast_output(e);
        assert(out == "False");
    }

    // String literal
    {
        expr e = make_literal_str("\"hello\"");
        std::string out = capture_ast_output(e);
        assert(out == "\"hello\"");
    }

    // None (monostate) literal
    {
        expr e = make_literal_none();
        std::string out = capture_ast_output(e);
        assert(out == "null");
    }

    std::cout << "  test_literal_exprs passed!\n";
}

void test_unary_exprs() {
    // -42
    {
        Token op{MINUS, 1, 1, "-", 1, std::monostate{}};
        expr e = make_unary(op, make_literal_int(42));
        std::string out = capture_ast_output(e);
        assert(out == "-42");
    }

    // not True
    {
        Token op{NOT, 1, 1, "not ", 4, std::monostate{}};
        expr e = make_unary(op, make_literal_bool(true));
        std::string out = capture_ast_output(e);
        assert(out == "not True");
    }

    std::cout << "  test_unary_exprs passed!\n";
}

void test_binary_exprs() {
    // 5 + 10
    {
        Token op{ADD, 1, 3, "+", 1, std::monostate{}};
        expr e = make_binary(make_literal_int(5), op, make_literal_int(10));
        std::string out = capture_ast_output(e);
        assert(out == "5 + 10");
    }

    // x == None
    {
        Token op{EQUAL, 1, 3, "==", 2, std::monostate{}};
        expr var{literal{Token{IDENTIFIER, 1, 1, "x", 1, "x"}, "x"}};
        expr e = make_binary(std::move(var), op, make_literal_none());
        std::string out = capture_ast_output(e);
        assert(out == "x == null");
    }

    std::cout << "  test_binary_exprs passed!\n";
}

void test_grouping_exprs() {
    // (42)
    {
        expr e = make_grouping(make_literal_int(42));
        std::string out = capture_ast_output(e);
        assert(out == "(42)");
    }

    std::cout << "  test_grouping_exprs passed!\n";
}

void test_complex_exprs() {
    // (5 + 10) * -2
    {
        Token op_plus{ADD, 1, 3, "+", 1, std::monostate{}};
        expr inner = make_binary(make_literal_int(5), op_plus, make_literal_int(10));
        expr group = make_grouping(std::move(inner));

        Token op_minus{MINUS, 1, 1, "-", 1, std::monostate{}};
        expr right = make_unary(op_minus, make_literal_int(2));

        Token op_mul{MULTIPLY, 1, 9, "*", 1, std::monostate{}};
        expr complex = make_binary(std::move(group), op_mul, std::move(right));

        std::string out = capture_ast_output(complex);
        assert(out == "(5 + 10) * -2");
    }

    std::cout << "  test_complex_exprs passed!\n";
}

void run_ast_printer_tests() {
    std::cout << "Running AST Printer unit tests...\n";
    test_literal_exprs();
    test_unary_exprs();
    test_binary_exprs();
    test_grouping_exprs();
    test_complex_exprs();
}
