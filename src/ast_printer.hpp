#pragma once

#include "ast.hpp"
#include <iostream>
#include <ostream>

void printAST(const expr& expression, std::ostream& out = std::cout);