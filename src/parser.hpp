#pragma once

#include "ast.hpp"
#include "lexer.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#include <initializer_list>

class ParseError : public std::runtime_error {
public:
    ParseError() : std::runtime_error("") {}
};

class Parser { 
private:
    std::vector<Token> tokens;
    std::vector<std::string> errors;
    int counter = 0;

    // Helper functions for traversing token stream
    Token previous();
    Token peek();
    bool isAtEnd();
    Token advance();
    bool check(TokenType type);
    bool match(std::initializer_list<TokenType> types);

    // Panic mode error recovery
    void synchronize();
    Token consume(TokenType type, std::string message);

    // Methods for parsing
    struct expr expression();
    struct expr equality();
    struct expr comparison();
    struct expr term();
    struct expr factor();
    struct expr unary();
    struct expr primary();

public: 
    explicit Parser(std::vector<Token> tokens);
    std::vector<expr> parse();
};
