/*
The Lexer takes in a sequence of chars and makes tokens.
*/

#include <string>
#include <iostream>
#include <string_view>
#include <variant>

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
    std::variant<int, std::string, std::monostate> literal;
};