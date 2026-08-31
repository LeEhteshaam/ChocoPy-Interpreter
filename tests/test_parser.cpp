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

// Helper function to capture AST print output
static std::string capture_ast(const expr& expression) {
    std::ostringstream oss;
    printAST(expression, oss);
    return oss.str();
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<literal>(ast[0].node));
        const auto& lit = std::get<literal>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<literal>(ast[0].node));
        const auto& lit = std::get<literal>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<literal>(ast[0].node));
        const auto& lit = std::get<literal>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<literal>(ast[0].node));
        const auto& lit = std::get<literal>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<literal>(ast[0].node));
        const auto& lit = std::get<literal>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<unary>(ast[0].node));
        const auto& u = std::get<unary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<unary>(ast[0].node));
        const auto& u = std::get<unary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<unary>(ast[0].node));
        const auto& u1 = std::get<unary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
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
        std::vector<expr> ast = parser.parse();
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
        std::vector<expr> ast = parser.parse();
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<binary>(ast[0].node));
        const auto& b = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<binary>(ast[0].node));
        const auto& b = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<binary>(ast[0].node));
        const auto& b = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(capture_ast(ast[0]) == "12 * 4 // 2 % 5");
        // Check tree structure: ((12 * 4) // 2) % 5
        const auto& root = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<binary>(ast[0].node));
        const auto& b = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<binary>(ast[0].node));
        const auto& b = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(capture_ast(ast[0]) == "1 + 2 - 3 + 4");
        // Check tree structure: (((1 + 2) - 3) + 4)
        const auto& root = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(ast[0].node).op.type == LESS);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(ast[0].node).op.type == LESS_EQUAL);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(ast[0].node).op.type == GREATER);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(ast[0].node).op.type == GREATER_EQUAL);
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
        std::vector<expr> ast = parser.parse();
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(ast[0].node).op.type == EQUAL);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::get<binary>(ast[0].node).op.type == NOT_EQUAL);
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
        std::vector<expr> ast = parser.parse();
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<grouping>(ast[0].node));
        const auto& g = std::get<grouping>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        const auto& root = std::get<binary>(ast[0].node);
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
        std::vector<expr> ast = parser.parse();
        assert(ast.size() == 1);
        assert(std::holds_alternative<unary>(ast[0].node));
        const auto& u = std::get<unary>(ast[0].node);
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
    std::vector<expr> ast = parser.parse();
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
        std::vector<expr> ast = parser.parse();
        assert(ast.empty());
    }

    // Multiple expressions in sequence
    {
        std::vector<Token> tokens = {
            make_test_token(INT, "1", 1, 1, 1),
            make_test_token(ADD, "+", 1, 3),
            make_test_token(INT, "2", 1, 5, 2),
            make_test_token(INT, "3", 2, 1, 3),
            make_test_token(MULTIPLY, "*", 2, 3),
            make_test_token(INT, "4", 2, 5, 4),
            make_test_token(END_OF_FILE, "", 2, 6)
        };
        Parser parser(tokens);
        std::vector<expr> ast = parser.parse();
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
}
