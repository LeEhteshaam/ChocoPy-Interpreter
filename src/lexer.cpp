/*
The Lexer takes in a sequence of chars and makes tokens.
*/

#include "lexer.hpp"
#include <string>
#include <iostream>
#include <string_view>
#include <variant>
#include <unordered_map>
#include <format>

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

        case ' ': case '\t':
            while (start + counter < buffer.length() && 
                  (buffer[start + counter] == ' ' || buffer[start + counter] == '\t')) {
                counter += 1;
            }
            advance(cur, column, counter);
            return nextToken(buffer, cur, line, column);
        
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
                    throw std::runtime_error(std::format("LexicalError: string literal cannot span multiple lines (detected at line {})", *line));
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

std::vector<Token> tokenizer(std::string_view buffer) {
    std::vector<Token> tokens;
    std::vector<int> indentStack = {0};

    int line = 1; 
    int column = 0;
    int cur = 0;

    int lastLine = 0;
    int peekCur;
    int indentLevel = 0;
    bool isBlank = false;

    // iterate until we see a EOF token
    while (true) {

        // are we on a new line, if so we need to track indentation level
        if (lastLine != line) {
            
            // get identation level
            peekCur = cur;
            isBlank = false;

            
            while (peekCur < buffer.size() && (buffer[peekCur] == ' ' || buffer[peekCur] == '\t')) {
                if (buffer[peekCur] == ' ') {
                    indentLevel += 1;
                } else {
                    indentLevel += 8 - (indentLevel % 8);
                }
                peekCur += 1;
            }

            // make sure that the line is not a new line or a comment
            if (peekCur < buffer.size() && (buffer[peekCur] == '\n' || buffer[peekCur] == '#')) {

                // This is a blank line
                isBlank = true;

            }
            
            else {

                // add a indent token
                if (indentLevel > indentStack.back()) {
                    indentStack.push_back(indentLevel);
                    Token indentToken = {INDENT, line, 0, "", 0, std::monostate{}};
                    tokens.push_back(indentToken);
                }

                // add dedent tokens
                else if (indentLevel < indentStack.back()) { 

                    while (indentLevel < indentStack.back()) {
                        indentStack.pop_back();
                        Token dedentToken = {DEDENT, line, 0, "", 0, std::monostate{}};
                        tokens.push_back(dedentToken);
                    }
                }

                if (indentLevel != indentStack.back()) {
                    throw std::runtime_error(std::format("IndentationError: unindent does not match any outer indentation level (detected at line {})", line));
                }
            }

            column = indentLevel;
            cur = peekCur;
            lastLine = line;
            indentLevel = 0; 
        }


        struct Token next = nextToken(buffer, &cur, &line, &column);

        // We need to add dedent tokens 
        if (next.type == END_OF_FILE) {
            while (indentStack.back() != 0) {
                indentStack.pop_back();
                Token dedentToken = {DEDENT, line, 0, "", 0, std::monostate{}};
                tokens.push_back(dedentToken);
            }

            tokens.push_back(next);
            break;
        }

        else if (next.type == NEW_LINE) {
            // Only add if its not a blank line 
            if (!isBlank) {
                tokens.push_back(next);
            }
        }
        else {
            tokens.push_back(next);
        }
    } 

    return tokens;

}