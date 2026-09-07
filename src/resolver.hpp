#pragma once
#include "parser.hpp"
#include <unordered_map>
#include <string>
#include <vector>

enum class varScopes {
    GLOBAL, 
    NONLOCAL, 
    LOCAL
};

enum class states {
    FUNCTION,
    NONE
};

class Resolver {
    private:
        std::vector<std::string> errors;
        states state = states::NONE;
        std::vector<std::unordered_map<std::string, varScopes>> scopes;
        int resolveLocal(Token name);
        int resolveNonlocal(Token name);
        int resolveGlobal(Token name);
        int resolveRead(Token name);
        int getDistance(Token name);
        void traverseExpr(expr& expression);
        void traverseStmt(stmt& stmt);

    public:
        void resolve(std::vector<stmt>& statements);
};