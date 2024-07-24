#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <regex>
#include <iostream>


// Enumeration of token types
enum TokenType {
    T_EOF,
    
    T_TEXT,

    T_ECHO,
    T_DB,
    T_HTTP,
    
    T_NUMBER,
    T_STRING,
    
    T_VARIABLE,
    T_OPERATOR,
    T_ASSIGN,
    
    T_SEMICOLON,
    T_LPAREN,   // '('
    T_RPAREN,  // ')'
    T_COMMA 
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

    std::vector<Token> tokenize();

private:
    bool match(const std::string& pattern);
    void skipComment();
    void skipCommentMultilene();

    std::string source;
    size_t position;
    std::string first;
    std::string lastMatch;
    bool insidePHP;
};

#endif // LEXER_H



