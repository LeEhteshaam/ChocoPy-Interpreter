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

Token Parser::peekNext() {
    if (counter + 1 >= tokens.size()) {
        return tokens.back(); 
    }
    return tokens[counter + 1];
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

    if (check(IDENTIFIER) && peekNext().type == COLON) {
        return varDeclaration();
    }
    
    if (check(IDENTIFIER) && peekNext().type == ASSIGN) {
        return assignStatement();
    }

    if (match({IF})) {
        return ifStatement();
    }

    if (match({WHILE})) {
        return whileStatement();
    }

    if (match({PRINT})) {
        return printStatement();
    } 

    return expressionStatement();
}

std::vector<stmt> Parser::block() {
    std::vector<stmt> res;
    Token prev = previous();

    consume(NEW_LINE, std::format("ParseError: Expected newline before block on line {}", previous().line));
    consume(INDENT, std::format("ParseError: Expected an indented block on line {}", previous().line));

    while(!isAtEnd() && !check(DEDENT)) {

        if (check(IDENTIFIER) && peekNext().type == COLON) {
            throw std::runtime_error(std::format("ParseError: Variable declarations are not allowed inside blocks on line {}", peek().line));
        }

        res.push_back(statement());
    }

    consume(DEDENT, std::format("ParseError: Expected dedent on line {}", previous().line));

    return res;
}

struct stmt Parser::whileStatement() {
    expr loopCondition = expression();
    consume(COLON, std::format("ParseError: Expected ':' after while condition on line {}", previous().line));    
    std::vector<stmt> loopBody = block();

    return stmt { whileStmt {
        std::make_unique<expr>(std::move(loopCondition)),
        std::move(loopBody)
    }};
}

struct stmt Parser::ifStatement() {
    expr branchCondition = expression();
    consume(COLON, std::format("ParseError: Expected ':' after if condition on line {}", previous().line));
    std::vector<stmt> ifBranch = block();
    std::vector<stmt> elseBranch;

    if (match({ELIF})) {
        elseBranch = { ifStatement() };
    } else if (match{ELSE}) {
        consume(COLON, std::format("ParseError: Expected ':' after else on line {}", previous().line));
        elseBranch = block();
    }

    return stmt { ifStmt {
        std::make_unqiue(std::move(branchCondition));
        std::move(ifBranch),
        std::move(elseBranch)
    }};
}

struct stmt Parser::varDeclaration() {
    Token name = consume(IDENTIFIER, std::format("ParseError: Expected variable name on line {}", peek().line));
    consume(COLON, std::format("ParseError: Expected ':' after '{}' on line {}", name.lexeme, name.line));

    if (!match({INT_TYPE, STR_TYPE, BOOL_TYPE})) {
        std::string msg = std::format("ParseError: Expected type 'int', 'str', or 'bool' on line {}", peek().line);
        errors.push_back(msg);
        throw ParseError();
    }
    Token idType = previous();
    consume(ASSIGN, std::format("ParseError: Expected '=' on line {}", name.line));
    expr val = expression();

    if (!isAtEnd()) {
        consume(NEW_LINE, std::format("ParseError: Expected a newline on line {}", name.line));
    }

    return stmt { varDecl { 
        idType.type, 
        name,
        std::make_unique<expr>(std::move(val))
    }};
}

struct stmt Parser::assignStatement() {
    Token name = consume(IDENTIFIER, "ParseError: Expected variable name");
    consume(ASSIGN, std::format("ParseError: Expected '=' after variable name on line {}", name.line));
    
    expr val = expression();
    
    if (!isAtEnd()) {
        consume(NEW_LINE, std::format("ParseError: Expected a newline on line {}", previous().line));
    }
    
    return stmt { assignStmt { name, std::make_unique<expr>(std::move(val)) } };
}

struct stmt Parser::printStatement() {
    Token prev = previous();
    expr val = expression();
    std::string msg = std::format("ParseError: Expected a newline on line {}", prev.line);
    if (!isAtEnd()) {
        consume(NEW_LINE, std::format("ParseError: Expected a newline on line {}", prev.line));
    }   
    return stmt { printStmt { std::make_unique<expr>(std::move(val)) } };
}

struct stmt Parser::expressionStatement() {
    Token prev = previous();
    expr val = expression();
    std::string msg = std::format("ParseError: Expected a newline on line {}", prev.line);
    if (!isAtEnd()) {
        consume(NEW_LINE, std::format("ParseError: Expected a newline on line {}", prev.line));
    }
    return stmt { exprStmt { std::make_unique<expr>(std::move(val)) } };
}

// Methods for parsing
struct expr Parser::expression() {
    return logicalOr();
}

struct expr Parser::logicalOr() {
    expr expression = logicalAnd();

    while (match({OR})) {
        Token op = previous();
        expr right = logicalAnd();

        expression = expr { binary {
            std::make_unique<expr>(std::move(expression)),
            op,
            std::make_unique<expr>(std::move(right))
        }};
    }

    return expression;
}

struct expr Parser::logicalAnd() {
    expr expression = equality();

    while (match({AND})) {
        Token op = previous();
        expr right = equality();

        expression = expr { binary {
            std::make_unique<expr>(std::move(expression)),
            op,
            std::make_unique<expr>(std::move(right))
        }};
    }

    return expression;
}

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
    if (match({IDENTIFIER})) {
        return expr { varExpr { previous() } };
    }
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