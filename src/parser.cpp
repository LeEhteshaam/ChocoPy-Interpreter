#include <ast.hpp>
#include <utility>
#include <initializer_list>

class Parser { 
    private:
        std::vector<Token> tokens;
        int counter = 0;

        // Helper functions for traversing token stream

        Token previous() {
            return tokens[counter - 1];
        }

        Token peek() {
            return tokens[counter];
        }

        bool isAtEnd() {
            Token cur  = peek();
            return cur.type == END_OF_FILE;
        }

        Token advance() {
            if (!isAtEnd()) {
                counter += 1;
            }
            return previous();
        }

        bool check(TokenType type) {
            if (isAtEnd()) {
                return false;
            }

            Token cur = peek();
            return cur.type == type;
        }

        bool match(std::initializer_list<TokenType> types) {
            for (const auto& type: types) {
                if (check(type)) {
                    advance();
                    return true;
                }
            }
            return false;
        }

        struct expr expression() {
            return equality();
        }

        // Methods for parsing
        struct expr equality() {
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

        struct expr comparison() {
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

        struct expr term() {
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

        struct expr factor() {
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

        struct expr unary() {
            while (match({NOT, MINUS})) {
                Token op = previous();
                expr right = unary();
                struct unary node = {
                    op, 
                    std::make_unique<expr>(std::move(right))
                 };
                return expr { std::move(node) } ;
            }

            // If we did not found a unary, it must be a primary
            return primary();
        }

        struct expr primary() {
            
            if (match({FALSE})) return expr { literal { previous(), false } };
            if (match({TRUE})) return expr { literal { previous(), true } }; 
            if (match({NONE})) return expr { literal { previous(), std::monostate{} } };
            if (match({INT, STR})) {
                Token res = previous();
                return expr { literal { res, res.literal } };
            }

            // consume left paren
            if (match({LEFT_PAREN})) {
                expr inside = expression();

                // make sure you consume a right paren 
                match({RIGHT_PAREN});
                return expr { grouping { std::make_unique<expr>(std::move(inside)) } };
            }
        }

    public: 

        explicit Parser(std::vector<Token> tokens) {
            this->tokens = std::move(tokens);
        }
};