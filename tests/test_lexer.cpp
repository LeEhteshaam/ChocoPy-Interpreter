#include "../src/lexer.hpp"
#include <cassert>
#include <iostream>

// --- MakeToken Unit Tests ---

void test_make_token_map() {
    for (const auto& [lexeme, expectedType] : tokenMap) {
        int start = 0;
        int column = 0;
        Token t = makeToken(lexeme, &start, lexeme.length(), &column, 1);
        assert(t.type == expectedType);
        assert(t.lexeme == lexeme);
    }
    std::cout << "  test_make_token_map passed!\n";
}

void test_make_token_literals() {
    // Test INT
    {
        std::string_view code = "456";
        int start = 0;
        int column = 0;
        Token t = makeToken(code, &start, 3, &column, 1);
        assert(t.type == INT);
        assert(std::get<int>(t.literal) == 456);
    }

    // Test STR
    {
        std::string_view code = "\"hello\"";
        int start = 0;
        int column = 0;
        Token t = makeToken(code, &start, 7, &column, 1);
        assert(t.type == STR);
        assert(std::get<std::string_view>(t.literal) == "\"hello\"");
    }

    // Test custom IDENTIFIER
    {
        std::string_view code = "my_var";
        int start = 0;
        int column = 0;
        Token t = makeToken(code, &start, 6, &column, 1);
        assert(t.type == IDENTIFIER);
        assert(t.lexeme == "my_var");
    }
    std::cout << "  test_make_token_literals passed!\n";
}

// --- Tokenizer Unit Tests ---

void test_tokenizer_simple_expr() {
    std::string_view code = "x = 42\nprint(x)\n";
    std::vector<Token> tokens = tokenizer(code);
    
    // Expecting: IDENTIFIER (x), ASSIGN (=), INT (42), NEW_LINE, 
    //            PRINT (print), LEFT_PAREN, IDENTIFIER (x), RIGHT_PAREN, NEW_LINE, EOF
    assert(tokens.size() == 10);
    assert(tokens[0].type == IDENTIFIER && tokens[0].lexeme == "x");
    assert(tokens[1].type == ASSIGN);
    assert(tokens[2].type == INT && std::get<int>(tokens[2].literal) == 42);
    assert(tokens[3].type == NEW_LINE);
    assert(tokens[4].type == PRINT);
    assert(tokens[5].type == LEFT_PAREN);
    assert(tokens[6].type == IDENTIFIER && tokens[6].lexeme == "x");
    assert(tokens[7].type == RIGHT_PAREN);
    assert(tokens[8].type == NEW_LINE);
    assert(tokens[9].type == END_OF_FILE);
    std::cout << "  test_tokenizer_simple_expr passed!\n";
}

void test_tokenizer_nested_indentation() {
    std::string_view code = 
        "if True:\n"
        "    if False:\n"
        "        pass\n";
    std::vector<Token> tokens = tokenizer(code);
    
    // Expecting: IF, TRUE, COLON, NEW_LINE,
    //            INDENT, IF, FALSE, COLON, NEW_LINE,
    //            INDENT, PASS, NEW_LINE,
    //            DEDENT, DEDENT, END_OF_FILE
    assert(tokens.size() == 15);
    assert(tokens[0].type == IF && tokens[0].lexeme == "if");
    assert(tokens[1].type == TRUE && tokens[1].lexeme == "True");
    assert(tokens[2].type == COLON && tokens[2].lexeme == ":");
    assert(tokens[3].type == NEW_LINE && tokens[3].lexeme == "\n");
    assert(tokens[4].type == INDENT);
    assert(tokens[5].type == IF && tokens[5].lexeme == "if");
    assert(tokens[6].type == FALSE && tokens[6].lexeme == "False");
    assert(tokens[7].type == COLON && tokens[7].lexeme == ":");
    assert(tokens[8].type == NEW_LINE && tokens[8].lexeme == "\n");
    assert(tokens[9].type == INDENT);
    assert(tokens[10].type == PASS && tokens[10].lexeme == "pass");
    assert(tokens[11].type == NEW_LINE && tokens[11].lexeme == "\n");
    assert(tokens[12].type == DEDENT);
    assert(tokens[13].type == DEDENT);
    assert(tokens[14].type == END_OF_FILE);
    std::cout << "  test_tokenizer_nested_indentation passed!\n";
}

void test_tokenizer_comments_and_blanks() {
    std::string_view code = 
        "# comment\n"
        "x = 1\n"
        "  \n"
        "# comment 2\n"
        "y = 2\n";
    std::vector<Token> tokens = tokenizer(code);
    
    // Expecting: IDENTIFIER (x), ASSIGN (=), INT (1), NEW_LINE,
    //            IDENTIFIER (y), ASSIGN (=), INT (2), NEW_LINE, EOF
    assert(tokens.size() == 9);
    assert(tokens[0].type == IDENTIFIER && tokens[0].lexeme == "x");
    assert(tokens[1].type == ASSIGN);
    assert(tokens[2].type == INT && std::get<int>(tokens[2].literal) == 1);
    assert(tokens[3].type == NEW_LINE);
    assert(tokens[4].type == IDENTIFIER && tokens[4].lexeme == "y");
    assert(tokens[5].type == ASSIGN);
    assert(tokens[6].type == INT && std::get<int>(tokens[6].literal) == 2);
    assert(tokens[7].type == NEW_LINE);
    assert(tokens[8].type == END_OF_FILE);
    std::cout << "  test_tokenizer_comments_and_blanks passed!\n";
}

void test_tokenizer_unterminated_string() {
    std::string_view code = "x = \"hello";
    bool exceptionThrown = false;
    try {
        std::vector<Token> tokens = tokenizer(code);
    } catch (const std::runtime_error& e) {
        exceptionThrown = true;
        std::string msg = e.what();
        assert(msg.find("unterminated string literal") != std::string::npos);
    }
    assert(exceptionThrown);
    std::cout << "  test_tokenizer_unterminated_string passed!\n";
}

void test_tokenizer_bad_indentation() {
    std::string_view code = 
        "if True:\n"
        "    pass\n"
        "  pass\n"; // Indentation level 2 doesn't match 0 or 4
    bool exceptionThrown = false;
    try {
        std::vector<Token> tokens = tokenizer(code);
    } catch (const std::runtime_error& e) {
        exceptionThrown = true;
        std::string msg = e.what();
        assert(msg.find("unindent does not match any outer indentation level") != std::string::npos);
    }
    assert(exceptionThrown);
    std::cout << "  test_tokenizer_bad_indentation passed!\n";
}

void test_tokenizer_single_slash() {
    std::string_view code = "x = 2 / 5\n";
    bool exceptionThrown = false;
    try {
        std::vector<Token> tokens = tokenizer(code);
    } catch (const std::runtime_error& e) {
        exceptionThrown = true;
        std::string msg = e.what();
        assert(msg.find("Single '/' is not supported") != std::string::npos);
    }
    assert(exceptionThrown);
    std::cout << "  test_tokenizer_single_slash passed!\n";
}

int main() {
    std::cout << "Running MakeToken unit tests...\n";
    test_make_token_map();
    test_make_token_literals();

    std::cout << "Running Tokenizer unit tests...\n";
    test_tokenizer_simple_expr();
    test_tokenizer_nested_indentation();
    test_tokenizer_comments_and_blanks();
    test_tokenizer_unterminated_string();
    test_tokenizer_bad_indentation();
    test_tokenizer_single_slash();

    std::cout << "All unit tests passed successfully!\n";
    return 0;
}
