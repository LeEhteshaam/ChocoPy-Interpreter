#include "../src/resolver.hpp"
#include "../src/parser.hpp"
#include "../src/lexer.hpp"
#include "../src/ast.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>

// Helper to parse source code into AST
static std::vector<stmt> parse_code(std::string_view code) {
    std::vector<Token> tokens = tokenizer(code);
    Parser parser(tokens);
    return parser.parse();
}

// Helper to resolve code and return AST (throws on error)
static std::vector<stmt> resolve_code(std::string_view code) {
    std::vector<stmt> ast = parse_code(code);
    Resolver resolver;
    resolver.resolve(ast);
    return ast;
}

// Helper to check if resolve_code throws runtime_error containing expected substrings
static void assert_resolve_error(std::string_view code, const std::vector<std::string>& expectedSubstrings) {
    bool caught = false;
    try {
        resolve_code(code);
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg = e.what();
        for (const auto& sub : expectedSubstrings) {
            assert(msg.find(sub) != std::string::npos);
        }
    }
    assert(caught);
}

// --- Literal & Basic Expressions Resolution Tests ---

void test_resolver_literals_and_expressions() {
    // Literals, grouping, unary, and binary expressions
    {
        std::string_view code = 
            "print 1 + 2 * (3 - 4) // 1 % 2\n"
            "print not True == (False != True)\n"
            "print -42\n"
            "print \"hello\" + \"world\"\n";
        auto ast = resolve_code(code);
        assert(ast.size() == 4);
    }

    std::cout << "  test_resolver_literals_and_expressions passed!\n";
}

// --- Local Variable Resolution Tests ---

void test_resolver_local_variables() {
    // Variable declaration, reading, and assigning at distance 0
    {
        std::string_view code = 
            "x: int = 10\n"
            "y: int = 0\n"
            "y = x + 5\n"
            "x = y * 2\n"
            "print x\n";
        auto ast = resolve_code(code);
        assert(ast.size() == 5);

        // Check y = x + 5 (x has distance 0)
        const auto& assignY = std::get<assignStmt>(ast[2].node);
        assert(assignY.distance == 0);
        const auto& addExpr = std::get<binary>(assignY.value->node);
        const auto& varX = std::get<varExpr>(addExpr.left->node);
        assert(varX.distance == 0);

        // Check x = y * 2 (x has distance 0, y has distance 0)
        const auto& assignX = std::get<assignStmt>(ast[3].node);
        assert(assignX.distance == 0);
        const auto& multExpr = std::get<binary>(assignX.value->node);
        const auto& varY = std::get<varExpr>(multExpr.left->node);
        assert(varY.distance == 0);
    }

    std::cout << "  test_resolver_local_variables passed!\n";
}

// --- Nested Function & Lexical Closures Distance Tests ---

void test_resolver_nested_function_distances() {
    // Multi-level nested function variable distances
    {
        std::string_view code = 
            "g: int = 1\n"
            "def outer(a: int) -> int:\n"
            "    def middle(b: int) -> int:\n"
            "        def inner(c: int) -> int:\n"
            "            return g + a + b + c\n"
            "        return inner(3)\n"
            "    return middle(2)\n"
            "print outer(1)\n";
        auto ast = resolve_code(code);
        assert(ast.size() == 3);

        // Traverse down to inner function's return statement: g + a + b + c
        const auto& outerFn = std::get<funcDef>(ast[1].node);
        const auto& middleFn = std::get<funcDef>((*outerFn.body)[0].node);
        const auto& innerFn = std::get<funcDef>((*middleFn.body)[0].node);
        const auto& retStmt = std::get<returnStmt>((*innerFn.body)[0].node);
        
        // (((g + a) + b) + c)
        const auto& b1 = std::get<binary>(retStmt.expression->node);
        const auto& varC = std::get<varExpr>(b1.right->node);
        assert(varC.distance == 0); // c is local to inner (distance 0)

        const auto& b2 = std::get<binary>(b1.left->node);
        const auto& varB = std::get<varExpr>(b2.right->node);
        assert(varB.distance == 1); // b is in middle (distance 1)

        const auto& b3 = std::get<binary>(b2.left->node);
        const auto& varA = std::get<varExpr>(b3.right->node);
        assert(varA.distance == 2); // a is in outer (distance 2)

        const auto& varG = std::get<varExpr>(b3.left->node);
        assert(varG.distance == 3); // g is in global scope (distance 3)
    }

    // Function calls distances
    {
        std::string_view code = 
            "def f() -> int:\n"
            "    return 42\n"
            "def g() -> int:\n"
            "    def local_fn() -> int:\n"
            "        return f()\n"
            "    return local_fn()\n";
        auto ast = resolve_code(code);
        assert(ast.size() == 2);

        const auto& gFn = std::get<funcDef>(ast[1].node);
        const auto& localFn = std::get<funcDef>((*gFn.body)[0].node);
        const auto& retInsideLocal = std::get<returnStmt>((*localFn.body)[0].node);
        const auto& callF = std::get<callExpr>(retInsideLocal.expression->node);
        assert(callF.distance == 2); // f is at global scope (2 hops up from local_fn)

        const auto& retInsideG = std::get<returnStmt>((*gFn.body)[1].node);
        const auto& callLocal = std::get<callExpr>(retInsideG.expression->node);
        assert(callLocal.distance == 0); // local_fn is in g's scope (0 hops)
    }

    std::cout << "  test_resolver_nested_function_distances passed!\n";
}

// --- Nonlocal & Global Keyword Resolution Tests ---

void test_resolver_nonlocal_and_global_keywords() {
    // Nonlocal variable resolution and assignment distance
    {
        std::string_view code = 
            "def outer(x: int) -> int:\n"
            "    def inner() -> int:\n"
            "        nonlocal x\n"
            "        x = 42\n"
            "        return x\n"
            "    return inner()\n";
        auto ast = resolve_code(code);
        assert(ast.size() == 1);

        const auto& outerFn = std::get<funcDef>(ast[0].node);
        const auto& innerFn = std::get<funcDef>((*outerFn.body)[0].node);
        
        // assignStmt x = 42 in inner
        const auto& assignX = std::get<assignStmt>((*innerFn.body)[1].node);
        assert(assignX.distance == 1); // x is in outer (distance 1)

        // return x in inner
        const auto& retX = std::get<returnStmt>((*innerFn.body)[2].node);
        const auto& varX = std::get<varExpr>(retX.expression->node);
        assert(varX.distance == 1); // x is in outer (distance 1)
    }

    // Global variable resolution and assignment distance
    {
        std::string_view code = 
            "val: int = 100\n"
            "def level1():\n"
            "    def level2():\n"
            "        global val\n"
            "        val = 200\n"
            "        print val\n";
        auto ast = resolve_code(code);
        assert(ast.size() == 2);

        const auto& level1Fn = std::get<funcDef>(ast[1].node);
        const auto& level2Fn = std::get<funcDef>((*level1Fn.body)[0].node);

        // assignStmt val = 200 in level2
        const auto& assignVal = std::get<assignStmt>((*level2Fn.body)[1].node);
        assert(assignVal.distance == 2); // val is in global (distance 2 from level2)

        // print val in level2
        const auto& printVal = std::get<printStmt>((*level2Fn.body)[2].node);
        const auto& varVal = std::get<varExpr>(printVal.expression->node);
        assert(varVal.distance == 2); // val is in global (distance 2 from level2)
    }

    std::cout << "  test_resolver_nonlocal_and_global_keywords passed!\n";
}

// --- Shadowing Tests ---

void test_resolver_shadowing() {
    // Parameter shadowing global variable
    {
        std::string_view code = 
            "x: int = 100\n"
            "def f(x: int) -> int:\n"
            "    return x\n"
            "print f(10)\n"
            "print x\n";
        auto ast = resolve_code(code);
        assert(ast.size() == 4);

        const auto& fFn = std::get<funcDef>(ast[1].node);
        const auto& retX = std::get<returnStmt>((*fFn.body)[0].node);
        const auto& paramX = std::get<varExpr>(retX.expression->node);
        assert(paramX.distance == 0); // x refers to parameter (distance 0)

        const auto& printX = std::get<printStmt>(ast[3].node);
        const auto& globalX = std::get<varExpr>(printX.expression->node);
        assert(globalX.distance == 0); // global x from global scope (distance 0)
    }

    std::cout << "  test_resolver_shadowing passed!\n";
}

// --- Control Flow Statements Resolution Tests ---

void test_resolver_control_flow() {
    // If statement and elif/else branches
    {
        std::string_view code = 
            "x: int = 5\n"
            "if x > 0:\n"
            "    print x\n"
            "else:\n"
            "    x = 0\n";
        auto ast = resolve_code(code);
        assert(ast.size() == 2);

        const auto& ifS = std::get<ifStmt>(ast[1].node);
        const auto& cond = std::get<binary>(ifS.condition->node);
        assert(std::get<varExpr>(cond.left->node).distance == 0);
        
        const auto& printS = std::get<printStmt>(ifS.ifBranch[0].node);
        assert(std::get<varExpr>(printS.expression->node).distance == 0);

        const auto& assignS = std::get<assignStmt>(ifS.elseBranch[0].node);
        assert(assignS.distance == 0);
    }

    // While statement
    {
        std::string_view code = 
            "i: int = 0\n"
            "while i < 10:\n"
            "    i = i + 1\n";
        auto ast = resolve_code(code);
        assert(ast.size() == 2);

        const auto& whileS = std::get<whileStmt>(ast[1].node);
        const auto& cond = std::get<binary>(whileS.condition->node);
        assert(std::get<varExpr>(cond.left->node).distance == 0);

        const auto& assignI = std::get<assignStmt>(whileS.body[0].node);
        assert(assignI.distance == 0);
    }

    // For statement with local loop variable
    {
        std::string_view code = 
            "s: str = \"abc\"\n"
            "ch: str = \"\"\n"
            "for ch in s:\n"
            "    print ch\n";
        auto ast = resolve_code(code);
        assert(ast.size() == 3);

        const auto& forS = std::get<forStmt>(ast[2].node);
        assert(forS.distance == 0);
        assert(std::get<varExpr>(forS.iterable->node).distance == 0);
        const auto& printCh = std::get<printStmt>(forS.body[0].node);
        assert(std::get<varExpr>(printCh.expression->node).distance == 0);
    }

    // For statement with nonlocal loop variable inside nested function
    {
        std::string_view code = 
            "def process(s: str, c: str):\n"
            "    def sub():\n"
            "        nonlocal c\n"
            "        for c in s:\n"
            "            print c\n";
        auto ast = resolve_code(code);
        assert(ast.size() == 1);

        const auto& procFn = std::get<funcDef>(ast[0].node);
        const auto& subFn = std::get<funcDef>((*procFn.body)[0].node);
        const auto& forS = std::get<forStmt>((*subFn.body)[1].node);
        assert(forS.distance == 1); // c is nonlocal at distance 1
        assert(std::get<varExpr>(forS.iterable->node).distance == 1); // s is param of process at distance 1
    }

    std::cout << "  test_resolver_control_flow passed!\n";
}

// --- Error Branches Unit Tests ---

void test_resolver_error_undefined_variable() {
    // Read undefined variable
    assert_resolve_error("print unknown_var\n", 
        {"ResolveError: Name unknown_var is not defined on line 1"});

    // Call undefined function
    assert_resolve_error("undefined_func(1, 2)\n", 
        {"ResolveError: Name undefined_func is not defined on line 1"});
    std::cout << "  test_resolver_error_undefined_variable passed!\n";
}

void test_resolver_error_assign_before_defined() {
    // Assignment before definition
    assert_resolve_error("x = 10\n", 
        {"ResolveError: Var x is assigned before defined on line 1"});
    std::cout << "  test_resolver_error_assign_before_defined passed!\n";
}

void test_resolver_error_duplicate_variable() {
    // Duplicate variable declaration in same scope
    assert_resolve_error(
        "x: int = 1\n"
        "x: int = 2\n",
        {"ResolveError: Already 'x' defined on line 2"});
    std::cout << "  test_resolver_error_duplicate_variable passed!\n";
}

void test_resolver_error_duplicate_function() {
    // Duplicate function definition in same scope
    assert_resolve_error(
        "def foo() -> int:\n"
        "    return 1\n"
        "def foo() -> int:\n"
        "    return 2\n",
        {"ResolveError: Function 'foo' already defined on line 3"});
    std::cout << "  test_resolver_error_duplicate_function passed!\n";
}

void test_resolver_error_duplicate_parameters() {
    // Duplicate parameter names in function signature
    assert_resolve_error(
        "def add(x: int, x: int) -> int:\n"
        "    return x\n",
        {"ResolveError: Duplicate parameter 'x' on line 1"});
    std::cout << "  test_resolver_error_duplicate_parameters passed!\n";
}

void test_resolver_error_return_outside_function() {
    // Return at top-level global scope
    assert_resolve_error("return 42\n", 
        {"ResolveError: Use of 'return' outside a function on line 1"});

    assert_resolve_error("return \"outside\"\n", 
        {"ResolveError: Use of 'return' outside a function on line 1"});
    std::cout << "  test_resolver_error_return_outside_function passed!\n";
}

void test_resolver_error_undefined_global_and_nonlocal() {
    // global x when x was never defined at global scope
    assert_resolve_error(
        "def test():\n"
        "    global not_in_global\n"
        "    not_in_global = 1\n",
        {"ResolveError: Name not_in_global is not defined on line 3"});

    // nonlocal x when x is not in any enclosing scope
    assert_resolve_error(
        "def test():\n"
        "    nonlocal not_in_parent\n"
        "    not_in_parent = 1\n",
        {"ResolveError: Name not_in_parent is not defined on line 3"});
    std::cout << "  test_resolver_error_undefined_global_and_nonlocal passed!\n";
}

void test_resolver_error_undefined_loop_var() {
    // For loop variable not defined in current scope
    assert_resolve_error(
        "s: str = \"hello\"\n"
        "for ch in s:\n"
        "    print ch\n",
        {"ResolveError: Loop variable 'ch' is not defined in current scope on line 2"});
    std::cout << "  test_resolver_error_undefined_loop_var passed!\n";
}

void test_resolver_error_multiple_errors() {
    // Multiple resolve errors accumulated into one message
    std::string_view code = 
        "return 1\n"
        "print undefined_a + undefined_b\n"
        "z = 100\n";
    assert_resolve_error(code, {
        "File contained syntax errors:",
        "ResolveError: Use of 'return' outside a function on line 1",
        "ResolveError: Name undefined_a is not defined on line 2",
        "ResolveError: Name undefined_b is not defined on line 2",
        "ResolveError: Var z is assigned before defined on line 3"
    });
    std::cout << "  test_resolver_error_multiple_errors passed!\n";
}

// --- Master Runner Function ---

void run_resolver_tests() {
    std::cout << "Running Resolver unit tests...\n";
    test_resolver_literals_and_expressions();
    test_resolver_local_variables();
    test_resolver_nested_function_distances();
    test_resolver_nonlocal_and_global_keywords();
    test_resolver_shadowing();
    test_resolver_control_flow();

    // Error branch tests
    test_resolver_error_undefined_variable();
    test_resolver_error_assign_before_defined();
    test_resolver_error_duplicate_variable();
    test_resolver_error_duplicate_function();
    test_resolver_error_duplicate_parameters();
    test_resolver_error_return_outside_function();
    test_resolver_error_undefined_global_and_nonlocal();
    test_resolver_error_undefined_loop_var();
    test_resolver_error_multiple_errors();
}
