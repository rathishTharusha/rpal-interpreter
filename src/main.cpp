#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./rpal20 <filename>\n";
        return 1;
    }

    std::string filename = argv[1];
    
    // Future integration:
    // 1. Lexer lexer(filename);
    // 2. Parser parser(lexer);
    // 3. auto ast = parser.parse();
    // 4. Standardizer st(ast);
    // 5. auto standardizedTree = st.standardize();
    // 6. CSEMachine machine(standardizedTree);
    // 7. machine.evaluate();

    std::cout << "RPAL Interpreter initialized for " << filename << std::endl;

    return 0;
}
