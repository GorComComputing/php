#ifndef INTERPRET_H
#define INTERPRET_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <variant>
#include <iostream>

#include "lexer.h"
#include "parser.h"


// Interpreter class
class Interpreter {
public:
    void interpret(const std::vector<std::unique_ptr<ASTNode>>& nodes);

private:
    std::string exec(const char* cmd);
    std::string evaluateExpression(ASTNode* node);

    std::unordered_map<std::string, VariableValue> variables;
};

#endif // INTERPRET_H

