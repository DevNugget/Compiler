#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


enum TokenType {
    ENTRY,
    STACKEXPR,
    IDENTIFIER,
    INT_T,
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    END
};

typedef std::string str_t;

struct Token {
    TokenType type;
    str_t value;
};

// Vector of type Token.
typedef std::vector<Token> tvec_t;

// Checks buffer for valid tokens, adds them to token list and
// clears the buffer, making room for the next word to be read.
void check_buffer(str_t &buffer, tvec_t &list) {
    if (buffer == "main") {
        list.push_back({ENTRY, buffer});

    } else if (buffer == "stackexpr") {
        list.push_back({STACKEXPR, buffer});

    } else if (buffer == "end") {
        list.push_back({END, buffer});

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
tvec_t tokenize(str_t &filename) {
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
        
        } else { buffer += ch; }

        std::cout << "Ch: " << ch << '\n';
        std::cout << "Buffer: " << buffer << '\n';
    }

    // Check the buffer characters after the last iteration.
    if (!buffer.empty()) {
        check_buffer(buffer, tokens);
    }

    return tokens;
}

void parse_tokens(tvec_t &tokens) {
    for (int ptr = 0; ptr < tokens.size(); ptr++) {
        if (tokens[ptr].type == STACKEXPR) {
            Token identifier = tokens[ptr++];

            if (identifier.type == IDENTIFIER) {

            } else {
                std::cerr << "ERROR: Expected name/identifier after 'stackexpr' for declaration." << '\n';
                std::cerr << "Try: stackexpr <name>\n    <body>\nend" << '\n';
                exit(EXIT_FAILURE);
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "ERROR: No file name provided." << '\n';
        exit(EXIT_FAILURE);
    }

    str_t src_filename = argv[1];

    tvec_t tokens = tokenize(src_filename);

    for (Token tok: tokens) {
        std::cout << "TokenType: " << tok.type << " Value: " << tok.value << '\n';
    }

    return 0;
}