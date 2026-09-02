#include "../src/interpreter.hpp"
#include "../src/environment.hpp"
#include "../src/parser.hpp"
#include "../src/ast.hpp"
#include "../src/lexer.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <variant>

// --- Statement and AST Node Construction Helpers ---

static stmt make_print_stmt(expr expression) {
    return stmt{printStmt{std::make_unique<expr>(std::move(expression))}};
}

static stmt make_expr_stmt(expr expression) {
    return stmt{exprStmt{std::make_unique<expr>(std::move(expression))}};
}

static stmt make_var_decl(TokenType type, Token identifier, expr value) {
    return stmt{varDecl{type, identifier, std::make_unique<expr>(std::move(value))}};
}

static stmt make_assign(Token name, expr value) {
    return stmt{assignStmt{name, std::make_unique<expr>(std::move(value))}};
}

// --- Stream Output Capture Helpers ---

struct CapturedOutput {
    std::string out;
    std::string err;
};

static CapturedOutput capture_interpret(const std::vector<stmt>& statements) {
    std::ostringstream out;
    std::ostringstream err;
    Interpreter interp;
    interp.interpret(statements, out, err);
    return {out.str(), err.str()};
}

static CapturedOutput capture_interpret(std::vector<expr>& expressions) {
    std::vector<stmt> stmts;
    for (auto& e : expressions) {
        stmts.push_back(make_print_stmt(std::move(e)));
    }
    return capture_interpret(stmts);
}

static CapturedOutput capture_interpret(expr expression) {
    std::vector<stmt> stmts;
    stmts.push_back(make_print_stmt(std::move(expression)));
    return capture_interpret(stmts);
}

// --- AST Node Construction Helpers ---

static expr make_literal_int(int val, int line = 1, int col = 1) {
    return expr{literal{Token{INT, line, col, "", 0, val}, val}};
}

static expr make_literal_bool(bool val, int line = 1, int col = 1) {
    return expr{literal{Token{val ? TRUE : FALSE, line, col, "", 0, std::monostate{}}, val}};
}

static expr make_literal_str(std::string_view val, int line = 1, int col = 1) {
    return expr{literal{Token{STR, line, col, val, val.length(), val}, val}};
}

static expr make_literal_none(int line = 1, int col = 1) {
    return expr{literal{Token{NONE, line, col, "None", 4, std::monostate{}}, std::monostate{}}};
}

static expr make_unary(Token op, expr right) {
    return expr{unary{op, std::make_unique<expr>(std::move(right))}};
}

static expr make_binary(expr left, Token op, expr right) {
    return expr{binary{
        std::make_unique<expr>(std::move(left)),
        op,
        std::make_unique<expr>(std::move(right))
    }};
}

static expr make_grouping(expr expression) {
    return expr{grouping{std::make_unique<expr>(std::move(expression))}};
}

static expr make_var_expr(std::string_view name, int line = 1, int col = 1) {
    return expr{varExpr{Token{IDENTIFIER, line, col, name, name.length(), std::monostate{}}}};
}

static Token make_op(TokenType type, std::string_view lexeme = "", int line = 1, int col = 1) {
    return Token{type, line, col, lexeme, lexeme.length(), std::monostate{}};
}

static Token make_token(TokenType type, std::string_view lexeme = "", int line = 1, int col = 1, std::variant<int, std::string_view, std::monostate> literal = std::monostate{}) {
    return Token{type, line, col, lexeme, lexeme.length(), literal};
}

// --- Primary Literals Unit Tests ---

void test_interpreter_literals() {
    // Integer literals
    {
        auto res = capture_interpret(make_literal_int(42));
        assert(res.out == "42\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_literal_int(-100));
        assert(res.out == "-100\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_literal_int(0));
        assert(res.out == "0\n");
        assert(res.err.empty());
    }

    // Boolean literals
    {
        auto res = capture_interpret(make_literal_bool(true));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_literal_bool(false));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // String literals
    {
        auto res = capture_interpret(make_literal_str("hello world"));
        assert(res.out == "hello world\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_literal_str(""));
        assert(res.out == "\n");
        assert(res.err.empty());
    }

    // None literal
    {
        auto res = capture_interpret(make_literal_none());
        assert(res.out == "None\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_literals passed!\n";
}

// --- Unary Operators Unit Tests ---

void test_interpreter_unary_not() {
    // not on booleans
    {
        auto res = capture_interpret(make_unary(make_op(NOT), make_literal_bool(true)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_unary(make_op(NOT), make_literal_bool(false)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }

    // not on integers (0 is falsy, non-zero is truthy)
    {
        auto res = capture_interpret(make_unary(make_op(NOT), make_literal_int(0)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_unary(make_op(NOT), make_literal_int(42)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_unary(make_op(NOT), make_literal_int(-5)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // not on strings ("" is falsy, non-empty is truthy)
    {
        auto res = capture_interpret(make_unary(make_op(NOT), make_literal_str("")));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_unary(make_op(NOT), make_literal_str("hello")));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // not on None (None is falsy)
    {
        auto res = capture_interpret(make_unary(make_op(NOT), make_literal_none()));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }

    // Chained not
    {
        expr inner = make_unary(make_op(NOT), make_literal_bool(true));
        expr outer = make_unary(make_op(NOT), std::move(inner));
        auto res = capture_interpret(std::move(outer));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        expr inner = make_unary(make_op(NOT), make_literal_int(0));
        expr outer = make_unary(make_op(NOT), std::move(inner));
        auto res = capture_interpret(std::move(outer));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_unary_not passed!\n";
}

void test_interpreter_unary_minus() {
    // Negating integer
    {
        auto res = capture_interpret(make_unary(make_op(MINUS), make_literal_int(42)));
        assert(res.out == "-42\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_unary(make_op(MINUS), make_literal_int(-15)));
        assert(res.out == "15\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_unary(make_op(MINUS), make_literal_int(0)));
        assert(res.out == "0\n");
        assert(res.err.empty());
    }

    // Negating boolean (bool * -1)
    {
        auto res = capture_interpret(make_unary(make_op(MINUS), make_literal_bool(true)));
        assert(res.out == "-1\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_unary(make_op(MINUS), make_literal_bool(false)));
        assert(res.out == "0\n");
        assert(res.err.empty());
    }

    // Chained unary minus
    {
        expr inner = make_unary(make_op(MINUS), make_literal_int(100));
        expr outer = make_unary(make_op(MINUS), std::move(inner));
        auto res = capture_interpret(std::move(outer));
        assert(res.out == "100\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_unary_minus passed!\n";
}

void test_interpreter_unary_minus_errors() {
    // -None
    {
        auto res = capture_interpret(make_unary(make_op(MINUS, "-", 12), make_literal_none()));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: bad operand type for unary -: 'NoneType' on line 12\n");
    }

    // -str (non-empty)
    {
        auto res = capture_interpret(make_unary(make_op(MINUS, "-", 34), make_literal_str("hello")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: bad operand type for unary -: 'str' on line 34\n");
    }

    // -str (empty)
    {
        auto res = capture_interpret(make_unary(make_op(MINUS, "-", 55), make_literal_str("")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: bad operand type for unary -: 'str' on line 55\n");
    }

    std::cout << "  test_interpreter_unary_minus_errors passed!\n";
}

// --- Binary Logical AND & OR Unit Tests ---

void test_interpreter_binary_and() {
    // Falsy left short-circuits (right is not evaluated, preventing error)
    {
        expr bad_div = make_binary(make_literal_int(1), make_op(INT_DIVIDE, "//", 1), make_literal_int(0));
        expr e = make_binary(make_literal_bool(false), make_op(AND, "and"), std::move(bad_div));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }
    {
        expr bad_div = make_binary(make_literal_int(1), make_op(INT_DIVIDE, "//", 1), make_literal_int(0));
        expr e = make_binary(make_literal_int(0), make_op(AND, "and"), std::move(bad_div));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "0\n");
        assert(res.err.empty());
    }
    {
        expr bad_div = make_binary(make_literal_int(1), make_op(INT_DIVIDE, "//", 1), make_literal_int(0));
        expr e = make_binary(make_literal_str(""), make_op(AND, "and"), std::move(bad_div));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "\n");
        assert(res.err.empty());
    }
    {
        expr bad_div = make_binary(make_literal_int(1), make_op(INT_DIVIDE, "//", 1), make_literal_int(0));
        expr e = make_binary(make_literal_none(), make_op(AND, "and"), std::move(bad_div));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "None\n");
        assert(res.err.empty());
    }

    // Truthy left evaluates right and returns it
    {
        expr e = make_binary(make_literal_bool(true), make_op(AND, "and"), make_literal_int(42));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "42\n");
        assert(res.err.empty());
    }
    {
        expr e = make_binary(make_literal_int(1), make_op(AND, "and"), make_literal_str("abc"));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "abc\n");
        assert(res.err.empty());
    }
    {
        expr e = make_binary(make_literal_str("yes"), make_op(AND, "and"), make_literal_none());
        auto res = capture_interpret(std::move(e));
        assert(res.out == "None\n");
        assert(res.err.empty());
    }
    {
        expr e = make_binary(make_literal_int(-3), make_op(AND, "and"), make_literal_bool(false));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }
    // Truthy left evaluates right, and if right errors, it throws
    {
        expr bad_div = make_binary(make_literal_int(1), make_op(INT_DIVIDE, "//", 9), make_literal_int(0));
        expr e = make_binary(make_literal_bool(true), make_op(AND, "and"), std::move(bad_div));
        auto res = capture_interpret(std::move(e));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: division by zero on line 9\n");
    }

    std::cout << "  test_interpreter_binary_and passed!\n";
}

void test_interpreter_binary_or() {
    // Truthy left short-circuits (right is not evaluated, preventing error)
    {
        expr bad_div = make_binary(make_literal_int(1), make_op(INT_DIVIDE, "//", 1), make_literal_int(0));
        expr e = make_binary(make_literal_bool(true), make_op(OR, "or"), std::move(bad_div));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        expr bad_div = make_binary(make_literal_int(1), make_op(INT_DIVIDE, "//", 1), make_literal_int(0));
        expr e = make_binary(make_literal_int(42), make_op(OR, "or"), std::move(bad_div));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "42\n");
        assert(res.err.empty());
    }
    {
        expr bad_div = make_binary(make_literal_int(1), make_op(INT_DIVIDE, "//", 1), make_literal_int(0));
        expr e = make_binary(make_literal_str("nonempty"), make_op(OR, "or"), std::move(bad_div));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "nonempty\n");
        assert(res.err.empty());
    }

    // Falsy left evaluates right and returns it
    {
        expr e = make_binary(make_literal_bool(false), make_op(OR, "or"), make_literal_int(100));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "100\n");
        assert(res.err.empty());
    }
    {
        expr e = make_binary(make_literal_int(0), make_op(OR, "or"), make_literal_str("fallback"));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "fallback\n");
        assert(res.err.empty());
    }
    {
        expr e = make_binary(make_literal_str(""), make_op(OR, "or"), make_literal_none());
        auto res = capture_interpret(std::move(e));
        assert(res.out == "None\n");
        assert(res.err.empty());
    }
    {
        expr e = make_binary(make_literal_none(), make_op(OR, "or"), make_literal_bool(true));
        auto res = capture_interpret(std::move(e));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    // Falsy left evaluates right, and if right errors, it throws
    {
        expr bad_div = make_binary(make_literal_int(1), make_op(INT_DIVIDE, "//", 15), make_literal_int(0));
        expr e = make_binary(make_literal_bool(false), make_op(OR, "or"), std::move(bad_div));
        auto res = capture_interpret(std::move(e));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: division by zero on line 15\n");
    }

    std::cout << "  test_interpreter_binary_or passed!\n";
}

// --- Binary Arithmetic Unit Tests ---

void test_interpreter_binary_add() {
    // int + int
    {
        auto res = capture_interpret(make_binary(make_literal_int(10), make_op(ADD, "+"), make_literal_int(20)));
        assert(res.out == "30\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(-5), make_op(ADD, "+"), make_literal_int(15)));
        assert(res.out == "10\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(0), make_op(ADD, "+"), make_literal_int(0)));
        assert(res.out == "0\n");
        assert(res.err.empty());
    }

    // str + str
    {
        auto res = capture_interpret(make_binary(make_literal_str("hello "), make_op(ADD, "+"), make_literal_str("world")));
        assert(res.out == "hello world\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str(""), make_op(ADD, "+"), make_literal_str("abc")));
        assert(res.out == "abc\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("xyz"), make_op(ADD, "+"), make_literal_str("")));
        assert(res.out == "xyz\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_binary_add passed!\n";
}

void test_interpreter_binary_add_errors() {
    // int + str
    {
        auto res = capture_interpret(make_binary(make_literal_int(1), make_op(ADD, "+", 10), make_literal_str("a")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for + on line 10\n");
    }
    // str + int
    {
        auto res = capture_interpret(make_binary(make_literal_str("a"), make_op(ADD, "+", 11), make_literal_int(1)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for + on line 11\n");
    }
    // bool + bool
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(ADD, "+", 12), make_literal_bool(false)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for + on line 12\n");
    }
    // None + None
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(ADD, "+", 13), make_literal_none()));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for + on line 13\n");
    }
    // int + bool
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(ADD, "+", 14), make_literal_bool(true)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for + on line 14\n");
    }
    // str + None
    {
        auto res = capture_interpret(make_binary(make_literal_str("test"), make_op(ADD, "+", 15), make_literal_none()));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for + on line 15\n");
    }

    std::cout << "  test_interpreter_binary_add_errors passed!\n";
}

void test_interpreter_binary_minus() {
    // int - int
    {
        auto res = capture_interpret(make_binary(make_literal_int(30), make_op(MINUS, "-"), make_literal_int(10)));
        assert(res.out == "20\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(MINUS, "-"), make_literal_int(15)));
        assert(res.out == "-10\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(0), make_op(MINUS, "-"), make_literal_int(0)));
        assert(res.out == "0\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_binary_minus passed!\n";
}

void test_interpreter_binary_minus_errors() {
    // str - str
    {
        auto res = capture_interpret(make_binary(make_literal_str("a"), make_op(MINUS, "-", 20), make_literal_str("b")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for - on line 20\n");
    }
    // int - str
    {
        auto res = capture_interpret(make_binary(make_literal_int(10), make_op(MINUS, "-", 21), make_literal_str("a")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for - on line 21\n");
    }
    // str - int
    {
        auto res = capture_interpret(make_binary(make_literal_str("a"), make_op(MINUS, "-", 22), make_literal_int(5)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for - on line 22\n");
    }
    // bool - bool
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(MINUS, "-", 23), make_literal_bool(false)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for - on line 23\n");
    }
    // None - None
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(MINUS, "-", 24), make_literal_none()));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for - on line 24\n");
    }
    // int - bool
    {
        auto res = capture_interpret(make_binary(make_literal_int(10), make_op(MINUS, "-", 25), make_literal_bool(true)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for - on line 25\n");
    }

    std::cout << "  test_interpreter_binary_minus_errors passed!\n";
}

void test_interpreter_binary_multiply() {
    // int * int
    {
        auto res = capture_interpret(make_binary(make_literal_int(6), make_op(MULTIPLY, "*"), make_literal_int(7)));
        assert(res.out == "42\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(-3), make_op(MULTIPLY, "*"), make_literal_int(4)));
        assert(res.out == "-12\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(MULTIPLY, "*"), make_literal_int(0)));
        assert(res.out == "0\n");
        assert(res.err.empty());
    }

    // str * int (repetition)
    {
        auto res = capture_interpret(make_binary(make_literal_str("ab"), make_op(MULTIPLY, "*"), make_literal_int(3)));
        assert(res.out == "ababab\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("hello"), make_op(MULTIPLY, "*"), make_literal_int(1)));
        assert(res.out == "hello\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("ab"), make_op(MULTIPLY, "*"), make_literal_int(0)));
        assert(res.out == "\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("ab"), make_op(MULTIPLY, "*"), make_literal_int(-2)));
        assert(res.out == "\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_binary_multiply passed!\n";
}

void test_interpreter_binary_multiply_errors() {
    // int * str
    {
        auto res = capture_interpret(make_binary(make_literal_int(3), make_op(MULTIPLY, "*", 30), make_literal_str("ab")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for * on line 30\n");
    }
    // str * str
    {
        auto res = capture_interpret(make_binary(make_literal_str("a"), make_op(MULTIPLY, "*", 31), make_literal_str("b")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for * on line 31\n");
    }
    // bool * int
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(MULTIPLY, "*", 32), make_literal_int(3)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for * on line 32\n");
    }
    // None * int
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(MULTIPLY, "*", 33), make_literal_int(2)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for * on line 33\n");
    }
    // str * None
    {
        auto res = capture_interpret(make_binary(make_literal_str("a"), make_op(MULTIPLY, "*", 34), make_literal_none()));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for * on line 34\n");
    }

    std::cout << "  test_interpreter_binary_multiply_errors passed!\n";
}

void test_interpreter_binary_int_divide() {
    // int // int
    {
        auto res = capture_interpret(make_binary(make_literal_int(20), make_op(INT_DIVIDE, "//"), make_literal_int(4)));
        assert(res.out == "5\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(7), make_op(INT_DIVIDE, "//"), make_literal_int(2)));
        assert(res.out == "3\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(-7), make_op(INT_DIVIDE, "//"), make_literal_int(2)));
        assert(res.out == "-3\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(7), make_op(INT_DIVIDE, "//"), make_literal_int(-2)));
        assert(res.out == "-3\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(0), make_op(INT_DIVIDE, "//"), make_literal_int(5)));
        assert(res.out == "0\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_binary_int_divide passed!\n";
}

void test_interpreter_binary_int_divide_errors() {
    // Division by zero
    {
        auto res = capture_interpret(make_binary(make_literal_int(10), make_op(INT_DIVIDE, "//", 40), make_literal_int(0)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: division by zero on line 40\n");
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(0), make_op(INT_DIVIDE, "//", 41), make_literal_int(0)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: division by zero on line 41\n");
    }

    // Unsupported operands
    {
        auto res = capture_interpret(make_binary(make_literal_str("a"), make_op(INT_DIVIDE, "//", 42), make_literal_str("b")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for // on line 42\n");
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(10), make_op(INT_DIVIDE, "//", 43), make_literal_str("2")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for // on line 43\n");
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("hello"), make_op(INT_DIVIDE, "//", 44), make_literal_int(2)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for // on line 44\n");
    }
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(INT_DIVIDE, "//", 45), make_literal_bool(true)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for // on line 45\n");
    }
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(INT_DIVIDE, "//", 46), make_literal_none()));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for // on line 46\n");
    }

    std::cout << "  test_interpreter_binary_int_divide_errors passed!\n";
}

// --- Binary Equality & Comparison Unit Tests ---

void test_interpreter_binary_equal() {
    // int == int
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(EQUAL, "=="), make_literal_int(5)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(EQUAL, "=="), make_literal_int(6)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // str == str
    {
        auto res = capture_interpret(make_binary(make_literal_str("hello"), make_op(EQUAL, "=="), make_literal_str("hello")));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("hello"), make_op(EQUAL, "=="), make_literal_str("world")));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // bool == bool
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(EQUAL, "=="), make_literal_bool(true)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_bool(false), make_op(EQUAL, "=="), make_literal_bool(false)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(EQUAL, "=="), make_literal_bool(false)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // None == None
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(EQUAL, "=="), make_literal_none()));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }

    // Cross-type comparisons
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(EQUAL, "=="), make_literal_str("5")));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(EQUAL, "=="), make_literal_int(1)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(EQUAL, "=="), make_literal_int(0)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str(""), make_op(EQUAL, "=="), make_literal_none()));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_binary_equal passed!\n";
}

void test_interpreter_binary_not_equal() {
    // int != int
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(NOT_EQUAL, "!="), make_literal_int(6)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(NOT_EQUAL, "!="), make_literal_int(5)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // str != str
    {
        auto res = capture_interpret(make_binary(make_literal_str("a"), make_op(NOT_EQUAL, "!="), make_literal_str("b")));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("a"), make_op(NOT_EQUAL, "!="), make_literal_str("a")));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // bool != bool
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(NOT_EQUAL, "!="), make_literal_bool(false)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(NOT_EQUAL, "!="), make_literal_bool(true)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // None != None
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(NOT_EQUAL, "!="), make_literal_none()));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // Cross-type comparisons
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(NOT_EQUAL, "!="), make_literal_str("5")));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(NOT_EQUAL, "!="), make_literal_int(1)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(NOT_EQUAL, "!="), make_literal_int(0)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_binary_not_equal passed!\n";
}

void test_interpreter_binary_greater() {
    // int > int
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(GREATER, ">"), make_literal_int(3)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(3), make_op(GREATER, ">"), make_literal_int(5)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(3), make_op(GREATER, ">"), make_literal_int(3)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // str > str
    {
        auto res = capture_interpret(make_binary(make_literal_str("banana"), make_op(GREATER, ">"), make_literal_str("apple")));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("apple"), make_op(GREATER, ">"), make_literal_str("banana")));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("apple"), make_op(GREATER, ">"), make_literal_str("apple")));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_binary_greater passed!\n";
}

void test_interpreter_binary_greater_errors() {
    // int > str
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(GREATER, ">", 50), make_literal_str("3")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for > on line 50\n");
    }
    // str > int
    {
        auto res = capture_interpret(make_binary(make_literal_str("5"), make_op(GREATER, ">", 51), make_literal_int(3)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for > on line 51\n");
    }
    // bool > bool
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(GREATER, ">", 52), make_literal_bool(false)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for > on line 52\n");
    }
    // None > None
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(GREATER, ">", 53), make_literal_none()));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for > on line 53\n");
    }
    // str > None
    {
        auto res = capture_interpret(make_binary(make_literal_str("a"), make_op(GREATER, ">", 54), make_literal_none()));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for > on line 54\n");
    }

    std::cout << "  test_interpreter_binary_greater_errors passed!\n";
}

void test_interpreter_binary_greater_equal() {
    // int >= int
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(GREATER_EQUAL, ">="), make_literal_int(3)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(GREATER_EQUAL, ">="), make_literal_int(5)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(3), make_op(GREATER_EQUAL, ">="), make_literal_int(5)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // str >= str
    {
        auto res = capture_interpret(make_binary(make_literal_str("banana"), make_op(GREATER_EQUAL, ">="), make_literal_str("apple")));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("apple"), make_op(GREATER_EQUAL, ">="), make_literal_str("apple")));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("apple"), make_op(GREATER_EQUAL, ">="), make_literal_str("banana")));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_binary_greater_equal passed!\n";
}

void test_interpreter_binary_greater_equal_errors() {
    // int >= str
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(GREATER_EQUAL, ">=", 60), make_literal_str("3")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for >= on line 60\n");
    }
    // str >= int
    {
        auto res = capture_interpret(make_binary(make_literal_str("5"), make_op(GREATER_EQUAL, ">=", 61), make_literal_int(3)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for >= on line 61\n");
    }
    // bool >= bool
    {
        auto res = capture_interpret(make_binary(make_literal_bool(true), make_op(GREATER_EQUAL, ">=", 62), make_literal_bool(false)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for >= on line 62\n");
    }
    // None >= None
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(GREATER_EQUAL, ">=", 63), make_literal_none()));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for >= on line 63\n");
    }

    std::cout << "  test_interpreter_binary_greater_equal_errors passed!\n";
}

void test_interpreter_binary_less() {
    // int < int
    {
        auto res = capture_interpret(make_binary(make_literal_int(3), make_op(LESS, "<"), make_literal_int(5)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(LESS, "<"), make_literal_int(3)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(3), make_op(LESS, "<"), make_literal_int(3)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // str < str
    {
        auto res = capture_interpret(make_binary(make_literal_str("apple"), make_op(LESS, "<"), make_literal_str("banana")));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("banana"), make_op(LESS, "<"), make_literal_str("apple")));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("apple"), make_op(LESS, "<"), make_literal_str("apple")));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_binary_less passed!\n";
}

void test_interpreter_binary_less_errors() {
    // int < str
    {
        auto res = capture_interpret(make_binary(make_literal_int(3), make_op(LESS, "<", 70), make_literal_str("5")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for < on line 70\n");
    }
    // str < int
    {
        auto res = capture_interpret(make_binary(make_literal_str("3"), make_op(LESS, "<", 71), make_literal_int(5)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for < on line 71\n");
    }
    // bool < bool
    {
        auto res = capture_interpret(make_binary(make_literal_bool(false), make_op(LESS, "<", 72), make_literal_bool(true)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for < on line 72\n");
    }
    // None < None
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(LESS, "<", 73), make_literal_none()));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for < on line 73\n");
    }

    std::cout << "  test_interpreter_binary_less_errors passed!\n";
}

void test_interpreter_binary_less_equal() {
    // int <= int
    {
        auto res = capture_interpret(make_binary(make_literal_int(3), make_op(LESS_EQUAL, "<="), make_literal_int(5)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(3), make_op(LESS_EQUAL, "<="), make_literal_int(3)));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(5), make_op(LESS_EQUAL, "<="), make_literal_int(3)));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    // str <= str
    {
        auto res = capture_interpret(make_binary(make_literal_str("apple"), make_op(LESS_EQUAL, "<="), make_literal_str("banana")));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("apple"), make_op(LESS_EQUAL, "<="), make_literal_str("apple")));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }
    {
        auto res = capture_interpret(make_binary(make_literal_str("banana"), make_op(LESS_EQUAL, "<="), make_literal_str("apple")));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_binary_less_equal passed!\n";
}

void test_interpreter_binary_less_equal_errors() {
    // int <= str
    {
        auto res = capture_interpret(make_binary(make_literal_int(3), make_op(LESS_EQUAL, "<=", 80), make_literal_str("5")));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for <= on line 80\n");
    }
    // str <= int
    {
        auto res = capture_interpret(make_binary(make_literal_str("3"), make_op(LESS_EQUAL, "<=", 81), make_literal_int(5)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for <= on line 81\n");
    }
    // bool <= bool
    {
        auto res = capture_interpret(make_binary(make_literal_bool(false), make_op(LESS_EQUAL, "<=", 82), make_literal_bool(true)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for <= on line 82\n");
    }
    // None <= None
    {
        auto res = capture_interpret(make_binary(make_literal_none(), make_op(LESS_EQUAL, "<=", 83), make_literal_none()));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: unsupported operand type(s) for <= on line 83\n");
    }

    std::cout << "  test_interpreter_binary_less_equal_errors passed!\n";
}

void test_interpreter_binary_invalid_op_error() {
    // Binary expression with an unhandled operator type (e.g. MODULO or ASSIGN)
    {
        auto res = capture_interpret(make_binary(make_literal_int(10), make_op(MODULO, "%", 90), make_literal_int(3)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: Invalid op type on line 90\n");
    }
    {
        auto res = capture_interpret(make_binary(make_literal_int(10), make_op(ASSIGN, "=", 91), make_literal_int(5)));
        assert(res.out.empty());
        assert(res.err == "RuntimeError: Invalid op type on line 91\n");
    }

    std::cout << "  test_interpreter_binary_invalid_op_error passed!\n";
}

// --- Grouping Unit Tests ---

void test_interpreter_grouping() {
    // Simple grouping: (42)
    {
        auto res = capture_interpret(make_grouping(make_literal_int(42)));
        assert(res.out == "42\n");
        assert(res.err.empty());
    }

    // Nested grouping: (((10 + 20)))
    {
        expr add = make_binary(make_literal_int(10), make_op(ADD, "+"), make_literal_int(20));
        expr g1 = make_grouping(std::move(add));
        expr g2 = make_grouping(std::move(g1));
        expr g3 = make_grouping(std::move(g2));
        auto res = capture_interpret(std::move(g3));
        assert(res.out == "30\n");
        assert(res.err.empty());
    }

    // Grouping altering precedence: (2 + 3) * 4 -> 20
    {
        expr inner = make_binary(make_literal_int(2), make_op(ADD, "+"), make_literal_int(3));
        expr group = make_grouping(std::move(inner));
        expr mul = make_binary(std::move(group), make_op(MULTIPLY, "*"), make_literal_int(4));
        auto res = capture_interpret(std::move(mul));
        assert(res.out == "20\n");
        assert(res.err.empty());
    }

    // Grouping with unary minus: -(2 + 3) -> -5
    {
        expr inner = make_binary(make_literal_int(2), make_op(ADD, "+"), make_literal_int(3));
        expr group = make_grouping(std::move(inner));
        expr un = make_unary(make_op(MINUS, "-"), std::move(group));
        auto res = capture_interpret(std::move(un));
        assert(res.out == "-5\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_grouping passed!\n";
}

// --- Complex Expressions Unit Tests ---

void test_interpreter_complex_expressions() {
    // (not (5 > 10)) and ("abc" * 2 == "abcabc")
    {
        expr comp = make_binary(make_literal_int(5), make_op(GREATER, ">"), make_literal_int(10));
        expr not_comp = make_unary(make_op(NOT, "not "), std::move(comp));
        expr left_g = make_grouping(std::move(not_comp));

        expr mul = make_binary(make_literal_str("abc"), make_op(MULTIPLY, "*"), make_literal_int(2));
        expr eq = make_binary(std::move(mul), make_op(EQUAL, "=="), make_literal_str("abcabc"));
        expr right_g = make_grouping(std::move(eq));

        expr full = make_binary(std::move(left_g), make_op(AND, "and"), std::move(right_g));
        auto res = capture_interpret(std::move(full));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }

    // (10 // 3 + 4 * 2 == 11) or (False != False)
    {
        expr div = make_binary(make_literal_int(10), make_op(INT_DIVIDE, "//"), make_literal_int(3));
        expr mul = make_binary(make_literal_int(4), make_op(MULTIPLY, "*"), make_literal_int(2));
        expr add = make_binary(std::move(div), make_op(ADD, "+"), std::move(mul));
        expr eq = make_binary(std::move(add), make_op(EQUAL, "=="), make_literal_int(11));
        expr left_g = make_grouping(std::move(eq));

        expr ne = make_binary(make_literal_bool(false), make_op(NOT_EQUAL, "!="), make_literal_bool(false));
        expr right_g = make_grouping(std::move(ne));

        expr full = make_binary(std::move(left_g), make_op(OR, "or"), std::move(right_g));
        auto res = capture_interpret(std::move(full));
        assert(res.out == "True\n");
        assert(res.err.empty());
    }

    // -(3 + 2) * -4 -> (-5) * -4 -> 20
    {
        expr add = make_binary(make_literal_int(3), make_op(ADD, "+"), make_literal_int(2));
        expr left_u = make_unary(make_op(MINUS, "-"), make_grouping(std::move(add)));
        expr right_u = make_unary(make_op(MINUS, "-"), make_literal_int(4));
        expr mul = make_binary(std::move(left_u), make_op(MULTIPLY, "*"), std::move(right_u));
        auto res = capture_interpret(std::move(mul));
        assert(res.out == "20\n");
        assert(res.err.empty());
    }

    // not ("" or "hello") -> not "hello" -> False
    {
        expr or_expr = make_binary(make_literal_str(""), make_op(OR, "or"), make_literal_str("hello"));
        expr group = make_grouping(std::move(or_expr));
        expr not_expr = make_unary(make_op(NOT, "not "), std::move(group));
        auto res = capture_interpret(std::move(not_expr));
        assert(res.out == "False\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_complex_expressions passed!\n";
}

// --- Multiple Expressions & Error Halting Unit Tests ---

void test_interpreter_multiple_expressions_and_halt_on_error() {
    // Empty list of expressions
    {
        std::vector<expr> exprs;
        auto res = capture_interpret(exprs);
        assert(res.out.empty());
        assert(res.err.empty());
    }

    // Multiple valid expressions executed in order
    {
        std::vector<expr> exprs;
        exprs.push_back(make_binary(make_literal_int(1), make_op(ADD, "+"), make_literal_int(1)));
        exprs.push_back(make_binary(make_literal_str("hello "), make_op(ADD, "+"), make_literal_str("world")));
        exprs.push_back(make_binary(make_literal_bool(true), make_op(EQUAL, "=="), make_literal_bool(true)));

        auto res = capture_interpret(exprs);
        assert(res.out == "2\nhello world\nTrue\n");
        assert(res.err.empty());
    }

    // Multiple expressions where middle one throws an error:
    // Earlier expression should print to stdout, error printed to stderr, and subsequent expressions must NOT execute
    {
        std::vector<expr> exprs;
        exprs.push_back(make_binary(make_literal_int(10), make_op(ADD, "+"), make_literal_int(20)));
        exprs.push_back(make_binary(make_literal_int(10), make_op(INT_DIVIDE, "//", 2), make_literal_int(0)));
        exprs.push_back(make_binary(make_literal_int(5), make_op(MULTIPLY, "*"), make_literal_int(5)));

        auto res = capture_interpret(exprs);
        assert(res.out == "30\n");
        assert(res.err == "RuntimeError: division by zero on line 2\n");
    }

    std::cout << "  test_interpreter_multiple_expressions_and_halt_on_error passed!\n";
}

// --- Environment Unit Tests (Testing all branches of define, assign, get, getTypeOfValue) ---

void test_interpreter_environment_unit() {
    Environment env;

    // Define success: int, str, bool
    Token tok_x{IDENTIFIER, 1, 1, "x", 1, std::monostate{}};
    env.define(tok_x, INT_TYPE, Value(10));
    assert(std::get<int>(env.get(tok_x)) == 10);

    Token tok_s{IDENTIFIER, 1, 1, "s", 1, std::monostate{}};
    env.define(tok_s, STR_TYPE, Value(std::string("hello")));
    assert(std::get<std::string>(env.get(tok_s)) == "hello");

    Token tok_b{IDENTIFIER, 1, 1, "b", 1, std::monostate{}};
    env.define(tok_b, BOOL_TYPE, Value(true));
    assert(std::get<bool>(env.get(tok_b)) == true);

    // Error: Redefining already defined variable
    {
        bool caught = false;
        try {
            env.define(tok_x, INT_TYPE, Value(20));
        } catch (const std::runtime_error& e) {
            caught = true;
            std::string msg = e.what();
            assert(msg.find("already defined on line 1") != std::string::npos);
        }
        assert(caught);
    }

    // Error: Type mismatch on definition (INT_TYPE with string)
    {
        Token tok_y{IDENTIFIER, 2, 1, "y", 1, std::monostate{}};
        bool caught = false;
        try {
            env.define(tok_y, INT_TYPE, Value(std::string("not an int")));
        } catch (const std::runtime_error& e) {
            caught = true;
            std::string msg = e.what();
            assert(msg.find("Type mismatch on line 2") != std::string::npos);
        }
        assert(caught);
    }

    // Error: Type mismatch on definition (STR_TYPE with int)
    {
        Token tok_y{IDENTIFIER, 3, 1, "y", 1, std::monostate{}};
        bool caught = false;
        try {
            env.define(tok_y, STR_TYPE, Value(123));
        } catch (const std::runtime_error& e) {
            caught = true;
            std::string msg = e.what();
            assert(msg.find("Type mismatch on line 3") != std::string::npos);
        }
        assert(caught);
    }

    // Error: Type mismatch on definition (BOOL_TYPE with monostate)
    {
        Token tok_y{IDENTIFIER, 4, 1, "y", 1, std::monostate{}};
        bool caught = false;
        try {
            env.define(tok_y, BOOL_TYPE, Value(std::monostate{}));
        } catch (const std::runtime_error& e) {
            caught = true;
            std::string msg = e.what();
            assert(msg.find("Type mismatch on line 4") != std::string::npos);
        }
        assert(caught);
    }

    // Assign success: same types
    env.assign(tok_x, Value(42));
    assert(std::get<int>(env.get(tok_x)) == 42);

    env.assign(tok_s, Value(std::string("world")));
    assert(std::get<std::string>(env.get(tok_s)) == "world");

    env.assign(tok_b, Value(false));
    assert(std::get<bool>(env.get(tok_b)) == false);

    // Error: Assign to undefined variable
    {
        Token tok_unknown{IDENTIFIER, 5, 1, "unknown", 7, std::monostate{}};
        bool caught = false;
        try {
            env.assign(tok_unknown, Value(100));
        } catch (const std::runtime_error& e) {
            caught = true;
            std::string msg = e.what();
            assert(msg.find("Undefined variable 'unknown' on line 5") != std::string::npos);
        }
        assert(caught);
    }

    // Error: Type mismatch on reassignment (int to string)
    {
        bool caught = false;
        try {
            env.assign(tok_x, Value(std::string("string for int")));
        } catch (const std::runtime_error& e) {
            caught = true;
            std::string msg = e.what();
            assert(msg.find("Type mismatch on reassignment to 'x' on line 1") != std::string::npos);
        }
        assert(caught);
    }

    // Error: Type mismatch on reassignment (string to bool)
    {
        bool caught = false;
        try {
            env.assign(tok_s, Value(true));
        } catch (const std::runtime_error& e) {
            caught = true;
            std::string msg = e.what();
            assert(msg.find("Type mismatch on reassignment to 's' on line 1") != std::string::npos);
        }
        assert(caught);
    }

    // Error: Get undefined variable
    {
        Token tok_unknown{IDENTIFIER, 6, 1, "unknown_var", 11, std::monostate{}};
        bool caught = false;
        try {
            env.get(tok_unknown);
        } catch (const std::runtime_error& e) {
            caught = true;
            std::string msg = e.what();
            assert(msg.find("Undefined variable 'unknown_var' on line 6") != std::string::npos);
        }
        assert(caught);
    }

    std::cout << "  test_interpreter_environment_unit passed!\n";
}

// --- Variable Declaration and Assignment Statements via Interpreter ---

void test_interpreter_var_decl_and_assign_stmts() {
    // Variable declaration and reading via varExpr in print
    {
        std::vector<stmt> stmts;
        Token tok_x = make_token(IDENTIFIER, "x", 1, 1);
        stmts.push_back(make_var_decl(INT_TYPE, tok_x, make_literal_int(42)));
        stmts.push_back(make_print_stmt(make_var_expr("x")));

        auto res = capture_interpret(stmts);
        assert(res.out == "42\n");
        assert(res.err.empty());
    }

    // Variable declaration, reassignment, and arithmetic with variables
    {
        std::vector<stmt> stmts;
        Token tok_x = make_token(IDENTIFIER, "x", 1, 1);
        Token tok_y = make_token(IDENTIFIER, "y", 2, 1);

        // x: int = 10
        stmts.push_back(make_var_decl(INT_TYPE, tok_x, make_literal_int(10)));
        // y: int = 20
        stmts.push_back(make_var_decl(INT_TYPE, tok_y, make_literal_int(20)));
        // x = x + y * 2  (10 + 40 = 50)
        expr mult = make_binary(make_var_expr("y"), make_op(MULTIPLY, "*"), make_literal_int(2));
        expr add = make_binary(make_var_expr("x"), make_op(ADD, "+"), std::move(mult));
        stmts.push_back(make_assign(tok_x, std::move(add)));
        // print x
        stmts.push_back(make_print_stmt(make_var_expr("x")));

        auto res = capture_interpret(stmts);
        assert(res.out == "50\n");
        assert(res.err.empty());
    }

    // String variables and concatenation
    {
        std::vector<stmt> stmts;
        Token tok_s1 = make_token(IDENTIFIER, "s1", 1, 1);
        Token tok_s2 = make_token(IDENTIFIER, "s2", 2, 1);
        Token tok_s3 = make_token(IDENTIFIER, "s3", 3, 1);

        // s1: str = "foo"
        stmts.push_back(make_var_decl(STR_TYPE, tok_s1, make_literal_str("foo")));
        // s2: str = "bar"
        stmts.push_back(make_var_decl(STR_TYPE, tok_s2, make_literal_str("bar")));
        // s3: str = s1 + s2
        expr add = make_binary(make_var_expr("s1"), make_op(ADD, "+"), make_var_expr("s2"));
        stmts.push_back(make_var_decl(STR_TYPE, tok_s3, std::move(add)));
        // print s3
        stmts.push_back(make_print_stmt(make_var_expr("s3")));

        auto res = capture_interpret(stmts);
        assert(res.out == "foobar\n");
        assert(res.err.empty());
    }

    // Boolean variables and logic
    {
        std::vector<stmt> stmts;
        Token tok_b = make_token(IDENTIFIER, "b", 1, 1);

        // b: bool = True
        stmts.push_back(make_var_decl(BOOL_TYPE, tok_b, make_literal_bool(true)));
        // print b
        stmts.push_back(make_print_stmt(make_var_expr("b")));
        // b = not b
        stmts.push_back(make_assign(tok_b, make_unary(make_op(NOT, "not "), make_var_expr("b"))));
        // print b
        stmts.push_back(make_print_stmt(make_var_expr("b")));

        auto res = capture_interpret(stmts);
        assert(res.out == "True\nFalse\n");
        assert(res.err.empty());
    }

    std::cout << "  test_interpreter_var_decl_and_assign_stmts passed!\n";
}

// --- Expression Statements via Interpreter ---

void test_interpreter_expression_statement() {
    // Expression statement executes without printing
    {
        std::vector<stmt> stmts;
        Token tok_x = make_token(IDENTIFIER, "x", 1, 1);
        // x: int = 100
        stmts.push_back(make_var_decl(INT_TYPE, tok_x, make_literal_int(100)));
        // x + 50 (exprStmt, does not modify x and does not print)
        stmts.push_back(make_expr_stmt(make_binary(make_var_expr("x"), make_op(ADD, "+"), make_literal_int(50))));
        // print x
        stmts.push_back(make_print_stmt(make_var_expr("x")));

        auto res = capture_interpret(stmts);
        assert(res.out == "100\n");
        assert(res.err.empty());
    }

    // Expression statement with runtime error halts execution and reports to err
    {
        std::vector<stmt> stmts;
        // 10 // 0 (exprStmt)
        stmts.push_back(make_expr_stmt(make_binary(make_literal_int(10), make_op(INT_DIVIDE, "//", 1), make_literal_int(0))));
        // print 42 (should not execute)
        stmts.push_back(make_print_stmt(make_literal_int(42)));

        auto res = capture_interpret(stmts);
        assert(res.out.empty());
        assert(res.err == "RuntimeError: division by zero on line 1\n");
    }

    std::cout << "  test_interpreter_expression_statement passed!\n";
}

// --- Runtime Error Handling in Statements ---

void test_interpreter_runtime_errors() {
    // Undefined variable in varExpr
    {
        std::vector<stmt> stmts;
        stmts.push_back(make_print_stmt(make_var_expr("unknown_var", 3, 1)));
        auto res = capture_interpret(stmts);
        assert(res.out.empty());
        assert(res.err == "RuntimeError: Undefined variable 'unknown_var' on line 3\n");
    }

    // Redefining variable
    {
        std::vector<stmt> stmts;
        Token tok_x1 = make_token(IDENTIFIER, "x", 1, 1);
        Token tok_x2 = make_token(IDENTIFIER, "x", 2, 1);
        stmts.push_back(make_var_decl(INT_TYPE, tok_x1, make_literal_int(1)));
        stmts.push_back(make_var_decl(INT_TYPE, tok_x2, make_literal_int(2)));
        auto res = capture_interpret(stmts);
        assert(res.out.empty());
        assert(res.err == "RuntimeError: Variable 'x' is already defined on line 2\n");
    }

    // Type mismatch on definition
    {
        std::vector<stmt> stmts;
        Token tok_x = make_token(IDENTIFIER, "x", 1, 1);
        stmts.push_back(make_var_decl(INT_TYPE, tok_x, make_literal_str("\"text\"", 1, 10)));
        auto res = capture_interpret(stmts);
        assert(res.out.empty());
        assert(res.err == "RuntimeError: Type mismatch on line 1\n");
    }

    // Type mismatch on reassignment
    {
        std::vector<stmt> stmts;
        Token tok_x1 = make_token(IDENTIFIER, "x", 1, 1);
        Token tok_x2 = make_token(IDENTIFIER, "x", 2, 1);
        stmts.push_back(make_var_decl(INT_TYPE, tok_x1, make_literal_int(10)));
        stmts.push_back(make_assign(tok_x2, make_literal_str("\"not int\"", 2, 5)));
        auto res = capture_interpret(stmts);
        assert(res.out.empty());
        assert(res.err == "RuntimeError: Type mismatch on reassignment to 'x' on line 2\n");
    }

    // Assigning to undefined variable
    {
        std::vector<stmt> stmts;
        Token tok_z = make_token(IDENTIFIER, "z", 4, 1);
        stmts.push_back(make_assign(tok_z, make_literal_int(99)));
        auto res = capture_interpret(stmts);
        assert(res.out.empty());
        assert(res.err == "RuntimeError: Undefined variable 'z' on line 4\n");
    }

    std::cout << "  test_interpreter_runtime_errors passed!\n";
}

// --- End-to-End Program Execution Tests (Lexer -> Parser -> Interpreter) ---

void test_interpreter_integration_programs() {
    // End-to-end: Arithmetic and variable reassignment
    {
        std::string_view code = 
            "x: int = 15\n"
            "y: int = 25\n"
            "z: int = x + y * 2\n"
            "print z\n"
            "z = z + 1\n"
            "print z\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();

        std::ostringstream out;
        std::ostringstream err;
        Interpreter interp;
        interp.interpret(ast, out, err);

        assert(out.str() == "65\n66\n");
        assert(err.str().empty());
    }

    // End-to-end: String assignment and comparison
    {
        std::string_view code = 
            "first: str = \"hello\"\n"
            "second: str = first\n"
            "print second\n"
            "print second == \"hello\"\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();

        std::ostringstream out;
        std::ostringstream err;
        Interpreter interp;
        interp.interpret(ast, out, err);

        assert(out.str() == "\"hello\"\nTrue\n");
        assert(err.str().empty());
    }

    // End-to-end: Boolean manipulation
    {
        std::string_view code = 
            "flag: bool = True\n"
            "print flag\n"
            "flag = not flag\n"
            "print flag\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();

        std::ostringstream out;
        std::ostringstream err;
        Interpreter interp;
        interp.interpret(ast, out, err);

        assert(out.str() == "True\nFalse\n");
        assert(err.str().empty());
    }

    // End-to-end: Expression statement followed by print
    {
        std::string_view code = 
            "10 + 20\n"
            "print 42\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();

        std::ostringstream out;
        std::ostringstream err;
        Interpreter interp;
        interp.interpret(ast, out, err);

        assert(out.str() == "42\n");
        assert(err.str().empty());
    }

    // End-to-end: Multiple assignments accumulating sum
    {
        std::string_view code = 
            "sum: int = 0\n"
            "sum = sum + 1\n"
            "sum = sum + 2\n"
            "sum = sum + 3\n"
            "print sum\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();

        std::ostringstream out;
        std::ostringstream err;
        Interpreter interp;
        interp.interpret(ast, out, err);

        assert(out.str() == "6\n");
        assert(err.str().empty());
    }

    std::cout << "  test_interpreter_integration_programs passed!\n";
}

// --- Master Runner Function ---

void run_interpreter_tests() {
    std::cout << "Running Interpreter unit tests...\n";
    test_interpreter_literals();
    test_interpreter_unary_not();
    test_interpreter_unary_minus();
    test_interpreter_unary_minus_errors();
    test_interpreter_binary_and();
    test_interpreter_binary_or();
    test_interpreter_binary_add();
    test_interpreter_binary_add_errors();
    test_interpreter_binary_minus();
    test_interpreter_binary_minus_errors();
    test_interpreter_binary_multiply();
    test_interpreter_binary_multiply_errors();
    test_interpreter_binary_int_divide();
    test_interpreter_binary_int_divide_errors();
    test_interpreter_binary_equal();
    test_interpreter_binary_not_equal();
    test_interpreter_binary_greater();
    test_interpreter_binary_greater_errors();
    test_interpreter_binary_greater_equal();
    test_interpreter_binary_greater_equal_errors();
    test_interpreter_binary_less();
    test_interpreter_binary_less_errors();
    test_interpreter_binary_less_equal();
    test_interpreter_binary_less_equal_errors();
    test_interpreter_binary_invalid_op_error();
    test_interpreter_grouping();
    test_interpreter_complex_expressions();
    test_interpreter_multiple_expressions_and_halt_on_error();

    // New tests for statements, environment, and program execution
    test_interpreter_environment_unit();
    test_interpreter_var_decl_and_assign_stmts();
    test_interpreter_expression_statement();
    test_interpreter_runtime_errors();
    test_interpreter_integration_programs();
}
