#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <fstream>
#include <stdexcept>
#include <array>
#include <unordered_map>
#include <variant>

// Enumeration of token types
enum TokenType {
    T_EOF,
    T_OPEN_TAG,
    T_CLOSE_TAG,
    T_PRINT,
    T_STRING,
    T_SEMICOLON,
    T_DB,
    T_TEXT,
    T_VARIABLE,
    T_OPERATOR,
    T_ASSIGN,
    T_NUMBER,
    T_LPAREN,   // Added for left parenthesis '('
    T_RPAREN    // Added for right parenthesis ')'
};

// Token structure
struct Token {
    TokenType type;
    std::string value;
};

// Tokenizer class
class Tokenizer {
public:
    Tokenizer(const std::string& source) : source(source), position(0), insidePHP(false) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (position < source.length()) {
            if (match("<\\?php\\b")) {
                if (insidePHP) {
                    std::cerr << "Unexpected <?php tag without closing ?>" << std::endl;
                    return tokens;
                }
                if (position > 0 && source[position - 1] == '<') {
                    tokens.push_back({T_TEXT, "<"});
                }
                insidePHP = true;
                tokens.push_back({T_OPEN_TAG, "<?php"});
            } else if (match("\\?>")) {
                if (!insidePHP) {
                    std::cerr << "Unexpected ?> tag without opening <?php" << std::endl;
                    return tokens;
                }
                insidePHP = false;
                tokens.push_back({T_CLOSE_TAG, "?>"});
            } else if (insidePHP) {
                if (match("//")) { 
                	skipComment();
                //} else if (match("/*")) {
                //	std::cout << "/*COM";
                //	position += 2;
                //	skipCommentMultilene();
                } else if (match("echo")) {
                    tokens.push_back({T_PRINT, "echo"});
                } else if (match("db")) {
                    tokens.push_back({T_DB, "db"});
                } else if (match("\\$[a-zA-Z_][a-zA-Z0-9_]*")) {
                    tokens.push_back({T_VARIABLE, lastMatch});
                } else if (match("\\d+(\\.\\d+)?")) {
                    tokens.push_back({T_NUMBER, lastMatch});
                } else if (match("[\\+\\-\\*/\\.]")) {
                    tokens.push_back({T_OPERATOR, lastMatch});
                } else if (match("=")) {
                    tokens.push_back({T_ASSIGN, "="});
                } else if (match("\"[^\"]*\"")) {
                    tokens.push_back({T_STRING, lastMatch});
                } else if (match(";")) {
                    tokens.push_back({T_SEMICOLON, ";"});
                } else if (match("\\(")) { // Match left parenthesis
                    tokens.push_back({T_LPAREN, "("});
                } else if (match("\\)")) { // Match right parenthesis
                    tokens.push_back({T_RPAREN, ")"});
                } else {
                    position++;
                }
            } else {
                if (match("<")) {
                    tokens.push_back({T_TEXT, "<"});
                } else if (match("[^<]+")) {
                    tokens.push_back({T_TEXT, lastMatch});
                } else {
                    position++;
                }
            }
        }

        tokens.push_back({T_EOF, ""});
        return tokens;
    }

private:
    bool match(const std::string& pattern) {
        std::regex re(pattern);
        std::smatch match;
        std::string substring = source.substr(position);
        if (std::regex_search(substring, match, re) && match.position() == 0) {
            lastMatch = match.str();
            position += lastMatch.length();
            return true;
        }
        return false;
    }
    
    void skipComment() {
        //std::cout << "//COM";
        while (position < source.length()) {
        	if (source[position] == '\n') {
            	position++; // Skip newline character
                break;
        	}
        	if (source.substr(position, 2) == "?>") {
            	// Add closing tag token
                //tokens.push_back({T_CLOSE_TAG, "?>"});
           		//position += 2; // Skip closing tag characters
                break;
            }
            position++;
        }
    }
    
    void skipCommentMultilene() {
        //std::cout << "//COM";
        while (position < source.length()) {
        	if (source.substr(position, 2) == "*/") {
           		position += 2; // Skip closing tag characters
                break;
            }
        	if (source.substr(position, 2) == "?>") {
            	// Add closing tag token
                //tokens.push_back({T_CLOSE_TAG, "?>"});
           		//position += 2; // Skip closing tag characters
                break;
            }
            position++;
        }
    }

    std::string source;
    size_t position;
    std::string lastMatch;
    bool insidePHP;
};

// Base class for AST nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

// Class for print nodes
class PrintNode : public ASTNode {
public:
    PrintNode(std::unique_ptr<ASTNode> expr) : expr(std::move(expr)) {}
    std::unique_ptr<ASTNode> expr;
};

// Class for database query nodes
class DatabaseQueryNode : public ASTNode {
public:
    DatabaseQueryNode(const std::string& query) : query(query) {}
    std::string query;
};

// Class for text nodes outside PHP tags
class TextNode : public ASTNode {
public:
    TextNode(const std::string& text) : text(text) {}
    std::string text;
};

// Class for variable nodes
class VariableNode : public ASTNode {
public:
    VariableNode(const std::string& name) : name(name) {}
    std::string name;
};

// Class for arithmetic and string expression nodes
class ExpressionNode : public ASTNode {
public:
    ExpressionNode(std::unique_ptr<ASTNode> left, const std::string& op, std::unique_ptr<ASTNode> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    std::unique_ptr<ASTNode> left;
    std::string op;
    std::unique_ptr<ASTNode> right;
};

// Class for string value nodes
class StringNode : public ASTNode {
public:
    StringNode(const std::string& value) : value(value) {}
    std::string value;
};

// Class for number value nodes
class NumberNode : public ASTNode {
public:
    NumberNode(const std::string& value) : value(value) {}
    std::string value;
};

// Class for assignment nodes
class AssignmentNode : public ASTNode {
public:
    AssignmentNode(std::unique_ptr<VariableNode> variable, std::unique_ptr<ASTNode> expr)
        : variable(std::move(variable)), expr(std::move(expr)) {}

    std::unique_ptr<VariableNode> variable;
    std::unique_ptr<ASTNode> expr;
};

// Define a variant type for variable values
using VariableValue = std::variant<double, std::string>;

// Parser class
class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), position(0) {}

    std::vector<std::unique_ptr<ASTNode>> parse() {
        std::vector<std::unique_ptr<ASTNode>> nodes;
        while (position < tokens.size() && tokens[position].type != T_EOF) {
            if (tokens[position].type == T_TEXT) {
                nodes.push_back(std::make_unique<TextNode>(tokens[position].value));
                position++;
            } else if (tokens[position].type == T_PRINT) {
                position++;
                auto expr = parseExpression();
                if (tokens[position].type == T_SEMICOLON) {
                    position++;
                    nodes.push_back(std::make_unique<PrintNode>(std::move(expr)));
                } else {
                    std::cerr << "Expected semicolon after expression" << std::endl;
                }
            } else if (tokens[position].type == T_OPEN_TAG) {
                position++;
            } else if (tokens[position].type == T_CLOSE_TAG) {
                position++;
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

private:
    const std::vector<Token>& tokens;
    size_t position;

    std::unique_ptr<ASTNode> parseExpression() {
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

    std::unique_ptr<ASTNode> parseTerm() {
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

    std::unique_ptr<ASTNode> parseFactor() {
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
        } else if (tokens[position].type == T_LPAREN) { // Check for left parenthesis
            position++;
            auto expr = parseExpression();
            if (tokens[position].type == T_RPAREN) { // Check for right parenthesis
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

    std::unique_ptr<VariableNode> parseVariable() {
        if (tokens[position].type == T_VARIABLE) {
            auto variableNode = std::make_unique<VariableNode>(tokens[position].value);
            position++;
            return variableNode;
        }
        return nullptr;
    }
};

// Interpreter class
class Interpreter {
public:
    void interpret(const std::vector<std::unique_ptr<ASTNode>>& nodes) {
        for (const auto& node : nodes) {
            if (auto textNode = dynamic_cast<TextNode*>(node.get())) {
                std::cout << textNode->text;
            } else if (auto printNode = dynamic_cast<PrintNode*>(node.get())) {
                std::cout << evaluateExpression(printNode->expr.get()) << std::endl;
            } else if (auto queryNode = dynamic_cast<DatabaseQueryNode*>(node.get())) {
                std::string result = exec(("echo \"Database query: " + queryNode->query + "\"").c_str());
                std::cout << result << std::endl;
            } else if (auto assignmentNode = dynamic_cast<AssignmentNode*>(node.get())) {
                std::string varName = assignmentNode->variable->name;
                std::string valueStr = evaluateExpression(assignmentNode->expr.get());
                try {
                    // Try to parse the value as a double
                    double value = std::stod(valueStr);
                    variables[varName] = value;
                } catch (const std::invalid_argument&) {
                    // If parsing fails, store it as a string
                    variables[varName] = valueStr;
                } catch (const std::out_of_range&) {
                    std::cerr << "Number out of range for variable assignment: " << valueStr << std::endl;
                }
            }
        }
    }

private:
    std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
        if (!pipe) throw std::runtime_error("popen() failed!");
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }

    std::string evaluateExpression(ASTNode* node) {
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

    std::unordered_map<std::string, VariableValue> variables;
};

// Main function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_name>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return 1;
    }
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    Tokenizer tokenizer(source);
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(tokens);
    std::vector<std::unique_ptr<ASTNode>> nodes = parser.parse();

    Interpreter interpreter;
    interpreter.interpret(nodes);

    return 0;
}

