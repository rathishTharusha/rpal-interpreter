#include "Parser.h"
#include "Standardizer.h"
#include "CSEMachine.h"
#include <iostream>
#include <cstring>

/**
 * @brief Interpreter Entry Point.
 * Orchestrates the full compilation and execution pipeline:
 * Lexical Analysis -> Syntax Parsing -> AST Standardization -> CSE Machine execution.
 * 
 * Supports the CLI arguments:
 * - './rpal20 -ast <file>' to print the original and standardized Abstract Syntax Trees.
 * - './rpal20 <file>' to run the program to completion and output results to stdout.
 */
int main(int argc, char** argv) {
    // 1. Argument validation
    if (argc < 2) {
        std::cerr << "Usage: ./rpal20 [-ast] <filename>\n";
        return 1;
    }

    bool printAST = false;
    std::string filename;

    // 2. Parse CLI flags and locate filename
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
        // 3. Phase A & B: Lex and Parse the program to build the AST
        Parser parser(filename);
        auto ast = parser.parse();
        
        if (printAST) {
            std::cout << "Original AST:\n";
            parser.printAST(ast);
        }
        
        // 4. Phase C: Standardize the tree (Syntactic desugaring)
        Standardizer st;
        auto standardizedTree = st.standardize(ast);
        
        if (printAST) {
            std::cout << "\nStandardized AST:\n";
            parser.printAST(standardizedTree);
        } else {
            // 5. Phase D: Load instructions into CSE Machine virtual machine and evaluate
            CSEMachine machine(standardizedTree);
            machine.evaluate();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
