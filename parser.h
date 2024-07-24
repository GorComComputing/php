#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <string>
#include <variant>
#include <vector>


// Базовый класс для AST узлов
class ASTNode {
public:
    virtual ~ASTNode() = default;
};


// Классы для узлов AST
class PrintNode : public ASTNode {
public:
    PrintNode(std::unique_ptr<ASTNode> expr) : expr(std::move(expr)) {}
    std::unique_ptr<ASTNode> expr;
};


class DatabaseQueryNode : public ASTNode {
public:
    DatabaseQueryNode(const std::string& query) : query(query) {}
    std::string query;
};


class HttpRequestAssignmentNode : public ASTNode {
public:
    HttpRequestAssignmentNode(const std::string& var, const std::string& url,
                              const std::string& data, const std::string& header, const std::string& type)
        : variable(var), url(url), data(data), header(header), type(type) {}

    std::string variable;
    std::string url;
    std::string data;
    std::string header;
    std::string type;
};


class TextNode : public ASTNode {
public:
    TextNode(const std::string& text) : text(text) {}
    std::string text;
};


class VariableNode : public ASTNode {
public:
    VariableNode(const std::string& name) : name(name) {}
    std::string name;
};


class ExpressionNode : public ASTNode {
public:
    ExpressionNode(std::unique_ptr<ASTNode> left, const std::string& op, std::unique_ptr<ASTNode> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    std::unique_ptr<ASTNode> left;
    std::string op;
    std::unique_ptr<ASTNode> right;
};


class StringNode : public ASTNode {
public:
    StringNode(const std::string& value) : value(value) {}
    std::string value;
};


class NumberNode : public ASTNode {
public:
    NumberNode(const std::string& value) : value(value) {}
    std::string value;
};


class AssignmentNode : public ASTNode {
public:
    AssignmentNode(std::unique_ptr<VariableNode> variable, std::unique_ptr<ASTNode> expr)
        : variable(std::move(variable)), expr(std::move(expr)) {}

    std::unique_ptr<VariableNode> variable;
    std::unique_ptr<ASTNode> expr;
};


// Определение варианта типа для значений переменных
using VariableValue = std::variant<double, std::string>;


class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), position(0) {}

    std::vector<std::unique_ptr<ASTNode>> parse();
    static void printAST(const std::vector<std::unique_ptr<ASTNode>>& nodes);

private:
    const std::vector<Token>& tokens;
    size_t position;

    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseFactor();
    std::unique_ptr<VariableNode> parseVariable();
    std::unique_ptr<ASTNode> parseHttpRequest();
};

#endif // PARSER_H

