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
    int distance = 0;
};

struct callExpr {
    std::unique_ptr<expr> callee; 
    std::vector<expr> arguments;
};

struct getExpr {
    std::unique_ptr<expr> object; 
    Token name;                   
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
    std::variant<literal, varExpr, callExpr, getExpr, unary, binary, grouping> node;
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
    int distance = 0;
};

struct setStmt {
    std::unique_ptr<struct expr> object;
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
    int distance = 0;
};

struct returnStmt {
    int line = 0;
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
    std::shared_ptr<std::vector<stmt>> body;
};

struct global {
    Token name;
};

struct nonlocal {
    Token name;
};

struct classDef {
    Token className;
    std::vector<stmt> body;
};

struct stmt {
    std::variant<exprStmt, printStmt, varDecl, assignStmt, setStmt, ifStmt, whileStmt, forStmt, returnStmt, funcDef, global, nonlocal, classDef> node;
};