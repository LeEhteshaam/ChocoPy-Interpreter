#include <ast.hpp>
#include <utility>
#include <initializer_list>
#include <format>
#include <ranges>

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

        // Panic mode error recovery
        void synchronize() {
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

        Token consume(TokenType type, std::string message) {
            if (check(type)) {
                return advance();
            }
            
            // add the error    
            errors.push_back(std::move(message));
            throw(ParseError());
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

            Token prev = previous();
            // consume left paren
            if (match({LEFT_PAREN})) {
                expr inside = expression();

                // Verify that we have a right paren, else add and error + synchronize
                Token prev = previous();
                std::string msg =  std::format("ParseError: Expected a ')' at line {}", prev.line);
                consume(RIGHT_PAREN, msg);
                return expr { grouping { std::make_unique<expr>(std::move(inside)) } };
            }

            // not valid syntax, return something but synchronize and add error message
            std::string msg = std::format("ParseError: Expected a int/bool on line {}", prev.line);
            errors.push_back(msg);
            throw(ParseError());
            
        }

    public: 

        explicit Parser(std::vector<Token> tokens) {
            this->tokens = std::move(tokens);
        }

        std::vector<expr> parse() {
            std::vector<expr> ast_nodes;

            while (!isAtEnd()) {
                try {
                    ast_nodes.push_back(expression());
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
};