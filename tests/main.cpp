#include <iostream>

// Declarations of test functions from other files
void run_lexer_tests();
void run_ast_printer_tests();
void run_parser_tests();

int main() {
    std::cout << "=======================================\n";
    std::cout << "Starting ChocoPy Compiler Test Suite\n";
    std::cout << "=======================================\n\n";

    run_lexer_tests();
    std::cout << "\n";
    
    run_ast_printer_tests();
    std::cout << "\n";

    run_parser_tests();
    std::cout << "\n";

    std::cout << "=======================================\n";
    std::cout << "All tests passed successfully!\n";
    std::cout << "=======================================\n";
    return 0;
}
