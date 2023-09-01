#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

enum TokenType {
    ENTRY,
    STACKEXPR,
    IDENTIFIER,
    CALL,
    INT_T,
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    INCLUDE,
    END
};

typedef std::string str_t;

struct Token {
    TokenType type;
    str_t value;
};

// Checks buffer for valid tokens, adds them to token list and
// clears the buffer, making room for the next word to be read.
void check_buffer(str_t &buffer, std::vector<Token> &list) {
    if (buffer == "main") {
        list.push_back({ENTRY, buffer});
        
    } else if (buffer == "stackexpr") {
        list.push_back({STACKEXPR, buffer});
        
    } else if (buffer == "end") {
        list.push_back({END, buffer});
        
    } else if (buffer == "do") {
        list.push_back({CALL, buffer});
        
    } else if (buffer == "include") {
        list.push_back({INCLUDE, buffer});
        
    } else if (buffer == "+") {
        list.push_back({ADD_OP, buffer});
        
    } else if (buffer == "-") {
        list.push_back({SUB_OP, buffer});
        
    } else if (buffer == "*") {
        list.push_back({MUL_OP, buffer});
        
    } else if (buffer == "/") {
        list.push_back({DIV_OP, buffer});
        
    } else if (isalpha(buffer[0])) {
        list.push_back({IDENTIFIER, buffer});
        
    } else if (isdigit(buffer[0])) {
        list.push_back({INT_T, buffer});
    }
    
    buffer.clear();
}

// Takes filename as parameter and performs lexical analysis,
// and returns a list of meaningful tokens.
// The values returned from this function will be fed into the
// parser that will generate assembly code.
std::vector<Token> tokenize(str_t &filename) {
    std::ifstream src;
    src.open(filename);
    
    // Throw error if file does not exist.
    if (!src) {
        std::cerr << "ERROR: File name provided does not exist." << '\n';
        exit(EXIT_FAILURE);
    }
    
    // ch holds the current character being read.
    // buffer is the accumulation of characters.
    str_t buffer;
    char ch;
    
    // This vector holds the tokens returned by the function.
    std::vector<Token> tokens;
    
    // noskipws is used to not skip whitespaces. Who came up with this???
    while (src >> std::noskipws >> ch) { 
        // Push the buffer to the list of tokens if a whitespace is encountered
        // or if it is the end of the file.
        if (isspace(ch) || (ch == EOF)) {
            check_buffer(buffer, tokens);
            
        } else { 
            buffer += ch; 
        }
    }
    
    // Check the buffer characters after the last iteration.
    if (!buffer.empty()) {
        check_buffer(buffer, tokens);
    }
    
    src.close();
    
    return tokens;
}

// Iterates over a list of tokens and generates assembly code if the tokens 
// form valid program statements.
void parse_tokens(std::vector<Token> &tokens, str_t &out_buffer) {
    // This keeps track of which scope the parser is in.
    // The end of the vector shows the current scope.
    std::vector<TokenType> end_prec_stack;
    
    // Contains every identifier declared.
    std::vector<str_t> declared_names;
    
    // Generating essentials: .text section contains most of the program code.
    out_buffer += "global _start\n";
    out_buffer += "section .text\n\n";
    
    for (int ptr = 0; ptr < tokens.size(); ptr++) {
        //std::cout << "parser: " << tokens[ptr].value << '\n';
        
        // stackexpr uses reverse polish notation, and is used to work with the stack.
        // it creates a callable macro that can be used repeatedly.
        if (tokens[ptr].type == STACKEXPR) {
            Token identifier = tokens[++ptr];
            
            if (identifier.type == IDENTIFIER) {
                declared_names.push_back(identifier.value);
                
                out_buffer += "%macro " + identifier.value + " 0\n";
                
                ptr++; // Make pointer point to the token after identifier, which is the stackexpr body.
                
                // Read the tokens until the end of the stackexpr body is reached.
                while (tokens[ptr].type != END) {
                    if (tokens[ptr].type == INT_T) {
                        out_buffer += "\n;; -- push --\n";
                        out_buffer += "mov rax, " + tokens[ptr].value + "\n";
                        out_buffer += "push rax\n";
                        
                    } else if (tokens[ptr].type == ADD_OP) {
                        out_buffer += "\n;; -- add --\n";
                        out_buffer += "pop rax\n";
                        out_buffer += "pop rbx\n";
                        out_buffer += "add rax, rbx\n";
                        out_buffer += "push rax\n";
                        
                    } else if (tokens[ptr].type == SUB_OP) {
                        out_buffer += "\n;; -- sub --\n";
                        out_buffer += "pop rbx\n";
                        out_buffer += "pop rax\n";
                        out_buffer += "sub rax, rbx\n";
                        out_buffer += "push rax\n";
                        
                    } else if (tokens[ptr].type == MUL_OP) {
                        out_buffer += "\n;; -- mul --\n";
                        out_buffer += "pop rax\n";
                        out_buffer += "pop rbx\n";
                        out_buffer += "imul rax, rbx\n";
                        out_buffer += "push rax\n";
                        
                    } else if (tokens[ptr].type == DIV_OP) {
                        out_buffer += "\n;; -- div --\n";
                        out_buffer += "pop rbx\n";
                        out_buffer += "pop rax\n";
                        out_buffer += "idiv rbx\n";
                        out_buffer += "push rax\n";
                    }
                    
                    ptr++;
                }
                
                out_buffer += "%endmacro\n\n";
                
            } else {
                std::cerr << "ERROR: Expected name/identifier after 'stackexpr' for declaration." << '\n';
                std::cerr << "Try: stackexpr <name>\n    <body>\nend" << '\n';
                exit(EXIT_FAILURE);
            }
        
        // Including files.
        } else if (tokens[ptr].type == INCLUDE) {
            if (tokens[++ptr].type == IDENTIFIER) {
                out_buffer += "%include " + tokens[ptr].value + "\n";
            
            } else {
                std::cerr << "ERROR: Tried including file but filename not provided after " << '"' << "include" << '"' << '\n';
                std::cerr << "Try: include <filename>" << '\n';
            }
        
        // Calling stackexpr.
        } else if (tokens[ptr].type == CALL) {
            if (tokens[++ptr].type == IDENTIFIER) {
                // Check if name called is declared.
                if (std::find(declared_names.begin(), declared_names.end(), tokens[ptr].value) != declared_names.end()) {
                    out_buffer += tokens[ptr].value + "\n";

                } else {
                    std::cerr << "ERROR: Tried to call stackexpr but the name " << '"' << tokens[ptr].value << '"' << " is not defined." << '\n';
                    exit(EXIT_FAILURE);
                }

            } else {
                std::cerr << "ERROR: Call keyword " << tokens[ptr].value << " expects name/identifier of an already defined stackexpr.";
                exit(EXIT_FAILURE);
            }
            
        // Creating entry point.
        } else if (tokens[ptr].type == ENTRY) {
            end_prec_stack.push_back(ENTRY);
            out_buffer += "_start:\n";
        
        } else if (tokens[ptr].type == END) {

            // If the scope is in the entry point, the end encountered will be the end of the program.
            if (!end_prec_stack.empty()) {
                if (end_prec_stack.back() == ENTRY) {
                    end_prec_stack.pop_back();
                    
                    out_buffer += ";; -- exit syscall--\n";
                    out_buffer += "mov rax, 60\n";
                    out_buffer += "pop rdi\n";
                    out_buffer += "syscall\n"; 
                }
            }
        }
    }
}

// ANCHOR: Main
int main(int argc, char** argv) {
    bool show_debug = false;
    
    if (argc < 2) {
        std::cerr << "ERROR: No file name provided." << '\n';
        exit(EXIT_FAILURE);
    }
    
    str_t src_filename = argv[1];
    
    std::vector<Token> tokens = tokenize(src_filename);
    
    if (show_debug) {
        for (Token tok: tokens) {
            std::cout << "TokenType: " << tok.type << " Value: " << tok.value << '\n';
        }
    }
    
    str_t out_buffer;
    
    parse_tokens(tokens, out_buffer);
    
    if (show_debug) {
        std::cout << '\n' << out_buffer << '\n';
    }
    
    std::ofstream asm_out;
    
    std::string out_filename = argv[2];
    
    asm_out.open(out_filename);
    
    asm_out << out_buffer;
    
    asm_out.close();
    
    std::string asm_suffix = ".asm";
    std::string obj_filename = out_filename.substr(0, out_filename.length() - asm_suffix.length());
    
    std::string nasm_cmd = "nasm -f elf64 " + out_filename + " -o " + obj_filename + ".o";
    std::string linker_cmd = "ld " + obj_filename + ".o -o " + obj_filename;
    
    std::system(const_cast<char*>(nasm_cmd.c_str()));
    std::system(const_cast<char*>(linker_cmd.c_str()));
    
    return 0;
}