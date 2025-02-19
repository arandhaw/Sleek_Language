#include "codegen.cpp"


std::string readFileIntoString(const std::string& filePath) {
    std::ifstream inputFile(filePath); // Open the file
    if (!inputFile) {
        cout << "Error: Could not open file " + filePath;
        exit(1);
    }
    // Read the file content into a string
    return std::string((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
}

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
        return 0;
    } else {
        cout << "Parser success" << endl;
        printProgram(result.result);
    }
    Program& p = result.result;
    cout << "Done" << endl;

    // code generation (incomplete)
    string filename = "output.cpp";
    Codegen cg;
    if(!cg.init(filename)){
        print("Error: Unable to open file", filename);
        return 0;
    }
    cg.genProgram(p);
    return 0;
}



