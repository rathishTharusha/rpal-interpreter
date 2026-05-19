#include "Parser.h"
#include "Standardizer.h"
#include "CSEMachine.h"
#include <iostream>
#include <cstring>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./rpal20 [-ast] <filename>\n";
        return 1;
    }

    bool printAST = false;
    std::string filename;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-ast") == 0) {
            printAST = true;
        } else {
            filename = argv[i];
        }
    }

    if (filename.empty()) {
        std::cerr << "Error: No input file specified.\n";
        return 1;
    }
    
    try {
        Parser parser(filename);
        auto ast = parser.parse();
        
        if (printAST) {
            std::cout << "Original AST:\n";
            parser.printAST(ast);
        }
        
        Standardizer st;
        auto standardizedTree = st.standardize(ast);
        
        if (printAST) {
            std::cout << "\nStandardized AST:\n";
            parser.printAST(standardizedTree);
        } else {
            CSEMachine machine(standardizedTree);
            machine.evaluate();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
