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

    // Test INT_TYPE ("int")
    {
        std::string_view code = "int";
        int start = 0;
        int column = 0;
        Token t = makeToken(code, &start, 3, &column, 1);
        assert(t.type == INT_TYPE);
        assert(t.lexeme == "int");
    }

    // Test STR_TYPE ("str")
    {
        std::string_view code = "str";
        int start = 0;
        int column = 0;
        Token t = makeToken(code, &start, 3, &column, 1);
        assert(t.type == STR_TYPE);
        assert(t.lexeme == "str");
    }

    // Test BOOL_TYPE ("bool")
    {
        std::string_view code = "bool";
        int start = 0;
        int column = 0;
        Token t = makeToken(code, &start, 4, &column, 1);
        assert(t.type == BOOL_TYPE);
        assert(t.lexeme == "bool");
    }
    std::cout << "  test_make_token_literals passed!\n";
}

// --- Tokenizer Unit Tests ---

void test_tokenizer_var_def() {
    // Test bool:b = True (compact syntax)
    {
        std::string_view code = "bool:b = True\n";
        std::vector<Token> tokens = tokenizer(code);
        
        // Expecting: BOOL_TYPE (bool), COLON (:), IDENTIFIER (b), ASSIGN (=), TRUE (True), NEW_LINE, EOF
        assert(tokens.size() == 7);
        assert(tokens[0].type == BOOL_TYPE && tokens[0].lexeme == "bool");
        assert(tokens[1].type == COLON && tokens[1].lexeme == ":");
        assert(tokens[2].type == IDENTIFIER && tokens[2].lexeme == "b");
        assert(tokens[3].type == ASSIGN && tokens[3].lexeme == "=");
        assert(tokens[4].type == TRUE && tokens[4].lexeme == "True");
        assert(tokens[5].type == NEW_LINE && tokens[5].lexeme == "\n");
        assert(tokens[6].type == END_OF_FILE);
    }

    // Test bool: b = False (with spaces)
    {
        std::string_view code = "bool: b = False\n";
        std::vector<Token> tokens = tokenizer(code);
        
        // Expecting: BOOL_TYPE (bool), COLON (:), IDENTIFIER (b), ASSIGN (=), FALSE (False), NEW_LINE, EOF
        assert(tokens.size() == 7);
        assert(tokens[0].type == BOOL_TYPE && tokens[0].lexeme == "bool");
        assert(tokens[1].type == COLON && tokens[1].lexeme == ":");
        assert(tokens[2].type == IDENTIFIER && tokens[2].lexeme == "b");
        assert(tokens[3].type == ASSIGN && tokens[3].lexeme == "=");
        assert(tokens[4].type == FALSE && tokens[4].lexeme == "False");
        assert(tokens[5].type == NEW_LINE && tokens[5].lexeme == "\n");
        assert(tokens[6].type == END_OF_FILE);
    }

    // Test int:i = 0 (compact syntax)
    {
        std::string_view code = "int:i = 0\n";
        std::vector<Token> tokens = tokenizer(code);
        
        // Expecting: INT_TYPE (int), COLON (:), IDENTIFIER (i), ASSIGN (=), INT (0), NEW_LINE, EOF
        assert(tokens.size() == 7);
        assert(tokens[0].type == INT_TYPE && tokens[0].lexeme == "int");
        assert(tokens[1].type == COLON && tokens[1].lexeme == ":");
        assert(tokens[2].type == IDENTIFIER && tokens[2].lexeme == "i");
        assert(tokens[3].type == ASSIGN && tokens[3].lexeme == "=");
        assert(tokens[4].type == INT && tokens[4].lexeme == "0" && std::get<int>(tokens[4].literal) == 0);
        assert(tokens[5].type == NEW_LINE && tokens[5].lexeme == "\n");
        assert(tokens[6].type == END_OF_FILE);
    }

    // Test int: i = 0 (with spaces)
    {
        std::string_view code = "int: i = 0\n";
        std::vector<Token> tokens = tokenizer(code);
        
        // Expecting: INT_TYPE (int), COLON (:), IDENTIFIER (i), ASSIGN (=), INT (0), NEW_LINE, EOF
        assert(tokens.size() == 7);
        assert(tokens[0].type == INT_TYPE && tokens[0].lexeme == "int");
        assert(tokens[1].type == COLON && tokens[1].lexeme == ":");
        assert(tokens[2].type == IDENTIFIER && tokens[2].lexeme == "i");
        assert(tokens[3].type == ASSIGN && tokens[3].lexeme == "=");
        assert(tokens[4].type == INT && tokens[4].lexeme == "0" && std::get<int>(tokens[4].literal) == 0);
        assert(tokens[5].type == NEW_LINE && tokens[5].lexeme == "\n");
        assert(tokens[6].type == END_OF_FILE);
    }

    // Test str: s = "hello"
    {
        std::string_view code = "str: s = \"hello\"\n";
        std::vector<Token> tokens = tokenizer(code);
        
        // Expecting: STR_TYPE (str), COLON (:), IDENTIFIER (s), ASSIGN (=), STR ("hello"), NEW_LINE, EOF
        assert(tokens.size() == 7);
        assert(tokens[0].type == STR_TYPE && tokens[0].lexeme == "str");
        assert(tokens[1].type == COLON && tokens[1].lexeme == ":");
        assert(tokens[2].type == IDENTIFIER && tokens[2].lexeme == "s");
        assert(tokens[3].type == ASSIGN && tokens[3].lexeme == "=");
        assert(tokens[4].type == STR && tokens[4].lexeme == "\"hello\"" && std::get<std::string_view>(tokens[4].literal) == "\"hello\"");
        assert(tokens[5].type == NEW_LINE && tokens[5].lexeme == "\n");
        assert(tokens[6].type == END_OF_FILE);
    }

    // Test standard ChocoPy syntax: b: bool = True
    {
        std::string_view code = "b: bool = True\n";
        std::vector<Token> tokens = tokenizer(code);
        
        // Expecting: IDENTIFIER (b), COLON (:), BOOL_TYPE (bool), ASSIGN (=), TRUE (True), NEW_LINE, EOF
        assert(tokens.size() == 7);
        assert(tokens[0].type == IDENTIFIER && tokens[0].lexeme == "b");
        assert(tokens[1].type == COLON && tokens[1].lexeme == ":");
        assert(tokens[2].type == BOOL_TYPE && tokens[2].lexeme == "bool");
        assert(tokens[3].type == ASSIGN && tokens[3].lexeme == "=");
        assert(tokens[4].type == TRUE && tokens[4].lexeme == "True");
        assert(tokens[5].type == NEW_LINE && tokens[5].lexeme == "\n");
        assert(tokens[6].type == END_OF_FILE);
    }

    // Test standard ChocoPy syntax: i: int = 0
    {
        std::string_view code = "i: int = 0\n";
        std::vector<Token> tokens = tokenizer(code);
        
        // Expecting: IDENTIFIER (i), COLON (:), INT_TYPE (int), ASSIGN (=), INT (0), NEW_LINE, EOF
        assert(tokens.size() == 7);
        assert(tokens[0].type == IDENTIFIER && tokens[0].lexeme == "i");
        assert(tokens[1].type == COLON && tokens[1].lexeme == ":");
        assert(tokens[2].type == INT_TYPE && tokens[2].lexeme == "int");
        assert(tokens[3].type == ASSIGN && tokens[3].lexeme == "=");
        assert(tokens[4].type == INT && tokens[4].lexeme == "0" && std::get<int>(tokens[4].literal) == 0);
        assert(tokens[5].type == NEW_LINE && tokens[5].lexeme == "\n");
        assert(tokens[6].type == END_OF_FILE);
    }

    // Test standard ChocoPy syntax: s: str = "hello"
    {
        std::string_view code = "s: str = \"hello\"\n";
        std::vector<Token> tokens = tokenizer(code);
        
        // Expecting: IDENTIFIER (s), COLON (:), STR_TYPE (str), ASSIGN (=), STR ("hello"), NEW_LINE, EOF
        assert(tokens.size() == 7);
        assert(tokens[0].type == IDENTIFIER && tokens[0].lexeme == "s");
        assert(tokens[1].type == COLON && tokens[1].lexeme == ":");
        assert(tokens[2].type == STR_TYPE && tokens[2].lexeme == "str");
        assert(tokens[3].type == ASSIGN && tokens[3].lexeme == "=");
        assert(tokens[4].type == STR && tokens[4].lexeme == "\"hello\"" && std::get<std::string_view>(tokens[4].literal) == "\"hello\"");
        assert(tokens[5].type == NEW_LINE && tokens[5].lexeme == "\n");
        assert(tokens[6].type == END_OF_FILE);
    }
    std::cout << "  test_tokenizer_var_def passed!\n";
}

void test_tokenizer_simple_expr() {
    std::string_view code = "int: x = 42\nprint(x)\n";
    std::vector<Token> tokens = tokenizer(code);
    
    // Expecting: INT_TYPE (int), COLON (:), IDENTIFIER (x), ASSIGN (=), INT (42), NEW_LINE, 
    //            PRINT (print), LEFT_PAREN, IDENTIFIER (x), RIGHT_PAREN, NEW_LINE, EOF
    assert(tokens.size() == 12);
    assert(tokens[0].type == INT_TYPE && tokens[0].lexeme == "int");
    assert(tokens[1].type == COLON && tokens[1].lexeme == ":");
    assert(tokens[2].type == IDENTIFIER && tokens[2].lexeme == "x");
    assert(tokens[3].type == ASSIGN && tokens[3].lexeme == "=");
    assert(tokens[4].type == INT && std::get<int>(tokens[4].literal) == 42);
    assert(tokens[5].type == NEW_LINE && tokens[5].lexeme == "\n");
    assert(tokens[6].type == PRINT && tokens[6].lexeme == "print");
    assert(tokens[7].type == LEFT_PAREN && tokens[7].lexeme == "(");
    assert(tokens[8].type == IDENTIFIER && tokens[8].lexeme == "x");
    assert(tokens[9].type == RIGHT_PAREN && tokens[9].lexeme == ")");
    assert(tokens[10].type == NEW_LINE && tokens[10].lexeme == "\n");
    assert(tokens[11].type == END_OF_FILE);
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
        "int: x = 1\n"
        "  \n"
        "# comment 2\n"
        "int: y = 2\n";
    std::vector<Token> tokens = tokenizer(code);
    
    // Expecting: INT_TYPE (int), COLON (:), IDENTIFIER (x), ASSIGN (=), INT (1), NEW_LINE,
    //            INT_TYPE (int), COLON (:), IDENTIFIER (y), ASSIGN (=), INT (2), NEW_LINE, EOF
    assert(tokens.size() == 13);
    assert(tokens[0].type == INT_TYPE && tokens[0].lexeme == "int");
    assert(tokens[1].type == COLON && tokens[1].lexeme == ":");
    assert(tokens[2].type == IDENTIFIER && tokens[2].lexeme == "x");
    assert(tokens[3].type == ASSIGN && tokens[3].lexeme == "=");
    assert(tokens[4].type == INT && std::get<int>(tokens[4].literal) == 1);
    assert(tokens[5].type == NEW_LINE && tokens[5].lexeme == "\n");
    assert(tokens[6].type == INT_TYPE && tokens[6].lexeme == "int");
    assert(tokens[7].type == COLON && tokens[7].lexeme == ":");
    assert(tokens[8].type == IDENTIFIER && tokens[8].lexeme == "y");
    assert(tokens[9].type == ASSIGN && tokens[9].lexeme == "=");
    assert(tokens[10].type == INT && std::get<int>(tokens[10].literal) == 2);
    assert(tokens[11].type == NEW_LINE && tokens[11].lexeme == "\n");
    assert(tokens[12].type == END_OF_FILE);
    std::cout << "  test_tokenizer_comments_and_blanks passed!\n";
}

void test_tokenizer_unterminated_string() {
    std::string_view code = "str: x = \"hello";
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
    std::string_view code = "int: x = 2 / 5\n";
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

void test_tokenizer_arrow() {
    // Arrow in function return type annotation
    {
        std::string_view code = "def foo() -> int:\n    pass\n";
        std::vector<Token> tokens = tokenizer(code);
        assert(tokens.size() == 13);
        assert(tokens[0].type == DEF);
        assert(tokens[1].type == IDENTIFIER && tokens[1].lexeme == "foo");
        assert(tokens[2].type == LEFT_PAREN);
        assert(tokens[3].type == RIGHT_PAREN);
        assert(tokens[4].type == ARROW && tokens[4].lexeme == "->");
        assert(tokens[5].type == INT_TYPE && tokens[5].lexeme == "int");
        assert(tokens[6].type == COLON);
        assert(tokens[7].type == NEW_LINE);
        assert(tokens[8].type == INDENT);
        assert(tokens[9].type == PASS);
        assert(tokens[10].type == NEW_LINE);
        assert(tokens[11].type == DEDENT);
        assert(tokens[12].type == END_OF_FILE);
    }

    // Minus followed by something other than '>'
    {
        std::string_view code = "x - 5\n";
        std::vector<Token> tokens = tokenizer(code);
        assert(tokens.size() == 5);
        assert(tokens[0].type == IDENTIFIER && tokens[0].lexeme == "x");
        assert(tokens[1].type == MINUS && tokens[1].lexeme == "-");
        assert(tokens[2].type == INT && std::get<int>(tokens[2].literal) == 5);
        assert(tokens[3].type == NEW_LINE);
        assert(tokens[4].type == END_OF_FILE);
    }

    // Trailing minus at EOF
    {
        std::string_view code = "-";
        std::vector<Token> tokens = tokenizer(code);
        assert(tokens.size() == 2);
        assert(tokens[0].type == MINUS && tokens[0].lexeme == "-");
        assert(tokens[1].type == END_OF_FILE);
    }

    std::cout << "  test_tokenizer_arrow passed!\n";
}

void test_tokenizer_for_in_loop() {
    // Test 'in' keyword in expressions
    {
        std::string_view code = "x in [1, 2]\n";
        std::vector<Token> tokens = tokenizer(code);
        assert(tokens.size() == 9);
        assert(tokens[0].type == IDENTIFIER && tokens[0].lexeme == "x");
        assert(tokens[1].type == IN && tokens[1].lexeme == "in");
        assert(tokens[2].type == LEFT_BRAC && tokens[2].lexeme == "[");
        assert(tokens[3].type == INT && std::get<int>(tokens[3].literal) == 1);
        assert(tokens[4].type == COMMA && tokens[4].lexeme == ",");
        assert(tokens[5].type == INT && std::get<int>(tokens[5].literal) == 2);
        assert(tokens[6].type == RIGHT_BRAC && tokens[6].lexeme == "]");
        assert(tokens[7].type == NEW_LINE && tokens[7].lexeme == "\n");
        assert(tokens[8].type == END_OF_FILE);
    }

    // Test identifiers containing 'in' as prefix/substring/suffix to ensure no false match
    {
        std::string_view code = "inside bin win_in_game\n";
        std::vector<Token> tokens = tokenizer(code);
        assert(tokens.size() == 5);
        assert(tokens[0].type == IDENTIFIER && tokens[0].lexeme == "inside");
        assert(tokens[1].type == IDENTIFIER && tokens[1].lexeme == "bin");
        assert(tokens[2].type == IDENTIFIER && tokens[2].lexeme == "win_in_game");
        assert(tokens[3].type == NEW_LINE && tokens[3].lexeme == "\n");
        assert(tokens[4].type == END_OF_FILE);
    }

    // Test for-in loop with string literal:
    // char: str = ""
    // for char in "chocopy":
    //     print(char)
    {
        std::string_view code = 
            "char: str = \"\"\n"
            "for char in \"chocopy\":\n"
            "    print(char)\n";
        std::vector<Token> tokens = tokenizer(code);

        // Expecting:
        // Line 1: IDENTIFIER (char), COLON (:), STR_TYPE (str), ASSIGN (=), STR (""), NEW_LINE
        // Line 2: FOR (for), IDENTIFIER (char), IN (in), STR ("chocopy"), COLON (:), NEW_LINE
        // Line 3: INDENT, PRINT (print), LEFT_PAREN ((), IDENTIFIER (char), RIGHT_PAREN ()), NEW_LINE, DEDENT, EOF
        assert(tokens.size() == 20);

        // Line 1
        assert(tokens[0].type == IDENTIFIER && tokens[0].lexeme == "char");
        assert(tokens[1].type == COLON && tokens[1].lexeme == ":");
        assert(tokens[2].type == STR_TYPE && tokens[2].lexeme == "str");
        assert(tokens[3].type == ASSIGN && tokens[3].lexeme == "=");
        assert(tokens[4].type == STR && tokens[4].lexeme == "\"\"" && std::get<std::string_view>(tokens[4].literal) == "\"\"");
        assert(tokens[5].type == NEW_LINE && tokens[5].lexeme == "\n");

        // Line 2
        assert(tokens[6].type == FOR && tokens[6].lexeme == "for");
        assert(tokens[7].type == IDENTIFIER && tokens[7].lexeme == "char");
        assert(tokens[8].type == IN && tokens[8].lexeme == "in");
        assert(tokens[9].type == STR && tokens[9].lexeme == "\"chocopy\"" && std::get<std::string_view>(tokens[9].literal) == "\"chocopy\"");
        assert(tokens[10].type == COLON && tokens[10].lexeme == ":");
        assert(tokens[11].type == NEW_LINE && tokens[11].lexeme == "\n");

        // Line 3
        assert(tokens[12].type == INDENT);
        assert(tokens[13].type == PRINT && tokens[13].lexeme == "print");
        assert(tokens[14].type == LEFT_PAREN && tokens[14].lexeme == "(");
        assert(tokens[15].type == IDENTIFIER && tokens[15].lexeme == "char");
        assert(tokens[16].type == RIGHT_PAREN && tokens[16].lexeme == ")");
        assert(tokens[17].type == NEW_LINE && tokens[17].lexeme == "\n");
        assert(tokens[18].type == DEDENT);
        assert(tokens[19].type == END_OF_FILE);
    }

    std::cout << "  test_tokenizer_for_in_loop passed!\n";
}

void run_lexer_tests() {
    std::cout << "Running MakeToken unit tests...\n";
    test_make_token_map();
    test_make_token_literals();

    std::cout << "Running Tokenizer unit tests...\n";
    test_tokenizer_var_def();
    test_tokenizer_arrow();
    test_tokenizer_simple_expr();
    test_tokenizer_nested_indentation();
    test_tokenizer_comments_and_blanks();
    test_tokenizer_unterminated_string();
    test_tokenizer_bad_indentation();
    test_tokenizer_single_slash();
    test_tokenizer_for_in_loop();
}
