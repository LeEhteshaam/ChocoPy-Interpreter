#include <iostream>

void run_lexer_tests();
void run_ast_printer_tests();
void run_parser_tests();
void run_resolver_tests();
void run_interpreter_tests();

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

    run_resolver_tests();
    std::cout << "\n";

    run_interpreter_tests();
    std::cout << "\n";

    std::cout << "=======================================\n";
    std::cout << "All tests passed successfully!\n";
    std::cout << "=======================================\n";
    return 0;
}
