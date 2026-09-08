#include "resolver.hpp"
#include <map>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

int Resolver::resolveLocal(Token name) {
    const auto& curScope = scopes.back();

    if (!curScope.contains(std::string(name.lexeme))) {
        errors.push_back(std::format("ResolveError: Name {} is not defined on line {}", name.lexeme, name.line));
    }

    return 0;
}

int Resolver::resolveNonlocal(Token name) {
    int distance = 1;
    bool found = false;

    for (int i = scopes.size() - 2; i >= 0; i--, distance ++) {
        const auto& curScope = scopes[i];
        if (curScope.contains(std::string(name.lexeme))) {
            found = true;
            break;
        }
    }

    if (!found) {
        errors.push_back(std::format("ResolveError: Name {} is not defined on line {}", name.lexeme, name.line));
    }
    return distance;
}

int Resolver::resolveGlobal(Token name) {
    int distance = scopes.size() - 1;

    if (!scopes[0].contains(std::string(name.lexeme))) {
        errors.push_back(std::format("ResolveError: Name {} is not defined on line {}", name.lexeme, name.line));
    }
    
    return distance;
}

int Resolver::resolveRead(Token name) {
    int distance = 0;
    bool found = false;

    for (int i = scopes.size() - 1; i >= 0; i--, distance ++) {
        const auto& curScope = scopes[i];
        if (curScope.contains(std::string(name.lexeme))) {
            found = true;
            break;
        }
    }

    if (!found) {
        errors.push_back(std::format("ResolveError: Name {} is not defined on line {}", name.lexeme, name.line));
    }
    return distance;
}

int Resolver::getDistance(Token name) {
    const auto& curScope = scopes.back();
    if (curScope.at(std::string(name.lexeme)) == varScopes::LOCAL) {
        return resolveLocal(name);
    } else if (curScope.at(std::string(name.lexeme)) == varScopes::NONLOCAL) {
        return resolveNonlocal(name);
    } else {
        return resolveGlobal(name);
    } 
}

void Resolver::traverseExpr(expr& expression) {
    std::visit(overloaded {
        [&](literal& l) { return; },
        [&](varExpr& v) {
            if (scopes.back().contains(std::string(v.name.lexeme))) {
                v.distance = getDistance(v.name);
            } else {
                v.distance = resolveRead(v.name);
            }
        },
        [&](callExpr& c) {
            traverseExpr(*(c.callee));
            for (auto& arg : c.arguments) {
                traverseExpr(arg);
            }
        },
        [&](getExpr& g) {
            traverseExpr(*(g.object));
        },
        [&](unary& u) { traverseExpr(*(u.right)); },
        [&](binary& b) {
            traverseExpr(*(b.left));
            traverseExpr(*(b.right));
        },
        [&](grouping& g) { traverseExpr(*(g.expression)); }
    }, expression.node); 
}

void Resolver::traverseStmt(stmt& statement) {
    std::visit(overloaded {
        [&](exprStmt& e) { traverseExpr(*(e.expression)); },
        [&](printStmt& p) { traverseExpr(*(p.expression)); },
        [&](varDecl& v) {
            std::string id = std::string(v.identifier.lexeme);

            if (state != states::CLASS) {
                if (scopes.back().contains(id)) {
                    errors.push_back(std::format("ResolveError: Already '{}' defined on line {}", id, v.identifier.line));
                } else {
                    scopes.back()[id] = varScopes::LOCAL;
                }
            }
            traverseExpr(*(v.expression));
        },
        [&](assignStmt &a) {
            if (scopes.back().contains(std::string(a.name.lexeme))) {
                a.distance = getDistance(a.name);
            } else {
                errors.push_back(std::format("ResolveError: Var {} is assigned before defined on line {}", a.name.lexeme, a.name.line));
            }
            traverseExpr(*(a.value));
        },
        [&](ifStmt& i) {
            traverseExpr(*(i.condition));
            for (auto& s : i.ifBranch) traverseStmt(s);
            for (auto& s : i.elseBranch) traverseStmt(s);
        },
        [&](whileStmt& w) {
            traverseExpr(*(w.condition));
            for (auto& s : w.body) traverseStmt(s);
        },
        [&](forStmt& f) {
            traverseExpr(*(f.iterable));
            std::string id = std::string(f.loopVar.lexeme);
            if (scopes.back().contains(id)) {
                f.distance = getDistance(f.loopVar); 
            } else {
                errors.push_back(std::format("ResolveError: Loop variable '{}' is not defined in current scope on line {}", id, f.loopVar.line));
            }
            for (auto& s : f.body) {
                traverseStmt(s);
            }
        },
        [&](returnStmt& r) {
            if (state != states::FUNCTION && state != states::METHOD) {
                errors.push_back(std::format("ResolveError: Use of 'return' outside a function on line {}", r.line));
            }
            if (r.expression) {
                traverseExpr(*(r.expression));
            }
        },
        [&](funcDef& f) {
            std::string funcName = std::string(f.name.lexeme);
            
            if (state != states::CLASS) {
                if (scopes.back().contains(funcName)) {
                    errors.push_back(std::format("ResolveError: Function '{}' already defined on line {}", f.name.lexeme, f.name.line));
                } else {
                    scopes.back()[funcName] = varScopes::LOCAL;
                }
            }

            scopes.push_back(std::unordered_map<std::string, varScopes>());
            states prevState = state;

            if (prevState == states::CLASS) {
                state = states::METHOD;
                if (f.params.empty() || f.params[0].name.lexeme != "self") {
                    errors.push_back(std::format("ResolveError: Method '{}' must have 'self' as first parameter on line {}", f.name.lexeme, f.name.line));
                }
            } else {
                state = states::FUNCTION;
            }

            for (const auto& param : f.params) {
                std::string paramName = std::string(param.name.lexeme);
                if (scopes.back().contains(paramName)) {
                    errors.push_back(std::format("ResolveError: Duplicate parameter '{}' on line {}", param.name.lexeme, param.name.line));
                } else {
                    scopes.back()[paramName] = varScopes::LOCAL;
                }
            }

            for (auto& stmt : *(f.body)) {
                traverseStmt(stmt);
            }

            state = prevState;
            scopes.pop_back();
        },
        [&](classDef& c) {
            std::string name = std::string(c.className.lexeme);
            states prevState = state;
            state = states::CLASS;

            scopes.back()[name] = varScopes::LOCAL;

            for (auto& stmt: c.body) {
                traverseStmt(stmt);
            }
            state = prevState;
        },
        [&](setStmt& s) {
            traverseExpr(*(s.value));
            traverseExpr(*(s.object));
        },
        [&](global& g) { scopes.back()[std::string(g.name.lexeme)] = varScopes::GLOBAL; },
        [&](nonlocal& n) { scopes.back()[std::string(n.name.lexeme)] = varScopes::NONLOCAL; }
    }, statement.node);
}

void Resolver::resolve(std::vector<stmt>& statements) {

    scopes.push_back(std::unordered_map<std::string, varScopes>());

    for (auto& stmt: statements) {
        traverseStmt(stmt);
    }

    scopes.pop_back();
    
    if (!errors.empty()) {
        std::string final_error_message = "File contained syntax errors:\n";
        for (const std::string& err : errors) {
            final_error_message += err + "\n";
        }
        
        throw std::runtime_error(final_error_message);
    }
}
