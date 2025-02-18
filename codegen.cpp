#include "parser.cpp"
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include "utils.cpp"



// each time a function or operation is called
// we need to search the function table for the type of the params
// validate whether its a function, get its return type

unordered_set<string> functions = {"print"};

void startup(){
    // generate precedence map
    int i = 0;
    for(auto x : operator_precedence){
        for(auto y : x){
            precedence.insert({y, i});
        }
        i++;
    }
}

// any C program is composed of functions, unions, structs, enums, global variables
// C++ adds classes, templates, namespaces/modules 

struct CodegenOption { 
    bool valid;
    string code;
};

CodegenOption codegen(Program program){
    auto ret = CodegenOption();
    // ret.code += 
    return CodegenOption();
}


void writeToFile(string fileName, string content){
    // Create an ofstream object
    std::ofstream outFile;
    // Open the file in write mode
    outFile.open(fileName);

    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
        std::cout << "File written successfully." << std::endl;
    } else {
        std::cerr << "Unable to open file." << std::endl;
    }
}

std::string readFileIntoString(const std::string& filePath) {
    std::ifstream inputFile(filePath); // Open the file
    if (!inputFile) {
        cout << "Error: Could not open file " + filePath;
        exit(1);
    }
    // Read the file content into a string
    return std::string((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
}

int main(){
    startup();
    string expression = readFileIntoString("program.sleek");
    //string expression = "3 + 4**3 * 4";
    cout << "Starting lexing..." << endl;
    LexerOutput stage1 = lexer(expression);
    if(stage1.success == false){
        cout << "Lexer error, terminating session" << endl;
        return 0;
    }
    cout << "Lexer successful" << endl;
    vector<Token>& tokens = stage1.tokens;
    
    cout << "Beginning parsing..." << endl;
    
    auto result = parseProgram(tokens, tokens.begin());
    if(result.valid == false){
        cout << "Oh no, parsing error at this line:" << endl;
    } else {
        cout << "Parser success" << endl;
        printProgram(result.result);
    }
    cout << "Done" << endl;
    writeToFile("output.c", "hello");
    return 0;
}



