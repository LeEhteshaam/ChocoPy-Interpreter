#pragma once

#include <string_view>
#include <variant>
#include <unordered_map>
#include <vector>

enum TokenType {
    // Line structure 
    NEW_LINE, INDENT, DEDENT,

    // Literals
    STR, INT, IDENTIFIER,

    // OPERATORS
    ADD, MINUS, INT_DIVIDE, MULTIPLY, MODULO, ASSIGN, 
    EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,

    // Keywords
    DEF, CLASS, TRUE, FALSE, IF, ELIF, ELSE, RETURN, NONE, NOT, 
    PRINT, AND, OR, IS, GLOBAL, NONLOCAL, PASS, FOR, WHILE, 

    // Delimiters
    LEFT_PAREN, LEFT_BRAC, RIGHT_PAREN, RIGHT_BRAC, COLON, ARROW, COMMA, DOT, 

    // EOF 
    END_OF_FILE
};

struct Token {
    TokenType type;
    int line;
    int column;
    std::string_view lexeme;
    size_t tokenLength;
    std::variant<int, std::string_view, std::monostate> literal;
};

// Global token map declaration
extern std::unordered_map<std::string_view, TokenType> tokenMap;

// Lexer function declarations
bool isAlpha(char ch);
void advance(int *start, int* column, int length);
struct Token makeToken(std::string_view p, int *start, int length, int *column, int line);
struct Token nextToken(std::string_view buffer, int* cur, int* line, int* column);
std::vector<Token> tokenizer(std::string_view buffer);
