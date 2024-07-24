#include <iostream>
#include "lexer.h"
#include "parser.h"


// Функция для парсинга исходного кода
std::vector<std::unique_ptr<ASTNode>> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> nodes;
    while (position < tokens.size() && tokens[position].type != T_EOF) {
        if (tokens[position].type == T_TEXT) {
            nodes.push_back(std::make_unique<TextNode>(tokens[position].value));
            position++;
        } else if (tokens[position].type == T_ECHO) {
            position++;
            auto expr = parseExpression();
            if (tokens[position].type == T_SEMICOLON) {
                position++;
                nodes.push_back(std::make_unique<PrintNode>(std::move(expr)));
            } else {
                std::cerr << "Expected semicolon after expression" << std::endl;
            }
        } else if (tokens[position].type == T_DB) {
            position++;
            if (tokens[position].type == T_STRING) {
                std::string query = tokens[position].value;
                position++;
                if (tokens[position].type == T_SEMICOLON) {
                    position++;
                    nodes.push_back(std::make_unique<DatabaseQueryNode>(query));
                } else {
                    std::cerr << "Expected semicolon after query" << std::endl;
                }
            } else {
                std::cerr << "Expected query after db" << std::endl;
            }
        } else if (tokens[position].type == T_HTTP) {
            auto httpRequestNode = parseHttpRequest();
            if (httpRequestNode) {
                nodes.push_back(std::move(httpRequestNode));
            }
        } else if (tokens[position].type == T_VARIABLE) {
            auto variable = parseVariable();
            if (tokens[position].type == T_ASSIGN) {
                position++;
                auto expr = parseExpression();
                if (tokens[position].type == T_SEMICOLON) {
                    position++;
                    nodes.push_back(std::make_unique<AssignmentNode>(std::move(variable), std::move(expr)));
                } else {
                    std::cerr << "Expected semicolon after assignment" << std::endl;
                }
            } else {
                std::cerr << "Expected assignment operator after variable" << std::endl;
            }
        } else {
            std::cerr << "Unexpected token: " << tokens[position].value << std::endl;
            position++;
        }
    }
    return nodes;
}


// Функция для печати AST
void Parser::printAST(const std::vector<std::unique_ptr<ASTNode>>& nodes) {
    for (const auto& node : nodes) {
        if (auto printNode = dynamic_cast<PrintNode*>(node.get())) {
            std::cout << "PrintNode: ";
            // Рекурсивный вызов для выражения
            std::vector<std::unique_ptr<ASTNode>> exprNodes;
            exprNodes.push_back(std::move(printNode->expr));
            printAST(exprNodes);
        } else if (auto dbQueryNode = dynamic_cast<DatabaseQueryNode*>(node.get())) {
            std::cout << "DatabaseQueryNode: " << dbQueryNode->query << std::endl;
        } else if (auto httpRequestNode = dynamic_cast<HttpRequestAssignmentNode*>(node.get())) {
            std::cout << "HttpRequestAssignmentNode: Variable = " << httpRequestNode->variable
                      << ", URL = " << httpRequestNode->url
                      << ", Data = " << httpRequestNode->data
                      << ", Header = " << httpRequestNode->header 
                      << ", Type = " << httpRequestNode->type << std::endl;
        } else if (auto textNode = dynamic_cast<TextNode*>(node.get())) {
            std::cout << "TextNode: " << textNode->text << std::endl;
        } else if (auto variableNode = dynamic_cast<VariableNode*>(node.get())) {
            std::cout << "VariableNode: " << variableNode->name << std::endl;
        } else if (auto exprNode = dynamic_cast<ExpressionNode*>(node.get())) {
            std::cout << "ExpressionNode: ";
            std::vector<std::unique_ptr<ASTNode>> exprNodes;
            exprNodes.push_back(std::move(exprNode->left));
            std::cout << " Operator = " << exprNode->op << " ";
            exprNodes.push_back(std::move(exprNode->right));
            printAST(exprNodes);
        } else if (auto stringNode = dynamic_cast<StringNode*>(node.get())) {
            std::cout << "StringNode: " << stringNode->value << std::endl;
        } else if (auto numberNode = dynamic_cast<NumberNode*>(node.get())) {
            std::cout << "NumberNode: " << numberNode->value << std::endl;
        } else if (auto assignmentNode = dynamic_cast<AssignmentNode*>(node.get())) {
            std::cout << "AssignmentNode: Variable = " << assignmentNode->variable->name << " ";
            std::vector<std::unique_ptr<ASTNode>> exprNodes;
            exprNodes.push_back(std::move(assignmentNode->expr));
            printAST(exprNodes);
        } else {
            std::cerr << "Unknown ASTNode type!" << std::endl;
        }
    }
}


// Функция для парсинга выражений
std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseTerm();
    while (position < tokens.size() && (tokens[position].type == T_OPERATOR &&
           (tokens[position].value == "+" || tokens[position].value == "-" || tokens[position].value == "."))) {
        std::string op = tokens[position].value;
        position++;
        auto right = parseTerm();
        left = std::make_unique<ExpressionNode>(std::move(left), op, std::move(right));
    }
    return left;
}


// Функция для парсинга термов
std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();
    while (position < tokens.size() && (tokens[position].type == T_OPERATOR &&
           (tokens[position].value == "*" || tokens[position].value == "/"))) {
        std::string op = tokens[position].value;
        position++;
        auto right = parseFactor();
        left = std::make_unique<ExpressionNode>(std::move(left), op, std::move(right));
    }
    return left;
}


// Функция для парсинга факторов
std::unique_ptr<ASTNode> Parser::parseFactor() {
    if (tokens[position].type == T_NUMBER) {
        auto numberNode = std::make_unique<NumberNode>(tokens[position].value);
        position++;
        return numberNode;
    } else if (tokens[position].type == T_VARIABLE) {
        auto variableNode = std::make_unique<VariableNode>(tokens[position].value);
        position++;
        return variableNode;
    } else if (tokens[position].type == T_STRING) {
        auto stringNode = std::make_unique<StringNode>(tokens[position].value);
        position++;
        return stringNode;
    } else if (tokens[position].type == T_LPAREN) {
        position++;
        auto expr = parseExpression();
        if (tokens[position].type == T_RPAREN) {
            position++;
        } else {
            std::cerr << "Expected closing parenthesis" << std::endl;
        }
        return expr;
    } else {
        std::cerr << "Unexpected token: " << tokens[position].value << std::endl;
        position++;
        return nullptr;
    }
}


// Функция для парсинга переменных
std::unique_ptr<VariableNode> Parser::parseVariable() {
    if (tokens[position].type == T_VARIABLE) {
        auto variableNode = std::make_unique<VariableNode>(tokens[position].value);
        position++;
        return variableNode;
    }
    return nullptr;
}


// Функция для парсинга HTTP запросов
std::unique_ptr<ASTNode> Parser::parseHttpRequest() {
    if (tokens[position].type == T_HTTP) {
        position++;
        if (tokens[position].type == T_LPAREN) {
            position++;
            std::string varName, url, data, header, type;

            if (tokens[position].type == T_VARIABLE) {
                varName = tokens[position].value;
                position++;
            } else {
                std::cerr << "Expected variable for assignment" << std::endl;
            }

            if (tokens[position].type == T_COMMA) {
                position++;
                if (tokens[position].type == T_STRING) {
                    url = tokens[position].value;
                    position++;
                } else {
                    std::cerr << "Expected URL" << std::endl;
                }

                    if (tokens[position].type == T_COMMA) {
                        position++;
                        if (tokens[position].type == T_STRING) {
                            data = tokens[position].value;
                            position++;
                        } else {
                            std::cerr << "Expected data" << std::endl;
                        }

                        if (tokens[position].type == T_COMMA) {
                            position++;
                            if (tokens[position].type == T_STRING) {
                                header = tokens[position].value;
                                position++;
                            } else {
                                std::cerr << "Expected header" << std::endl;
                            }
                            
                            if (tokens[position].type == T_COMMA) {
                            	position++;
                            	if (tokens[position].type == T_STRING) {
                                	type = tokens[position].value;
                                	position++;
                            	} else {
                                	std::cerr << "Expected type" << std::endl;
                            	}
                        	}
                        }
                    }

                    if (tokens[position].type == T_RPAREN) {
                        position++;
                        if (tokens[position].type == T_SEMICOLON) {
                            position++;
                            return std::make_unique<HttpRequestAssignmentNode>(varName, url, data, header, type);
                        } else {
                            std::cerr << "Expected semicolon after HTTP request" << std::endl;
                        }
                    } else {
                        std::cerr << "Expected closing parenthesis" << std::endl;
                    }
            } else {
                std::cerr << "Expected comma after URL" << std::endl;
            }
        } else {
            std::cerr << "Expected opening parenthesis" << std::endl;
        }
    }
    return nullptr;
}

