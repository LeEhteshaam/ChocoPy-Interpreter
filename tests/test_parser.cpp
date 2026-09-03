#include "../src/parser.hpp"
#include "../src/lexer.hpp"
#include "../src/ast_printer.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <memory>

// Helper function to capture AST print output for an expression
static std::string capture_ast(const expr& expression) {
    std::ostringstream oss;
    printAST(expression, oss);
    return oss.str();
}

// Helper function to extract expression from an exprStmt
static const expr& get_expr(const stmt& statement) {
    assert(std::holds_alternative<exprStmt>(statement.node));
    return *std::get<exprStmt>(statement.node).expression;
}

// Helper function to capture AST print output for an exprStmt
static std::string capture_ast(const stmt& statement) {
    return capture_ast(get_expr(statement));
}

// Helper to create a token manually
static Token make_test_token(TokenType type, std::string_view lexeme = "", int line = 1, int column = 1, std::variant<int, std::string_view, std::monostate> literal = std::monostate{}) {
    return Token{type, line, column, lexeme, lexeme.length(), literal};
}

// --- Primary Literals Unit Tests ---

void test_parser_primary_literals() {
    // Integer literal
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "42", 1, 1, 42),
            make_test_token(END_OF_FILE, "", 1, 3)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<literal>(e.node));
        const auto& lit = std::get<literal>(e.node);
        assert(lit.token.type == INT);
        assert(std::holds_alternative<int>(lit.val));
        assert(std::get<int>(lit.val) == 42);
        assert(capture_ast(ast[0]) == "42");
    }

    // Boolean True literal
    {
        std::vector<Token> tokens = {
            make_test_token(TRUE, "True", 1, 1),
            make_test_token(END_OF_FILE, "", 1, 5)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<literal>(e.node));
        const auto& lit = std::get<literal>(e.node);
        assert(lit.token.type == TRUE);
        assert(std::holds_alternative<bool>(lit.val));
        assert(std::get<bool>(lit.val) == true);
        assert(capture_ast(ast[0]) == "True");
    }

    // Boolean False literal
    {
        std::vector<Token> tokens = {
            make_test_token(FALSE, "False", 1, 1),
            make_test_token(END_OF_FILE, "", 1, 6)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<literal>(e.node));
        const auto& lit = std::get<literal>(e.node);
        assert(lit.token.type == FALSE);
        assert(std::holds_alternative<bool>(lit.val));
        assert(std::get<bool>(lit.val) == false);
        assert(capture_ast(ast[0]) == "False");
    }

    // None literal
    {
        std::vector<Token> tokens = {
            make_test_token(NONE, "None", 1, 1),
            make_test_token(END_OF_FILE, "", 1, 5)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<literal>(e.node));
        const auto& lit = std::get<literal>(e.node);
        assert(lit.token.type == NONE);
        assert(std::holds_alternative<std::monostate>(lit.val));
        assert(capture_ast(ast[0]) == "null");
    }

    // String literal
    {
        std::vector<Token> tokens = {
            make_test_token(STR, "\"hello world\"", 1, 1, std::string_view("\"hello world\"")),
            make_test_token(END_OF_FILE, "", 1, 14)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<literal>(e.node));
        const auto& lit = std::get<literal>(e.node);
        assert(lit.token.type == STR);
        assert(std::holds_alternative<std::string_view>(lit.val));
        assert(std::get<std::string_view>(lit.val) == "\"hello world\"");
        assert(capture_ast(ast[0]) == "\"hello world\"");
    }

    std::cout << "  test_parser_primary_literals passed!\n";
}

// --- Unary Operators Unit Tests ---

void test_parser_unary_operators() {
    // Unary minus: -42
    {
        std::vector<Token> tokens = {
            make_test_token(MINUS, "-", 1, 1),
            make_test_token(INT, "42", 1, 2, 42),
            make_test_token(END_OF_FILE, "", 1, 4)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<unary>(e.node));
        const auto& u = std::get<unary>(e.node);
        assert(u.op.type == MINUS);
        assert(std::holds_alternative<literal>(u.right->node));
        assert(capture_ast(ast[0]) == "-42");
    }

    // Unary not: not True
    {
        std::vector<Token> tokens = {
            make_test_token(NOT, "not ", 1, 1),
            make_test_token(TRUE, "True", 1, 5),
            make_test_token(END_OF_FILE, "", 1, 9)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<unary>(e.node));
        const auto& u = std::get<unary>(e.node);
        assert(u.op.type == NOT);
        assert(std::holds_alternative<literal>(u.right->node));
        assert(capture_ast(ast[0]) == "not True");
    }

    // Chained unary: not not False
    {
        std::vector<Token> tokens = {
            make_test_token(NOT, "not ", 1, 1),
            make_test_token(NOT, "not ", 1, 5),
            make_test_token(FALSE, "False", 1, 9),
            make_test_token(END_OF_FILE, "", 1, 14)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<unary>(e.node));
        const auto& u1 = std::get<unary>(e.node);
        assert(u1.op.type == NOT);
        assert(std::holds_alternative<unary>(u1.right->node));
        const auto& u2 = std::get<unary>(u1.right->node);
        assert(u2.op.type == NOT);
        assert(capture_ast(ast[0]) == "not not False");
    }

    // Chained unary: - - 100
    {
        std::vector<Token> tokens = {
            make_test_token(MINUS, "-", 1, 1),
            make_test_token(MINUS, "-", 1, 2),
            make_test_token(INT, "100", 1, 3, 100),
            make_test_token(END_OF_FILE, "", 1, 6)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(capture_ast(ast[0]) == "--100");
    }

    // Mixed chained unary: not - 5
    {
        std::vector<Token> tokens = {
            make_test_token(NOT, "not ", 1, 1),
            make_test_token(MINUS, "-", 1, 5),
            make_test_token(INT, "5", 1, 6, 5),
            make_test_token(END_OF_FILE, "", 1, 7)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(capture_ast(ast[0]) == "not -5");
    }

    // Mixed chained unary: - not True
    {
        std::vector<Token> tokens = {
            make_test_token(MINUS, "-", 1, 1),
            make_test_token(NOT, "not ", 1, 2),
            make_test_token(TRUE, "True", 1, 6),
            make_test_token(END_OF_FILE, "", 1, 10)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(capture_ast(ast[0]) == "-not True");
    }

    std::cout << "  test_parser_unary_operators passed!\n";
}

// --- Factor / Multiplicative Unit Tests ---

void test_parser_factor_operators() {
    // Multiplication: 6 * 7
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "6", 1, 1, 6),
            make_test_token(MULTIPLY, "*", 1, 3),
            make_test_token(INT, "7", 1, 5, 7),
            make_test_token(END_OF_FILE, "", 1, 6)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<binary>(e.node));
        const auto& b = std::get<binary>(e.node);
        assert(b.op.type == MULTIPLY);
        assert(capture_ast(ast[0]) == "6 * 7");
    }

    // Integer division: 100 // 5
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "100", 1, 1, 100),
            make_test_token(INT_DIVIDE, "//", 1, 5),
            make_test_token(INT, "5", 1, 8, 5),
            make_test_token(END_OF_FILE, "", 1, 9)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<binary>(e.node));
        const auto& b = std::get<binary>(e.node);
        assert(b.op.type == INT_DIVIDE);
        assert(capture_ast(ast[0]) == "100 // 5");
    }

    // Modulo: 17 % 4
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "17", 1, 1, 17),
            make_test_token(MODULO, "%", 1, 4),
            make_test_token(INT, "4", 1, 6, 4),
            make_test_token(END_OF_FILE, "", 1, 7)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<binary>(e.node));
        const auto& b = std::get<binary>(e.node);
        assert(b.op.type == MODULO);
        assert(capture_ast(ast[0]) == "17 % 4");
    }

    // Left associativity: 12 * 4 // 2 % 5
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "12", 1, 1, 12),
            make_test_token(MULTIPLY, "*", 1, 4),
            make_test_token(INT, "4", 1, 6, 4),
            make_test_token(INT_DIVIDE, "//", 1, 8),
            make_test_token(INT, "2", 1, 11, 2),
            make_test_token(MODULO, "%", 1, 13),
            make_test_token(INT, "5", 1, 15, 5),
            make_test_token(END_OF_FILE, "", 1, 16)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(capture_ast(ast[0]) == "12 * 4 // 2 % 5");
        // Check tree structure: ((12 * 4) // 2) % 5
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == MODULO);
        const auto& mid = std::get<binary>(root.left->node);
        assert(mid.op.type == INT_DIVIDE);
        const auto& leftMost = std::get<binary>(mid.left->node);
        assert(leftMost.op.type == MULTIPLY);
    }

    std::cout << "  test_parser_factor_operators passed!\n";
}

// --- Term / Additive Unit Tests ---

void test_parser_term_operators() {
    // Addition: 10 + 20
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "10", 1, 1, 10),
            make_test_token(ADD, "+", 1, 4),
            make_test_token(INT, "20", 1, 6, 20),
            make_test_token(END_OF_FILE, "", 1, 8)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<binary>(e.node));
        const auto& b = std::get<binary>(e.node);
        assert(b.op.type == ADD);
        assert(capture_ast(ast[0]) == "10 + 20");
    }

    // Subtraction: 30 - 15
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "30", 1, 1, 30),
            make_test_token(MINUS, "-", 1, 4),
            make_test_token(INT, "15", 1, 6, 15),
            make_test_token(END_OF_FILE, "", 1, 8)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<binary>(e.node));
        const auto& b = std::get<binary>(e.node);
        assert(b.op.type == MINUS);
        assert(capture_ast(ast[0]) == "30 - 15");
    }

    // Left associativity: 1 + 2 - 3 + 4
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "1", 1, 1, 1),
            make_test_token(ADD, "+", 1, 3),
            make_test_token(INT, "2", 1, 5, 2),
            make_test_token(MINUS, "-", 1, 7),
            make_test_token(INT, "3", 1, 9, 3),
            make_test_token(ADD, "+", 1, 11),
            make_test_token(INT, "4", 1, 13, 4),
            make_test_token(END_OF_FILE, "", 1, 14)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(capture_ast(ast[0]) == "1 + 2 - 3 + 4");
        // Check tree structure: (((1 + 2) - 3) + 4)
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == ADD);
        const auto& mid = std::get<binary>(root.left->node);
        assert(mid.op.type == MINUS);
        const auto& leftMost = std::get<binary>(mid.left->node);
        assert(leftMost.op.type == ADD);
    }

    // Precedence: term over factor (2 + 3 * 4) -> 2 + (3 * 4)
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "2", 1, 1, 2),
            make_test_token(ADD, "+", 1, 3),
            make_test_token(INT, "3", 1, 5, 3),
            make_test_token(MULTIPLY, "*", 1, 7),
            make_test_token(INT, "4", 1, 9, 4),
            make_test_token(END_OF_FILE, "", 1, 10)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == ADD);
        assert(std::holds_alternative<literal>(root.left->node));
        assert(std::holds_alternative<binary>(root.right->node));
        assert(std::get<binary>(root.right->node).op.type == MULTIPLY);
        assert(capture_ast(ast[0]) == "2 + 3 * 4");
    }

    // Precedence: factor first (2 * 3 + 4) -> (2 * 3) + 4
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "2", 1, 1, 2),
            make_test_token(MULTIPLY, "*", 1, 3),
            make_test_token(INT, "3", 1, 5, 3),
            make_test_token(ADD, "+", 1, 7),
            make_test_token(INT, "4", 1, 9, 4),
            make_test_token(END_OF_FILE, "", 1, 10)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == ADD);
        assert(std::holds_alternative<binary>(root.left->node));
        assert(std::get<binary>(root.left->node).op.type == MULTIPLY);
        assert(std::holds_alternative<literal>(root.right->node));
        assert(capture_ast(ast[0]) == "2 * 3 + 4");
    }

    std::cout << "  test_parser_term_operators passed!\n";
}

// --- Comparison Unit Tests ---

void test_parser_comparison_operators() {
    // Less: 1 < 2
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "1", 1, 1, 1),
            make_test_token(LESS, "<", 1, 3),
            make_test_token(INT, "2", 1, 5, 2),
            make_test_token(END_OF_FILE, "", 1, 6)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(get_expr(ast[0]).node).op.type == LESS);
        assert(capture_ast(ast[0]) == "1 < 2");
    }

    // Less equal: 2 <= 3
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "2", 1, 1, 2),
            make_test_token(LESS_EQUAL, "<=", 1, 3),
            make_test_token(INT, "3", 1, 6, 3),
            make_test_token(END_OF_FILE, "", 1, 7)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(get_expr(ast[0]).node).op.type == LESS_EQUAL);
        assert(capture_ast(ast[0]) == "2 <= 3");
    }

    // Greater: 5 > 3
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "5", 1, 1, 5),
            make_test_token(GREATER, ">", 1, 3),
            make_test_token(INT, "3", 1, 5, 3),
            make_test_token(END_OF_FILE, "", 1, 6)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(get_expr(ast[0]).node).op.type == GREATER);
        assert(capture_ast(ast[0]) == "5 > 3");
    }

    // Greater equal: 4 >= 4
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "4", 1, 1, 4),
            make_test_token(GREATER_EQUAL, ">=", 1, 3),
            make_test_token(INT, "4", 1, 6, 4),
            make_test_token(END_OF_FILE, "", 1, 7)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(get_expr(ast[0]).node).op.type == GREATER_EQUAL);
        assert(capture_ast(ast[0]) == "4 >= 4");
    }

    // Chained comparisons: 1 < 2 <= 3 > 0 >= 0
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "1", 1, 1, 1),
            make_test_token(LESS, "<", 1, 3),
            make_test_token(INT, "2", 1, 5, 2),
            make_test_token(LESS_EQUAL, "<=", 1, 7),
            make_test_token(INT, "3", 1, 10, 3),
            make_test_token(GREATER, ">", 1, 12),
            make_test_token(INT, "0", 1, 14, 0),
            make_test_token(GREATER_EQUAL, ">=", 1, 16),
            make_test_token(INT, "0", 1, 19, 0),
            make_test_token(END_OF_FILE, "", 1, 20)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(capture_ast(ast[0]) == "1 < 2 <= 3 > 0 >= 0");
    }

    // Precedence: comparison over term (1 + 2 < 3 + 4) -> (1 + 2) < (3 + 4)
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "1", 1, 1, 1),
            make_test_token(ADD, "+", 1, 3),
            make_test_token(INT, "2", 1, 5, 2),
            make_test_token(LESS, "<", 1, 7),
            make_test_token(INT, "3", 1, 9, 3),
            make_test_token(ADD, "+", 1, 11),
            make_test_token(INT, "4", 1, 13, 4),
            make_test_token(END_OF_FILE, "", 1, 14)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == LESS);
        assert(std::get<binary>(root.left->node).op.type == ADD);
        assert(std::get<binary>(root.right->node).op.type == ADD);
        assert(capture_ast(ast[0]) == "1 + 2 < 3 + 4");
    }

    std::cout << "  test_parser_comparison_operators passed!\n";
}

// --- Equality Unit Tests ---

void test_parser_equality_operators() {
    // Equal: 5 == 5
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "5", 1, 1, 5),
            make_test_token(EQUAL, "==", 1, 3),
            make_test_token(INT, "5", 1, 6, 5),
            make_test_token(END_OF_FILE, "", 1, 7)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(get_expr(ast[0]).node).op.type == EQUAL);
        assert(capture_ast(ast[0]) == "5 == 5");
    }

    // Not equal: True != False
    {
        std::vector<Token> tokens = {
            make_test_token(TRUE, "True", 1, 1),
            make_test_token(NOT_EQUAL, "!=", 1, 6),
            make_test_token(FALSE, "False", 1, 9),
            make_test_token(END_OF_FILE, "", 1, 14)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(get_expr(ast[0]).node).op.type == NOT_EQUAL);
        assert(capture_ast(ast[0]) == "True != False");
    }

    // Chained equality: 1 == 2 != 3
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "1", 1, 1, 1),
            make_test_token(EQUAL, "==", 1, 3),
            make_test_token(INT, "2", 1, 6, 2),
            make_test_token(NOT_EQUAL, "!=", 1, 8),
            make_test_token(INT, "3", 1, 11, 3),
            make_test_token(END_OF_FILE, "", 1, 12)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(capture_ast(ast[0]) == "1 == 2 != 3");
    }

    // Precedence: equality over comparison (1 < 2 == 3 < 4) -> (1 < 2) == (3 < 4)
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "1", 1, 1, 1),
            make_test_token(LESS, "<", 1, 3),
            make_test_token(INT, "2", 1, 5, 2),
            make_test_token(EQUAL, "==", 1, 7),
            make_test_token(INT, "3", 1, 10, 3),
            make_test_token(LESS, "<", 1, 12),
            make_test_token(INT, "4", 1, 14, 4),
            make_test_token(END_OF_FILE, "", 1, 15)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == EQUAL);
        assert(std::get<binary>(root.left->node).op.type == LESS);
        assert(std::get<binary>(root.right->node).op.type == LESS);
        assert(capture_ast(ast[0]) == "1 < 2 == 3 < 4");
    }

    std::cout << "  test_parser_equality_operators passed!\n";
}

// --- Grouping Unit Tests ---

void test_parser_grouping() {
    // Simple grouping: (42)
    {
        std::vector<Token> tokens = {
            make_test_token(LEFT_PAREN, "(", 1, 1),
            make_test_token(INT, "42", 1, 2, 42),
            make_test_token(RIGHT_PAREN, ")", 1, 4),
            make_test_token(END_OF_FILE, "", 1, 5)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<grouping>(e.node));
        const auto& g = std::get<grouping>(e.node);
        assert(std::holds_alternative<literal>(g.expression->node));
        assert(capture_ast(ast[0]) == "(42)");
    }

    // Nested grouping: (((42)))
    {
        std::vector<Token> tokens = {
            make_test_token(LEFT_PAREN, "(", 1, 1),
            make_test_token(LEFT_PAREN, "(", 1, 2),
            make_test_token(LEFT_PAREN, "(", 1, 3),
            make_test_token(INT, "42", 1, 4, 42),
            make_test_token(RIGHT_PAREN, ")", 1, 6),
            make_test_token(RIGHT_PAREN, ")", 1, 7),
            make_test_token(RIGHT_PAREN, ")", 1, 8),
            make_test_token(END_OF_FILE, "", 1, 9)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(capture_ast(ast[0]) == "(((42)))");
    }

    // Grouping overrides precedence: (2 + 3) * 4
    {
        std::vector<Token> tokens = {
            make_test_token(LEFT_PAREN, "(", 1, 1),
            make_test_token(INT, "2", 1, 2, 2),
            make_test_token(ADD, "+", 1, 4),
            make_test_token(INT, "3", 1, 6, 3),
            make_test_token(RIGHT_PAREN, ")", 1, 7),
            make_test_token(MULTIPLY, "*", 1, 9),
            make_test_token(INT, "4", 1, 11, 4),
            make_test_token(END_OF_FILE, "", 1, 12)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == MULTIPLY);
        assert(std::holds_alternative<grouping>(root.left->node));
        assert(capture_ast(ast[0]) == "(2 + 3) * 4");
    }

    // Unary with grouping: -(1 + 2)
    {
        std::vector<Token> tokens = {
            make_test_token(MINUS, "-", 1, 1),
            make_test_token(LEFT_PAREN, "(", 1, 2),
            make_test_token(INT, "1", 1, 3, 1),
            make_test_token(ADD, "+", 1, 5),
            make_test_token(INT, "2", 1, 7, 2),
            make_test_token(RIGHT_PAREN, ")", 1, 8),
            make_test_token(END_OF_FILE, "", 1, 9)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const expr& e = get_expr(ast[0]);
        assert(std::holds_alternative<unary>(e.node));
        const auto& u = std::get<unary>(e.node);
        assert(u.op.type == MINUS);
        assert(std::holds_alternative<grouping>(u.right->node));
        assert(capture_ast(ast[0]) == "-(1 + 2)");
    }

    std::cout << "  test_parser_grouping passed!\n";
}

// --- Complex Expressions Unit Tests ---

void test_parser_complex_expression() {
    // not (5 + 3 * 2 == 11 // 1) != (False == True)
    std::string_view code = "not (5 + 3 * 2 == 11 // 1) != (False == True)";
    std::vector<Token> tokens = tokenizer(code);
    Parser parser(tokens);
    std::vector<stmt> ast = parser.parse();
    assert(ast.size() == 1);
    assert(capture_ast(ast[0]) == "not(5 + 3 * 2 == 11 // 1) != (False == True)");

    std::cout << "  test_parser_complex_expression passed!\n";
}

// --- Empty and Multiple Expressions Unit Tests ---

void test_parser_empty_and_multiple() {
    // Empty stream (only EOF)
    {
        std::vector<Token> tokens = {
            make_test_token(END_OF_FILE, "", 1, 1)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.empty());
    }

    // Multiple expressions in sequence (with newlines between statements)
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "1", 1, 1, 1),
            make_test_token(ADD, "+", 1, 3),
            make_test_token(INT, "2", 1, 5, 2),
            make_test_token(NEW_LINE, "\n", 1, 6),
            make_test_token(INT, "3", 2, 1, 3),
            make_test_token(MULTIPLY, "*", 2, 3),
            make_test_token(INT, "4", 2, 5, 4),
            make_test_token(END_OF_FILE, "", 2, 6)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 2);
        assert(capture_ast(ast[0]) == "1 + 2");
        assert(capture_ast(ast[1]) == "3 * 4");
    }

    std::cout << "  test_parser_empty_and_multiple passed!\n";
}

// --- Error Handling Unit Tests ---

void test_parser_error_unclosed_paren() {
    // ( 1 + 2  (missing ')')
    std::vector<Token> tokens = {
        make_test_token(LEFT_PAREN, "(", 1, 1),
        make_test_token(INT, "1", 1, 3, 1),
        make_test_token(ADD, "+", 1, 5),
        make_test_token(INT, "2", 1, 7, 2),
        make_test_token(END_OF_FILE, "", 1, 8)
    };
    Parser parser(tokens);
    bool exceptionThrown = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        exceptionThrown = true;
        std::string msg = e.what();
        assert(msg.find("File contained syntax errors:") != std::string::npos);
        assert(msg.find("Expected a ')'") != std::string::npos);
    }
    assert(exceptionThrown);

    std::cout << "  test_parser_error_unclosed_paren passed!\n";
}

void test_parser_error_unexpected_primary() {
    // Unexpected token at start: * 5
    {
        std::vector<Token> tokens = {
            make_test_token(MULTIPLY, "*", 1, 1),
            make_test_token(INT, "5", 1, 3, 5),
            make_test_token(END_OF_FILE, "", 1, 4)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected a int/bool on line 1") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    // Trailing binary operator: 1 +
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "1", 2, 1, 1),
            make_test_token(ADD, "+", 2, 3),
            make_test_token(END_OF_FILE, "", 2, 4)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected a int/bool on line 2") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    // Trailing unary operator: not
    {
        std::vector<Token> tokens = {
            make_test_token(NOT, "not ", 3, 1),
            make_test_token(END_OF_FILE, "", 3, 5)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected a int/bool on line 3") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    std::cout << "  test_parser_error_unexpected_primary passed!\n";
}

// --- Panic Mode Synchronization Unit Tests (testing all synchronization branches) ---

void test_parser_synchronization_branches() {
    // 1. Synchronize stopping at NEW_LINE
    {
        std::vector<Token> tokens = {
            make_test_token(COMMA, ",", 1, 1),           // Error token
            make_test_token(DOT, ".", 1, 2),             // Skipped token
            make_test_token(NEW_LINE, "\n", 1, 3),       // Synchronizes here!
            make_test_token(INT, "42", 2, 1, 42),        // Parsed next
            make_test_token(END_OF_FILE, "", 2, 3)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
    }

    // 2. Synchronize stopping at CLASS
    {
        std::vector<Token> tokens = {
            make_test_token(COMMA, ",", 1, 1),           // Error token
            make_test_token(DOT, ".", 1, 2),             // Skipped token
            make_test_token(CLASS, "class", 1, 3),       // Synchronizes before CLASS
            make_test_token(END_OF_FILE, "", 1, 8)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
    }

    // 3. Synchronize stopping at DEF
    {
        std::vector<Token> tokens = {
            make_test_token(COMMA, ",", 1, 1),
            make_test_token(DEF, "def", 1, 2),           // Synchronizes before DEF
            make_test_token(END_OF_FILE, "", 1, 5)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
    }

    // 4. Synchronize stopping at IF
    {
        std::vector<Token> tokens = {
            make_test_token(COMMA, ",", 1, 1),
            make_test_token(IF, "if", 1, 2),             // Synchronizes before IF
            make_test_token(END_OF_FILE, "", 1, 4)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
    }

    // 5. Synchronize stopping at WHILE
    {
        std::vector<Token> tokens = {
            make_test_token(COMMA, ",", 1, 1),
            make_test_token(WHILE, "while", 1, 2),       // Synchronizes before WHILE
            make_test_token(END_OF_FILE, "", 1, 7)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
    }

    // 6. Synchronize stopping at PRINT
    {
        std::vector<Token> tokens = {
            make_test_token(COMMA, ",", 1, 1),
            make_test_token(PRINT, "print", 1, 2),       // Synchronizes before PRINT
            make_test_token(END_OF_FILE, "", 1, 7)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
    }

    // 7. Synchronize stopping at RETURN
    {
        std::vector<Token> tokens = {
            make_test_token(COMMA, ",", 1, 1),
            make_test_token(RETURN, "return", 1, 2),     // Synchronizes before RETURN
            make_test_token(END_OF_FILE, "", 1, 8)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
    }

    // 8. Synchronize reaching END_OF_FILE directly
    {
        std::vector<Token> tokens = {
            make_test_token(COMMA, ",", 1, 1),
            make_test_token(DOT, ".", 1, 2),
            make_test_token(COLON, ":", 1, 3),
            make_test_token(END_OF_FILE, "", 1, 4)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
    }

    std::cout << "  test_parser_synchronization_branches passed!\n";
}

// --- Variable Declarations Unit Tests (Branches: INT_TYPE, STR_TYPE, BOOL_TYPE, with/without newline, errors) ---

void test_parser_var_declaration() {
    // Int declaration with newline
    {
        std::string_view code = "x: int = 42\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<varDecl>(ast[0].node));
        const auto& decl = std::get<varDecl>(ast[0].node);
        assert(decl.type == INT_TYPE);
        assert(decl.identifier.lexeme == "x");
        assert(std::holds_alternative<literal>(decl.expression->node));
        assert(std::get<int>(std::get<literal>(decl.expression->node).val) == 42);
    }

    // Int declaration without newline at EOF
    {
        std::string_view code = "x: int = 42";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<varDecl>(ast[0].node));
        const auto& decl = std::get<varDecl>(ast[0].node);
        assert(decl.type == INT_TYPE);
        assert(decl.identifier.lexeme == "x");
    }

    // Str declaration with newline
    {
        std::string_view code = "s: str = \"hello\"\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<varDecl>(ast[0].node));
        const auto& decl = std::get<varDecl>(ast[0].node);
        assert(decl.type == STR_TYPE);
        assert(decl.identifier.lexeme == "s");
        assert(std::holds_alternative<literal>(decl.expression->node));
        assert(std::get<std::string_view>(std::get<literal>(decl.expression->node).val) == "\"hello\"");
    }

    // Bool declaration True with newline
    {
        std::string_view code = "b: bool = True\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<varDecl>(ast[0].node));
        const auto& decl = std::get<varDecl>(ast[0].node);
        assert(decl.type == BOOL_TYPE);
        assert(decl.identifier.lexeme == "b");
        assert(std::holds_alternative<literal>(decl.expression->node));
        assert(std::get<bool>(std::get<literal>(decl.expression->node).val) == true);
    }

    // Bool declaration False at EOF
    {
        std::string_view code = "b: bool = False";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<varDecl>(ast[0].node));
        const auto& decl = std::get<varDecl>(ast[0].node);
        assert(decl.type == BOOL_TYPE);
        assert(decl.identifier.lexeme == "b");
        assert(std::holds_alternative<literal>(decl.expression->node));
        assert(std::get<bool>(std::get<literal>(decl.expression->node).val) == false);
    }

    // Var declaration with complex expression
    {
        std::string_view code = "result: int = 10 + 20 * 2\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<varDecl>(ast[0].node));
        const auto& decl = std::get<varDecl>(ast[0].node);
        assert(decl.type == INT_TYPE);
        assert(decl.identifier.lexeme == "result");
        assert(std::holds_alternative<binary>(decl.expression->node));
    }

    // Error: Invalid type annotation (e.g. unknown type identifier)
    {
        std::vector<Token> tokens = {
            make_test_token(IDENTIFIER, "x", 1, 1),
            make_test_token(COLON, ":", 1, 2),
            make_test_token(IDENTIFIER, "float", 1, 4),
            make_test_token(ASSIGN, "=", 1, 10),
            make_test_token(INT, "1", 1, 12, 1),
            make_test_token(END_OF_FILE, "", 1, 13)
        };
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected type 'int', 'str', or 'bool'") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    // Error: Missing '=' after type
    {
        std::string_view code = "x: int 42\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected '='") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    // Error: Trailing token before newline (missing newline)
    {
        std::string_view code = "x: int = 1 2\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected a newline") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    std::cout << "  test_parser_var_declaration passed!\n";
}

// --- Assignment Statements Unit Tests (Branches: with/without newline, complex RHS, errors) ---

void test_parser_assign_statement() {
    // Simple assignment with newline
    {
        std::string_view code = "x = 42\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<assignStmt>(ast[0].node));
        const auto& assign = std::get<assignStmt>(ast[0].node);
        assert(assign.name.lexeme == "x");
        assert(std::holds_alternative<literal>(assign.value->node));
        assert(std::get<int>(std::get<literal>(assign.value->node).val) == 42);
    }

    // Simple assignment at EOF without newline
    {
        std::string_view code = "x = 100";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<assignStmt>(ast[0].node));
        const auto& assign = std::get<assignStmt>(ast[0].node);
        assert(assign.name.lexeme == "x");
    }

    // Assignment with complex expression RHS
    {
        std::string_view code = "x = y + 5 * 2\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<assignStmt>(ast[0].node));
        const auto& assign = std::get<assignStmt>(ast[0].node);
        assert(assign.name.lexeme == "x");
        assert(std::holds_alternative<binary>(assign.value->node));
    }

    // Assignment with string and bool
    {
        std::string_view code = "s = \"world\"\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<assignStmt>(ast[0].node));
    }
    {
        std::string_view code = "b = False\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<assignStmt>(ast[0].node));
    }

    // Error: Trailing tokens before newline
    {
        std::string_view code = "x = 1 2\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected a newline") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    std::cout << "  test_parser_assign_statement passed!\n";
}

// --- Print Statements Unit Tests (Branches: with/without newline, complex RHS, errors) ---

void test_parser_print_statement() {
    // Print integer with newline
    {
        std::string_view code = "print 42\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<printStmt>(ast[0].node));
        const auto& pr = std::get<printStmt>(ast[0].node);
        assert(std::holds_alternative<literal>(pr.expression->node));
        assert(std::get<int>(std::get<literal>(pr.expression->node).val) == 42);
    }

    // Print at EOF without newline
    {
        std::string_view code = "print 42";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<printStmt>(ast[0].node));
    }

    // Print complex expression
    {
        std::string_view code = "print (x + 10) * 2\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<printStmt>(ast[0].node));
        const auto& pr = std::get<printStmt>(ast[0].node);
        assert(std::holds_alternative<binary>(pr.expression->node));
    }

    // Print identifier
    {
        std::string_view code = "print my_var\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<printStmt>(ast[0].node));
        const auto& pr = std::get<printStmt>(ast[0].node);
        assert(std::holds_alternative<varExpr>(pr.expression->node));
        assert(std::get<varExpr>(pr.expression->node).name.lexeme == "my_var");
    }

    // Error: Trailing tokens before newline
    {
        std::string_view code = "print 1 2\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected a newline") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    std::cout << "  test_parser_print_statement passed!\n";
}

// --- Expression Statements Unit Tests (Branches: with/without newline, errors) ---

void test_parser_expression_statement() {
    // Expression statement with newline
    {
        std::string_view code = "1 + 2\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<exprStmt>(ast[0].node));
        const auto& es = std::get<exprStmt>(ast[0].node);
        assert(std::holds_alternative<binary>(es.expression->node));
    }

    // Expression statement at EOF without newline
    {
        std::string_view code = "1 + 2";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<exprStmt>(ast[0].node));
    }

    // Error: Trailing tokens before newline
    {
        std::string_view code = "1 2\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected a newline") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    std::cout << "  test_parser_expression_statement passed!\n";
}

// --- Variable Expressions in Primary Unit Tests ---

void test_parser_var_expression() {
    // Variable identifier in expression
    {
        std::string_view code = "x\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<exprStmt>(ast[0].node));
        const auto& es = std::get<exprStmt>(ast[0].node);
        assert(std::holds_alternative<varExpr>(es.expression->node));
        assert(std::get<varExpr>(es.expression->node).name.lexeme == "x");
    }

    // Variable in binary expression: x + y
    {
        std::string_view code = "x + y\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(std::holds_alternative<varExpr>(root.left->node));
        assert(std::get<varExpr>(root.left->node).name.lexeme == "x");
        assert(std::holds_alternative<varExpr>(root.right->node));
        assert(std::get<varExpr>(root.right->node).name.lexeme == "y");
    }

    // Variable in unary expression: not b
    {
        std::string_view code = "not b\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& u = std::get<unary>(get_expr(ast[0]).node);
        assert(std::holds_alternative<varExpr>(u.right->node));
        assert(std::get<varExpr>(u.right->node).name.lexeme == "b");
    }

    std::cout << "  test_parser_var_expression passed!\n";
}

// --- PeekNext Boundary Tests ---

void test_parser_peek_next_branches() {
    // Single identifier token at EOF: check(IDENTIFIER) is true, peekNext() checks token after it which is END_OF_FILE
    {
        std::vector<Token> tokens = {
            make_test_token(IDENTIFIER, "x", 1, 1),
            make_test_token(END_OF_FILE, "", 1, 2)
        };
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<exprStmt>(ast[0].node));
    }

    std::cout << "  test_parser_peek_next_branches passed!\n";
}

// --- Multi-statement Mixed Program Unit Tests ---

void test_parser_mixed_program() {
    std::string_view code = 
        "x: int = 5\n"
        "y: int = 10\n"
        "x = x + y\n"
        "print x\n";
    std::vector<Token> tokens = tokenizer(code);
    Parser parser(tokens);
    std::vector<stmt> ast = parser.parse();
    assert(ast.size() == 4);
    assert(std::holds_alternative<varDecl>(ast[0].node));
    assert(std::get<varDecl>(ast[0].node).identifier.lexeme == "x");
    assert(std::holds_alternative<varDecl>(ast[1].node));
    assert(std::get<varDecl>(ast[1].node).identifier.lexeme == "y");
    assert(std::holds_alternative<assignStmt>(ast[2].node));
    assert(std::get<assignStmt>(ast[2].node).name.lexeme == "x");
    assert(std::holds_alternative<printStmt>(ast[3].node));

    std::cout << "  test_parser_mixed_program passed!\n";
}

// --- Logical AND & OR Operators Unit Tests ---

void test_parser_logical_operators() {
    // Simple AND: True and False
    {
        std::string_view code = "True and False\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& b = std::get<binary>(get_expr(ast[0]).node);
        assert(b.op.type == AND);
        assert(std::holds_alternative<literal>(b.left->node));
        assert(std::get<bool>(std::get<literal>(b.left->node).val) == true);
        assert(std::holds_alternative<literal>(b.right->node));
        assert(std::get<bool>(std::get<literal>(b.right->node).val) == false);
        assert(capture_ast(ast[0]) == "True and False");
    }

    // Simple OR: a or b
    {
        std::string_view code = "a or b\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& b = std::get<binary>(get_expr(ast[0]).node);
        assert(b.op.type == OR);
        assert(std::holds_alternative<varExpr>(b.left->node));
        assert(std::get<varExpr>(b.left->node).name.lexeme == "a");
        assert(std::holds_alternative<varExpr>(b.right->node));
        assert(std::get<varExpr>(b.right->node).name.lexeme == "b");
        assert(capture_ast(ast[0]) == "a or b");
    }

    // Left associativity of AND: a and b and c -> ((a and b) and c)
    {
        std::string_view code = "a and b and c\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == AND);
        assert(std::holds_alternative<varExpr>(root.right->node));
        assert(std::get<varExpr>(root.right->node).name.lexeme == "c");
        assert(std::holds_alternative<binary>(root.left->node));
        const auto& left_bin = std::get<binary>(root.left->node);
        assert(left_bin.op.type == AND);
        assert(std::get<varExpr>(left_bin.left->node).name.lexeme == "a");
        assert(std::get<varExpr>(left_bin.right->node).name.lexeme == "b");
    }

    // Left associativity of OR: a or b or c -> ((a or b) or c)
    {
        std::string_view code = "a or b or c\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == OR);
        assert(std::holds_alternative<varExpr>(root.right->node));
        assert(std::get<varExpr>(root.right->node).name.lexeme == "c");
        assert(std::holds_alternative<binary>(root.left->node));
        const auto& left_bin = std::get<binary>(root.left->node);
        assert(left_bin.op.type == OR);
        assert(std::get<varExpr>(left_bin.left->node).name.lexeme == "a");
        assert(std::get<varExpr>(left_bin.right->node).name.lexeme == "b");
    }

    // Precedence: AND binds tighter than OR -> a or b and c -> a or (b and c)
    {
        std::string_view code = "a or b and c\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == OR);
        assert(std::holds_alternative<varExpr>(root.left->node));
        assert(std::get<varExpr>(root.left->node).name.lexeme == "a");
        assert(std::holds_alternative<binary>(root.right->node));
        const auto& right_bin = std::get<binary>(root.right->node);
        assert(right_bin.op.type == AND);
        assert(std::get<varExpr>(right_bin.left->node).name.lexeme == "b");
        assert(std::get<varExpr>(right_bin.right->node).name.lexeme == "c");
    }

    // Precedence: AND binds tighter than OR -> a and b or c -> (a and b) or c
    {
        std::string_view code = "a and b or c\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == OR);
        assert(std::holds_alternative<varExpr>(root.right->node));
        assert(std::get<varExpr>(root.right->node).name.lexeme == "c");
        assert(std::holds_alternative<binary>(root.left->node));
        const auto& left_bin = std::get<binary>(root.left->node);
        assert(left_bin.op.type == AND);
        assert(std::get<varExpr>(left_bin.left->node).name.lexeme == "a");
        assert(std::get<varExpr>(left_bin.right->node).name.lexeme == "b");
    }

    // Precedence: Equality binds tighter than AND -> x == 1 and y != 2
    {
        std::string_view code = "x == 1 and y != 2\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == AND);
        assert(std::holds_alternative<binary>(root.left->node));
        assert(std::get<binary>(root.left->node).op.type == EQUAL);
        assert(std::holds_alternative<binary>(root.right->node));
        assert(std::get<binary>(root.right->node).op.type == NOT_EQUAL);
    }

    // Precedence: Comparison binds tighter than AND/OR -> x > 0 and y <= 10 or z == 5
    {
        std::string_view code = "x > 0 and y <= 10 or z == 5\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == OR);
        assert(std::holds_alternative<binary>(root.left->node));
        const auto& left_and = std::get<binary>(root.left->node);
        assert(left_and.op.type == AND);
        assert(std::get<binary>(left_and.left->node).op.type == GREATER);
        assert(std::get<binary>(left_and.right->node).op.type == LESS_EQUAL);
        assert(std::holds_alternative<binary>(root.right->node));
        assert(std::get<binary>(root.right->node).op.type == EQUAL);
    }

    // Precedence: Unary NOT binds tighter than AND -> not a and not b
    {
        std::string_view code = "not a and not b\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == AND);
        assert(std::holds_alternative<unary>(root.left->node));
        assert(std::get<unary>(root.left->node).op.type == NOT);
        assert(std::holds_alternative<unary>(root.right->node));
        assert(std::get<unary>(root.right->node).op.type == NOT);
    }

    // Grouping overrides precedence: (a or b) and c
    {
        std::string_view code = "(a or b) and c\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(get_expr(ast[0]).node);
        assert(root.op.type == AND);
        assert(std::holds_alternative<grouping>(root.left->node));
        const auto& group = std::get<grouping>(root.left->node);
        assert(std::holds_alternative<binary>(group.expression->node));
        assert(std::get<binary>(group.expression->node).op.type == OR);
        assert(std::holds_alternative<varExpr>(root.right->node));
        assert(std::get<varExpr>(root.right->node).name.lexeme == "c");
    }

    std::cout << "  test_parser_logical_operators passed!\n";
}

// --- If Statement Unit Tests ---

void test_parser_if_statement() {
    // Simple if statement
    {
        std::string_view code = 
            "if x > 0:\n"
            "    print x\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<ifStmt>(ast[0].node));
        const auto& stmt_if = std::get<ifStmt>(ast[0].node);
        assert(std::holds_alternative<binary>(stmt_if.condition->node));
        assert(std::get<binary>(stmt_if.condition->node).op.type == GREATER);
        assert(stmt_if.ifBranch.size() == 1);
        assert(std::holds_alternative<printStmt>(stmt_if.ifBranch[0].node));
        assert(stmt_if.elseBranch.empty());
    }

    // If-else statement
    {
        std::string_view code = 
            "if x == 0:\n"
            "    print \"zero\"\n"
            "else:\n"
            "    print \"nonzero\"\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<ifStmt>(ast[0].node));
        const auto& stmt_if = std::get<ifStmt>(ast[0].node);
        assert(stmt_if.ifBranch.size() == 1);
        assert(std::holds_alternative<printStmt>(stmt_if.ifBranch[0].node));
        assert(stmt_if.elseBranch.size() == 1);
        assert(std::holds_alternative<printStmt>(stmt_if.elseBranch[0].node));
    }

    // If-elif-else statement (desugared to nested if in elseBranch)
    {
        std::string_view code = 
            "if x > 0:\n"
            "    print \"pos\"\n"
            "elif x < 0:\n"
            "    print \"neg\"\n"
            "else:\n"
            "    print \"zero\"\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<ifStmt>(ast[0].node));
        const auto& outer_if = std::get<ifStmt>(ast[0].node);
        assert(outer_if.ifBranch.size() == 1);
        assert(outer_if.elseBranch.size() == 1);
        assert(std::holds_alternative<ifStmt>(outer_if.elseBranch[0].node));
        const auto& elif_node = std::get<ifStmt>(outer_if.elseBranch[0].node);
        assert(elif_node.ifBranch.size() == 1);
        assert(elif_node.elseBranch.size() == 1);
        assert(std::holds_alternative<printStmt>(elif_node.elseBranch[0].node));
    }

    // Multiple elif blocks: if - elif - elif - else
    {
        std::string_view code = 
            "if x == 1:\n"
            "    print 1\n"
            "elif x == 2:\n"
            "    print 2\n"
            "elif x == 3:\n"
            "    print 3\n"
            "else:\n"
            "    print 0\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& if1 = std::get<ifStmt>(ast[0].node);
        assert(if1.elseBranch.size() == 1);
        const auto& if2 = std::get<ifStmt>(if1.elseBranch[0].node);
        assert(if2.elseBranch.size() == 1);
        const auto& if3 = std::get<ifStmt>(if2.elseBranch[0].node);
        assert(if3.elseBranch.size() == 1);
        assert(std::holds_alternative<printStmt>(if3.elseBranch[0].node));
    }

    // Nested if statement inside block
    {
        std::string_view code = 
            "if x > 0:\n"
            "    if y > 0:\n"
            "        print 1\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& outer_if = std::get<ifStmt>(ast[0].node);
        assert(outer_if.ifBranch.size() == 1);
        assert(std::holds_alternative<ifStmt>(outer_if.ifBranch[0].node));
    }

    // Error: Missing colon after if condition
    {
        std::string_view code = 
            "if x > 0\n"
            "    print x\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected ':' after if condition") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    // Error: Missing colon after else
    {
        std::string_view code = 
            "if x > 0:\n"
            "    print x\n"
            "else\n"
            "    print 0\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected ':' after else") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    // Error: Variable declaration not allowed inside block
    {
        std::string_view code = 
            "if True:\n"
            "    x: int = 5\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Variable declarations are not allowed inside blocks") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    std::cout << "  test_parser_if_statement passed!\n";
}

// --- While Statement Unit Tests ---

void test_parser_while_statement() {
    // Simple while loop
    {
        std::string_view code = 
            "while x > 0:\n"
            "    x = x - 1\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<whileStmt>(ast[0].node));
        const auto& stmt_while = std::get<whileStmt>(ast[0].node);
        assert(std::holds_alternative<binary>(stmt_while.condition->node));
        assert(std::get<binary>(stmt_while.condition->node).op.type == GREATER);
        assert(stmt_while.body.size() == 1);
        assert(std::holds_alternative<assignStmt>(stmt_while.body[0].node));
        assert(std::get<assignStmt>(stmt_while.body[0].node).name.lexeme == "x");
    }

    // While loop with multiple body statements including if
    {
        std::string_view code = 
            "while i < 10:\n"
            "    if i == 5:\n"
            "        print i\n"
            "    i = i + 1\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& stmt_while = std::get<whileStmt>(ast[0].node);
        assert(stmt_while.body.size() == 2);
        assert(std::holds_alternative<ifStmt>(stmt_while.body[0].node));
        assert(std::holds_alternative<assignStmt>(stmt_while.body[1].node));
    }

    // Nested while loop
    {
        std::string_view code = 
            "while i < 3:\n"
            "    while j < 3:\n"
            "        j = j + 1\n"
            "    i = i + 1\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& outer_while = std::get<whileStmt>(ast[0].node);
        assert(outer_while.body.size() == 2);
        assert(std::holds_alternative<whileStmt>(outer_while.body[0].node));
    }

    // Error: Missing colon after while condition
    {
        std::string_view code = 
            "while x > 0\n"
            "    x = x - 1\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected ':' after while condition") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    // Error: Variable declaration inside while block
    {
        std::string_view code = 
            "while True:\n"
            "    x: int = 1\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Variable declarations are not allowed inside blocks") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    std::cout << "  test_parser_while_statement passed!\n";
}

// --- For Statement Unit Tests ---

void test_parser_for_statement() {
    // Simple for loop with string literal iterable
    {
        std::string_view code = 
            "for c in \"hello\":\n"
            "    print c\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<forStmt>(ast[0].node));
        const auto& stmt_for = std::get<forStmt>(ast[0].node);
        assert(stmt_for.loopVar.type == IDENTIFIER);
        assert(stmt_for.loopVar.lexeme == "c");
        assert(std::holds_alternative<literal>(stmt_for.iterable->node));
        assert(stmt_for.body.size() == 1);
        assert(std::holds_alternative<printStmt>(stmt_for.body[0].node));
    }

    // For loop with identifier iterable
    {
        std::string_view code = 
            "for char in word:\n"
            "    print char\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& stmt_for = std::get<forStmt>(ast[0].node);
        assert(stmt_for.loopVar.lexeme == "char");
        assert(std::holds_alternative<varExpr>(stmt_for.iterable->node));
        assert(std::get<varExpr>(stmt_for.iterable->node).name.lexeme == "word");
    }

    // Nested for loop
    {
        std::string_view code = 
            "for i in \"ab\":\n"
            "    for j in \"cd\":\n"
            "        print j\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& outer_for = std::get<forStmt>(ast[0].node);
        assert(outer_for.loopVar.lexeme == "i");
        assert(outer_for.body.size() == 1);
        assert(std::holds_alternative<forStmt>(outer_for.body[0].node));
        const auto& inner_for = std::get<forStmt>(outer_for.body[0].node);
        assert(inner_for.loopVar.lexeme == "j");
    }

    // For loop containing if statement
    {
        std::string_view code = 
            "for c in \"a b c\":\n"
            "    if c != \" \":\n"
            "        print c\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        std::vector<stmt> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& stmt_for = std::get<forStmt>(ast[0].node);
        assert(stmt_for.body.size() == 1);
        assert(std::holds_alternative<ifStmt>(stmt_for.body[0].node));
    }

    // Error: Missing identifier after for
    {
        std::string_view code = 
            "for in \"abc\":\n"
            "    print 1\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected a identifier") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    // Error: Missing 'in' keyword
    {
        std::string_view code = 
            "for c \"abc\":\n"
            "    print c\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected 'in' after identifier") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    // Error: Missing colon after for condition
    {
        std::string_view code = 
            "for c in \"abc\"\n"
            "    print c\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Expected ':' after for condition") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    // Error: Variable declaration inside for block
    {
        std::string_view code = 
            "for c in \"abc\":\n"
            "    x: int = 1\n";
        std::vector<Token> tokens = tokenizer(code);
        Parser parser(tokens);
        bool exceptionThrown = false;
        try {
            parser.parse();
        } catch (const std::runtime_error& e) {
            exceptionThrown = true;
            std::string msg = e.what();
            assert(msg.find("Variable declarations are not allowed inside blocks") != std::string::npos);
        }
        assert(exceptionThrown);
    }

    std::cout << "  test_parser_for_statement passed!\n";
}

// --- Master Runner Function ---

void run_parser_tests() {
    std::cout << "Running Parser unit tests...\n";
    test_parser_primary_literals();
    test_parser_unary_operators();
    test_parser_factor_operators();
    test_parser_term_operators();
    test_parser_comparison_operators();
    test_parser_equality_operators();
    test_parser_grouping();
    test_parser_complex_expression();
    test_parser_empty_and_multiple();
    test_parser_error_unclosed_paren();
    test_parser_error_unexpected_primary();
    test_parser_synchronization_branches();

    // Statement and expression parsing
    test_parser_var_declaration();
    test_parser_assign_statement();
    test_parser_print_statement();
    test_parser_expression_statement();
    test_parser_var_expression();
    test_parser_peek_next_branches();
    test_parser_mixed_program();

    // Control flow and logical operator parsing
    test_parser_logical_operators();
    test_parser_if_statement();
    test_parser_while_statement();
    test_parser_for_statement();
}
