#include "utils.cpp"
#include "lexer.cpp"
#include "parser.cpp"
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
void make_ft(){
    // Type_table tt;
    // tt.add("int", Type_info("int", Supertype::PRIMITIVE));
    // tt.add("float", Type_info("double", Supertype::PRIMITIVE));
    // tt.add("bool", Type_info("bool", Supertype::PRIMITIVE));
    Function_table& ft = func_table;
    
    ft.add(Signature{"print", vector<string>{"int"}, "none"});
    ft.add(Signature{"print", vector<string>{"float"}, "none"});
    ft.add(Signature{"print", vector<string>{"char"}, "none"});
    ft.add(Signature{"print", vector<string>{"bool"}, "none"});
    ft.add(Signature{"print", vector<string>{"none"}, "none"});

    ft.add(Signature{"+", vector<string>{"int", "int"}, "int"}, "add");
    ft.add(Signature{"+", vector<string>{"float", "int"}, "float"}, "add");
    ft.add(Signature{"+", vector<string>{"int", "float"}, "float"}, "add");
    ft.add(Signature{"+", vector<string>{"float", "float"}, "float"}, "add");

    ft.add(Signature{"-", vector<string>{"int", "int"}, "int"}, "sub");
    ft.add(Signature{"-", vector<string>{"int", "float"}, "float"}, "sub");
    ft.add(Signature{"-", vector<string>{"float", "int"}, "float"}, "sub");
    ft.add(Signature{"-", vector<string>{"float", "float"}, "float"}, "sub");

    ft.add(Signature{"*", vector<string>{"int", "int"}, "int"}, "mul");
    ft.add(Signature{"*", vector<string>{"int", "float"}, "float"}, "mul");
    ft.add(Signature{"*", vector<string>{"float", "int"}, "float"}, "mul");
    ft.add(Signature{"*", vector<string>{"float", "float"}, "float"}, "mul");

    ft.add(Signature{"/", vector<string>{"int", "int"}, "float"}, "div");
    ft.add(Signature{"/", vector<string>{"int", "float"}, "float"}, "div");
    ft.add(Signature{"/", vector<string>{"float", "int"}, "float"}, "div");
    ft.add(Signature{"/", vector<string>{"float", "float"}, "float"}, "div");

    ft.add(Signature{"//", vector<string>{"int", "int"}, "float"}, "idiv");
    ft.add(Signature{"**", vector<string>{"int", "int"}, "int"}, "iexp");

    ft.add(Signature{"^", vector<string>{"int", "int"}, "float"}, "exp");
    ft.add(Signature{"^", vector<string>{"int", "float"}, "float"}, "exp");
    ft.add(Signature{"^", vector<string>{"float", "int"}, "float"}, "exp");
    ft.add(Signature{"^", vector<string>{"float", "float"}, "float"}, "exp");
    
    ft.add(Signature{"%", vector<string>{"int", "int"}, "int"}, "mod");

    ft.add(Signature{"++", vector<string>{"int"}, "int"}, "increment");
    ft.add(Signature{"--", vector<string>{"int"}, "int"}, "decrement");

    ft.add(Signature{"u-", vector<string>{"int"}, "int"}, "unary_minus");
    ft.add(Signature{"u-", vector<string>{"float"}, "float"}, "unary_minus");

    ft.add(Signature{"and", vector<string>{"bool", "bool"}, "bool"}, "logic_and");
    ft.add(Signature{"or", vector<string>{"bool", "bool"}, "bool"}, "logic_or");
    ft.add(Signature{"!", vector<string>{"bool", "bool"}, "bool"}, "logic_not");

    ft.add(Signature{"==", vector<string>{"bool", "bool"}, "bool"}, "equals");
    ft.add(Signature{"==", vector<string>{"int", "int"}, "bool"}, "equals");
    ft.add(Signature{"==", vector<string>{"char", "char"}, "bool"}, "equals");
    ft.add(Signature{"==", vector<string>{"byte", "byte"}, "bool"}, "equals");

    ft.add(Signature{"!=", vector<string>{"bool", "bool"}, "bool"}, "neq");
    ft.add(Signature{"!=", vector<string>{"int", "int"}, "bool"}, "neq");
    ft.add(Signature{"!=", vector<string>{"char", "char"}, "bool"}, "neq");
    ft.add(Signature{"!=", vector<string>{"byte", "byte"}, "bool"}, "neq");

    ft.add(Signature{">", vector<string>{"int", "int"}, "bool"}, "gt");
    ft.add(Signature{">", vector<string>{"byte", "byte"}, "bool"}, "gt");
    ft.add(Signature{">", vector<string>{"float", "float"}, "bool"}, "gt");

    ft.add(Signature{">=", vector<string>{"int", "int"}, "bool"}, "geq");
    ft.add(Signature{">=", vector<string>{"byte", "byte"}, "bool"}, "geq");
    ft.add(Signature{">=", vector<string>{"float", "float"}, "bool"}, "geq");

    ft.add(Signature{"<", vector<string>{"int", "int"}, "bool"}, "lt");
    ft.add(Signature{"<", vector<string>{"byte", "byte"}, "bool"}, "lt");
    ft.add(Signature{"<", vector<string>{"float", "float"}, "bool"}, "lt");

    ft.add(Signature{"<=", vector<string>{"int", "int"}, "bool"}, "leq");
    ft.add(Signature{"<=", vector<string>{"byte", "byte"}, "bool"}, "leq");
    ft.add(Signature{"<=", vector<string>{"float", "float"}, "bool"}, "leq");

    // cout << ft.rtype("+", vector<string>{"int", "int"}).valid << endl;
    // cout << ft.rtype("+", vector<string>{"int", "float"}).valid << endl;
    // cout << ft.rtype("+", vector<string>{"float", "float"}).valid << endl;
    // cout << ft.rtype("+", vector<string>{"float", "int"}).valid << endl;
}


// struct TypeInfo {
//     string name;
//     size_t size;
//     Entity supertype;
//     Type_ast structure;
//     bool copyable;
//     bool sized;
//     bool destructor;
// };

Type type_none;
Type type_int;
Type type_float;
Type type_byte;
Type type_bool;
Type type_char;

void init_type_table(){
    type_none = type_table.insert( TypeInfo{"none", sizeof($none), PRIMITIVE_TYPE, Type_ast{Named_type{}}, true, true, false} );
    type_int = type_table.insert( TypeInfo{"int", sizeof($int), PRIMITIVE_TYPE,  Type_ast{Named_type{}}, true, true, false} );
    type_float = type_table.insert( TypeInfo{"float", sizeof($float), PRIMITIVE_TYPE,  Type_ast{Named_type{}}, true, true, false} );
    type_byte = type_table.insert( TypeInfo{"byte", sizeof($byte), PRIMITIVE_TYPE,  Type_ast{Named_type{}}, true, true, false} );
    type_bool = type_table.insert( TypeInfo{"bool", sizeof($bool), PRIMITIVE_TYPE, Type_ast{Named_type{}}, true, true, false} );
    type_char = type_table.insert( TypeInfo{"char", sizeof($char), PRIMITIVE_TYPE,  Type_ast{Named_type{}}, true, true, false} );
}

void startup(){
    // add builtin functions to function table
    make_ft();
    init_type_table();
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
    tokens = stage1.tokens;
    
    cout << "Beginning parsing..." << endl;
    
    auto result = parseProgram(tokens.begin());
    if(result.valid == false){
        cout << "Parsing Failed" << endl;
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
    print("Starting codegen");
    cg.genProgram(p);
    print("Codegen complete");
    return 0;
}



