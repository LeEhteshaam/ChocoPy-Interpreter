/*
The Lexer takes in a sequence of chars and makes tokens.
*/

#include <string>
#include <iostream>
#include <string_view>
#include <variant>
#include <unordered_map>
#include <format>

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

// Check if the lexeme is a keyword, delimiter, line structure or operator 
std::unordered_map<std::string_view, TokenType> tokenMap {
    {"\n", NEW_LINE},
    {"str", IDENTIFIER},
    {"int", IDENTIFIER},
    {"+", ADD},
    {"-", MINUS},
    {"//", INT_DIVIDE},
    {"*", MULTIPLY},
    {"%", MODULO},
    {"=", ASSIGN},
    {"==", EQUAL},
    {"!=", NOT_EQUAL},
    {"<", LESS},
    {"<=", LESS_EQUAL},
    {">", GREATER},
    {">=", GREATER_EQUAL},
    {"def", DEF},
    {"class", CLASS},
    {"True", TRUE},
    {"False", FALSE},
    {"if", IF},
    {"elif", ELIF},
    {"else", ELSE},
    {"return", RETURN},
    {"None", NONE},
    {"not", NOT},
    {"print", PRINT},
    {"and", AND},
    {"or", OR},
    {"is", IS},
    {"global", GLOBAL},
    {"nonlocal", NONLOCAL},
    {"pass", PASS},
    {"for", FOR},
    {"while", WHILE},
    {"(", LEFT_PAREN},
    {")", RIGHT_PAREN},
    {"[", LEFT_BRAC},
    {"]", RIGHT_BRAC},
    {":", COLON},
    {"->", ARROW},
    {",", COMMA},
    {".", DOT}
};
struct Token {
    TokenType type;
    int line;
    int column;
    std::string_view lexeme;
    size_t tokenLength;
    std::variant<int, std::string_view, std::monostate> literal;
};

bool isAlpha(char ch) {
    return std::isalnum(ch) || ch == '_';
}

// Advances our pointer and our column 
void advance(int *start, int* column, int length) {
    *column += length;
    *start += length;
}

// Given a sequence of chars creates a token
struct Token makeToken(std::string_view p, int *start, int length, int *column, int line) {

    std::string_view lexeme = p.substr(*start, length);
    struct Token res; 
    int cur = *start;

    // check if it is a number
    if (std::isdigit(p[cur])) {
        int acc = 0;

        for (int i = 0; i < length; i++) {
            acc *= 10;
            int val = p[cur + i] - '0';
            acc += val;
        }

        res.type = INT;
        res.literal = acc;

    } else if (p[cur] == '"') {

        res.type = STR;
        res.literal = lexeme;

    } else if (tokenMap.contains(lexeme)) {

        TokenType type = tokenMap[lexeme];
        res.type = type;
        res.literal = std::monostate{};

    } else {

        // Must be an identifier (ie. var name)
        res.type = IDENTIFIER;
        res.literal = std::monostate{};

    }

    res.line = line;
    res.column = *column;
    res.lexeme = lexeme; 
    res.tokenLength =  length;

    advance(start, column, length);

    return res;
}

// Given buffer string, returns me the next token
struct Token nextToken(std::string_view buffer, int* cur, int* line, int* column) {

    // return EOF token
    if (*cur >= buffer.size()) {
        struct Token res = {END_OF_FILE, *line, *column, "", 1, std::monostate{}};
        return res;
    }

    char curChar;
    int start = *cur;

    // How many chars have we seen
    int counter = 0;

    switch (buffer[start]) {
        
        case '\n': {
            counter += 1;
            struct Token res = makeToken(buffer, cur, counter, column, *line);
            *line += 1;
            *column = 0;
            return res;
        }

        case '+': case '*': case '%': case ')': case '(': 
        case '-': case ',': case '.': case ':': case ']':
        case '[':
            counter += 1;
            return makeToken(buffer, cur, counter, column, *line);

        case '=': case '!': case '>': case '<':
            // check if the next char is an equal sign
            if (start + 1 < buffer.size() && buffer[start + 1] == '=') {
                counter += 2;
            } else {
                counter += 1;
            }
            return makeToken(buffer, cur, counter, column, *line);
        
        case '/':

            if (start + 1 < buffer.size() && buffer[start + 1] == '/') { 
                counter += 2;
                return makeToken(buffer, cur, counter, column, *line);
            } else {
                throw std::runtime_error(std::format("LexicalError: Single '/' is not supported (detected at line {})", *line));
            }

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            //get the number
            while (start + counter < buffer.length() && (std::isdigit(buffer[start + counter]))) {
                counter += 1;
            }
            return makeToken(buffer, cur, counter, column, *line);

        // comment, not tokenized. Iterate until new line
        case '#':
            counter += 1;

            while (start + counter < buffer.size()) {
                curChar = buffer[start + counter];
                
                if (curChar == '\n') {
                    break;
                }

                counter += 1;
            }

            advance(cur, column, counter);
            return nextToken(buffer, cur, line, column);
        
        // Must be a string
        case '"': {
            counter += 1;
            int originalLine =  *line;

            while (true) {

                if (start + counter >= buffer.length()) {
                    throw std::runtime_error(std::format("LexicalError: unterminated string literal (detected at line {})", originalLine));                    
                }

                char curChar = buffer[start + counter];
                
                if (curChar == '"') {
                    break;
                }

                if (curChar == '\\') {
                    counter += 2;
                    continue;
                }

                if (curChar == '\n') {
                    *line += 1;
                }

                // just a normal char 
                counter += 1;

            }

            return makeToken(buffer, cur, counter, column, originalLine);
        }

        // Must be a keyword 
        default:
            // iterate until delimtier 
            while (start + counter < buffer.size() && isAlpha(buffer[start + counter])) {
                counter += 1;
            }

            return makeToken(buffer, cur, counter, column, *line);
    } 
}