#pragma once 

#include <memory>
#include "lexer.hpp"

struct expr;
struct stmt;

struct literal {
    Token token;
    std::variant<int, bool, std::string_view, std::monostate> val;
};

struct varExpr {
    Token name;
};

struct callExpr {
    Token name; 
    std::vector<expr> arguments;
};

struct unary {
    Token op;
    std::unique_ptr<expr> right;
};

struct binary {
    std::unique_ptr<expr> left;
    Token op;
    std::unique_ptr<expr> right;   
};

struct grouping {
    std::unique_ptr<expr> expression;
};

struct expr {
    std::variant<literal, varExpr, callExpr, unary, binary, grouping> node;
};

struct exprStmt {
    std::unique_ptr<struct expr> expression;
};
struct printStmt {  
    std::unique_ptr<struct expr> expression;
};

struct varDecl {
    TokenType type;
    Token identifier;
    std::unique_ptr<struct expr> expression;
};

struct assignStmt {
    Token name;
    std::unique_ptr<struct expr> value;
};

struct ifStmt {
    std::unique_ptr<expr> condition;
    std::vector<stmt> ifBranch;
    std::vector<stmt> elseBranch;
};

struct whileStmt {
    std::unique_ptr<expr> condition;
    std::vector<stmt> body;
};

struct forStmt {
    Token loopVar;
    std::unique_ptr<expr> iterable;
    std::vector<stmt> body;
};

struct returnStmt {
    std::unique_ptr<expr> expression;
};

struct param {
    Token name;
    TokenType type; 
};

struct funcDef {
    Token name;
    std::vector<param> params;
    TokenType returnType;
    std::vector<stmt> body;
};
struct stmt {
    std::variant<exprStmt, printStmt, varDecl, assignStmt, ifStmt, whileStmt, forStmt, returnStmt, funcDef> node;
};