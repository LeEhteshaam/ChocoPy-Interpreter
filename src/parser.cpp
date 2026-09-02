#include "parser.hpp"
#include <utility>
#include <initializer_list>
#include <format>
#include <ranges>

Parser::Parser(std::vector<Token> tokens) {
    this->tokens = std::move(tokens);
}

Token Parser::previous() {
    return tokens[counter - 1];
}

Token Parser::peek() {
    return tokens[counter];
}

bool Parser::isAtEnd() {
    Token cur = peek();
    return cur.type == END_OF_FILE;
}

Token Parser::advance() {
    if (!isAtEnd()) {
        counter += 1;
    }
    return previous();
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) {
        return false;
    }

    Token cur = peek();
    return cur.type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (const auto& type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

// Panic mode error recovery
void Parser::synchronize() {
    advance(); // Consume the token that initially caused the error

    while (!isAtEnd()) {
        if (previous().type == NEW_LINE) return;

        // Peek at the next token without advancing past it
        TokenType type = peek().type;
        if (type == CLASS || type == DEF || type == IF || 
            type == WHILE || type == PRINT || type == RETURN) {
            return;
        }

        advance();
    }
}

Token Parser::consume(TokenType type, std::string message) {
    if (check(type)) {
        return advance();
    }
    
    // add the error    
    errors.push_back(std::move(message));
    throw ParseError();
}

struct stmt Parser::statement() {
    if (match({PRINT})) {
        return printStatement();
    }

    return expressionStatement();
}

struct stmt Parser::printStatement() {
    Token prev = previous();
    expr val = expression();
    std::string msg = std::format("ParseError: Expected a newline on line {}", prev.line);
    consume(NEW_LINE, msg);
    return stmt { printStmt { std::make_unique<expr>(std::move(val)) } };
}

struct stmt Parser::expressionStatement() {
    Token prev = previous();
    expr val = expression();
    std::string msg = std::format("ParseError: Expected a newline on line {}", prev.line);
    consume(NEW_LINE, msg);
    return stmt { exprStmt { std::make_unique<expr>(std::move(val)) } };
}

struct expr Parser::expression() {
    return equality();
}

// Methods for parsing
struct expr Parser::equality() {
    expr expression = comparison();

    while (match({EQUAL, NOT_EQUAL})) {
        Token op = previous();
        expr right = comparison();
        
        expression = expr { binary {
            std::make_unique<expr>(std::move(expression)),
            op,
            std::make_unique<expr>(std::move(right))
        }};
    }

    return expression;
}

struct expr Parser::comparison() {
    expr expression = term();

    while (match({LESS, LESS_EQUAL, GREATER, GREATER_EQUAL})) {
        Token op = previous();
        expr right = term();

        expression = expr { binary {
            std::make_unique<expr>(std::move(expression)),
            op,
            std::make_unique<expr>(std::move(right))
        }};
    }

    return expression;
}

struct expr Parser::term() {
    expr expression = factor();

    while (match({ADD, MINUS})) {
        Token op = previous();
        expr right = factor();

        expression = expr { binary {
            std::make_unique<expr>(std::move(expression)),
            op,
            std::make_unique<expr>(std::move(right))
        }};
    }

    return expression;
}

struct expr Parser::factor() {
    expr expression = unary();

    while (match({INT_DIVIDE, MODULO, MULTIPLY})) {
        Token op = previous();
        expr right = unary();

        expression = expr { binary {
            std::make_unique<expr>(std::move(expression)),
            op,
            std::make_unique<expr>(std::move(right))
        }};
    }

    return expression;
}

struct expr Parser::unary() {
    while (match({NOT, MINUS})) {
        Token op = previous();
        expr right = unary();
        struct unary node = {
            op, 
            std::make_unique<expr>(std::move(right))
        };
        return expr { std::move(node) };
    }

    // If we did not find a unary, it must be a primary
    return primary();
}

struct expr Parser::primary() {
    if (match({FALSE})) return expr { literal { previous(), false } };
    if (match({TRUE})) return expr { literal { previous(), true } }; 
    if (match({NONE})) return expr { literal { previous(), std::monostate{} } };
    if (match({INT, STR})) {
        Token res = previous();
        return std::visit([&res](auto&& v) -> expr {
            return expr { literal { res, v } };
        }, res.literal);
    }

    // consume left paren
    if (match({LEFT_PAREN})) {
        expr inside = expression();

        // Verify that we have a right paren, else add an error + synchronize
        Token prev = previous();
        std::string msg = std::format("ParseError: Expected a ')' at line {}", prev.line);
        consume(RIGHT_PAREN, msg);
        return expr { grouping { std::make_unique<expr>(std::move(inside)) } };
    }

    // not valid syntax, return something but synchronize and add error message
    Token cur = peek();
    std::string msg = std::format("ParseError: Expected a int/bool on line {}", cur.line);
    errors.push_back(msg);
    throw ParseError();
}

std::vector<stmt> Parser::parse() {
    std::vector<stmt> ast_nodes;

    while (!isAtEnd()) {
        try {
            ast_nodes.push_back(statement());
        } catch (const ParseError& error) {
            // Panic mode caught the error, synchronize and try parsing the next line
            synchronize();
        }
    }

    if (!errors.empty()) {
        std::string final_error_message = "File contained syntax errors:\n";
        for (const std::string& err : errors) {
            final_error_message += err + "\n";
        }
        
        throw std::runtime_error(final_error_message);
    }

    return ast_nodes;
}