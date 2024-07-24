#include "lexer.h"


std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    while (position < source.length()) {
        if (match("<\\?php\\b")) {
        	first = "";
            if (insidePHP) {
                std::cerr << "Unexpected <?php tag without closing ?>" << std::endl;
                return tokens;
            }
            if (position > 0 && source[position - 1] == '<') {
                tokens.push_back({T_TEXT, "<"});
            }
            insidePHP = true;
        } else if (match("\\?>")) {
            if (!insidePHP) {
                std::cerr << "Unexpected ?> tag without opening <?php" << std::endl;
                return tokens;
            }
            insidePHP = false;
        } else if (insidePHP) {
            if (match("//")) { 
                skipComment();
            } else if (match("\\/\\*")) {
              position += 2;
              skipCommentMultilene();
            } else if (match("echo")) {
                tokens.push_back({T_ECHO, "echo"});
            } else if (match("db")) {
                tokens.push_back({T_DB, "db"});
            } else if (match("http")) { // Добавляем обработку токена http
                tokens.push_back({T_HTTP, "http"});
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
            } else if (match(",")) {
                tokens.push_back({T_COMMA, ","});
            } else if (match("\\(")) { // Match left parenthesis
                tokens.push_back({T_LPAREN, "("});
            } else if (match("\\)")) { // Match right parenthesis
                tokens.push_back({T_RPAREN, ")"});
            } else {
                position++;
            }
        } else {
            if (match("<")) {
                first = "<";
            } else if (match("[^<]+")) {
                tokens.push_back({T_TEXT, first + lastMatch});
            } else {
                position++;
            }
        }
    }

    tokens.push_back({T_EOF, ""});
    return tokens;
}


bool Tokenizer::match(const std::string& pattern) {
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


void Tokenizer::skipComment() {
    while (position < source.length()) {
        if (source[position] == '\n') {
            position++; // Skip newline character
            break;
        }
        if (source.substr(position, 2) == "?>") {
            break;
        }
        position++;
    }
}


void Tokenizer::skipCommentMultilene() {
    while (position < source.length()) {
        if (source.substr(position, 2) == "*/") {
            position += 2; // Skip closing tag characters
            break;
        }
        if (source.substr(position, 2) == "?>") {
            break;
        }
        position++;
    }
}

