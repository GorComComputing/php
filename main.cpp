#include <iostream>
#include <ostream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>

#include "lexer.h"
#include "parser.h"
#include "interpret.h"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_name>" << std::endl;
        return 1;
    }

    // Открыть файл
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Unable to open file: " << argv[1] << std::endl;
        return 1;
    }
    
    // Прочитать файл в строку
    std::string source{(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()};

    // Конструктор класса Tokenizer
    Tokenizer tokenizer{source};
    std::vector<Token> tokens = tokenizer.tokenize();
/*
    // Вывести на экран список токенов
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << i + 1 << ". (" << tokens[i].type << ") " << tokens[i].value << std::endl;
    }
*/
    // Конструктор класса Parser
    Parser parser(tokens);
    std::vector<std::unique_ptr<ASTNode>> nodes = parser.parse();
/*
    // Вывести на экран содержимое AST
    Parser::printAST(nodes);
*/    
    // Конструктор класса Interpreter
    Interpreter interpreter;
    interpreter.interpret(nodes);

    return 0;
}


