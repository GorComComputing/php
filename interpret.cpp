#include <stdexcept>
#include <array>
#include <memory>
#include <cstdio>
#include <cstdlib>

#include "interpret.h"


void Interpreter::interpret(const std::vector<std::unique_ptr<ASTNode>>& nodes) {
    for (const auto& node : nodes) {
        if (auto textNode = dynamic_cast<TextNode*>(node.get())) {
            std::cout << textNode->text;
        } else if (auto printNode = dynamic_cast<PrintNode*>(node.get())) {
            std::cout << evaluateExpression(printNode->expr.get()) << std::endl;
        } else if (auto queryNode = dynamic_cast<DatabaseQueryNode*>(node.get())) {
            std::string result = exec(("echo \"Database query: " + queryNode->query + "\"").c_str());
            std::cout << result << std::endl;
        } else if (auto httpRequestNode = dynamic_cast<HttpRequestAssignmentNode*>(node.get())) {
            std::string url = httpRequestNode->url;
            std::string data = httpRequestNode->data;
            std::string header = httpRequestNode->header;

            std::string command = "curl -X GET ";
            command += "--data \"" + data + "\" ";
            command += "-H \"" + header + "\" ";
            command += url;

            std::string result = exec(command.c_str());
            variables[httpRequestNode->variable] = result;
        } else if (auto assignmentNode = dynamic_cast<AssignmentNode*>(node.get())) {
            std::string varName = assignmentNode->variable->name;
            std::string valueStr = evaluateExpression(assignmentNode->expr.get());
            try {
                double value = std::stod(valueStr);
                variables[varName] = value;
            } catch (const std::invalid_argument&) {
                variables[varName] = valueStr;
            } catch (const std::out_of_range&) {
                std::cerr << "Number out of range for variable assignment: " << valueStr << std::endl;
            }
        }
    }
}

std::string Interpreter::exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string Interpreter::evaluateExpression(ASTNode* node) {
    if (auto textNode = dynamic_cast<TextNode*>(node)) {
        return textNode->text;
    } else if (auto variableNode = dynamic_cast<VariableNode*>(node)) {
        auto it = variables.find(variableNode->name);
        if (it != variables.end()) {
            if (auto value = std::get_if<double>(&it->second)) {
                return std::to_string(*value);
            } else if (auto value = std::get_if<std::string>(&it->second)) {
                return *value;
            }
        } else {
            std::cerr << "Undefined variable: " << variableNode->name << std::endl;
            return "0";
        }
    } else if (auto expressionNode = dynamic_cast<ExpressionNode*>(node)) {
        std::string leftValue = evaluateExpression(expressionNode->left.get());
        std::string rightValue = evaluateExpression(expressionNode->right.get());
        if (expressionNode->op == "+") {
            return std::to_string(std::stod(leftValue) + std::stod(rightValue));
        }
        if (expressionNode->op == "-") {
            return std::to_string(std::stod(leftValue) - std::stod(rightValue));
        }
        if (expressionNode->op == "*") {
            return std::to_string(std::stod(leftValue) * std::stod(rightValue));
        }
        if (expressionNode->op == "/") {
            return std::to_string(std::stod(leftValue) / std::stod(rightValue));
        }
        if (expressionNode->op == ".") {
            return leftValue + rightValue;
        }
    } else if (auto stringNode = dynamic_cast<StringNode*>(node)) {
        return stringNode->value.substr(1, stringNode->value.length() - 2); // Remove quotes
    } else if (auto numberNode = dynamic_cast<NumberNode*>(node)) {
        return numberNode->value;
    }
    return "0";
}

