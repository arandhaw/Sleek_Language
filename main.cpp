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

// list of built-in functions
void init_function_table(){
    Function_table& ft = func_table;
    ft.add(Signature{"length", vector<Type>{type_string}, type_int});
    ft.add(Signature{"print", vector<Type>{type_string}, type_none});
    ft.add(Signature{"copy", vector<Type>{type_string}, type_string});

    ft.add(Signature{"print", vector<Type>{type_int}, type_none});
    ft.add(Signature{"print", vector<Type>{type_float}, type_none});
    ft.add(Signature{"print", vector<Type>{type_char}, type_none});
    ft.add(Signature{"print", vector<Type>{type_bool}, type_none});
    ft.add(Signature{"print", vector<Type>{type_none}, type_none});
    ft.add(Signature{"print", vector<Type>{type_byte}, type_none});
    ft.add(Signature{"+", vector<Type>{type_int, type_int}, type_int}, "add");
    ft.add(Signature{"+", vector<Type>{type_float, type_int}, type_float}, "add");
    ft.add(Signature{"+", vector<Type>{type_int, type_float}, type_float}, "add");
    ft.add(Signature{"+", vector<Type>{type_float, type_float}, type_float}, "add");
    ft.add(Signature{"-", vector<Type>{type_int, type_int}, type_int}, "sub");
    ft.add(Signature{"-", vector<Type>{type_int, type_float}, type_float}, "sub");
    ft.add(Signature{"-", vector<Type>{type_float, type_int}, type_float}, "sub");
    ft.add(Signature{"-", vector<Type>{type_float, type_float}, type_float}, "sub");

    ft.add(Signature{"*", vector<Type>{type_int, type_int}, type_int}, "mul");
    ft.add(Signature{"*", vector<Type>{type_int, type_float}, type_float}, "mul");
    ft.add(Signature{"*", vector<Type>{type_float, type_int}, type_float}, "mul");
    ft.add(Signature{"*", vector<Type>{type_float, type_float}, type_float}, "mul");

    ft.add(Signature{"/", vector<Type>{type_int, type_int}, type_float}, "div");
    ft.add(Signature{"/", vector<Type>{type_int, type_float}, type_float}, "div");
    ft.add(Signature{"/", vector<Type>{type_float, type_int}, type_float}, "div");
    ft.add(Signature{"/", vector<Type>{type_float, type_float}, type_float}, "div");

    ft.add(Signature{"//", vector<Type>{type_int, type_int}, type_float}, "idiv");
    ft.add(Signature{"**", vector<Type>{type_int, type_int}, type_int}, "iexp");

    ft.add(Signature{"^", vector<Type>{type_int, type_int}, type_float}, "exp");
    ft.add(Signature{"^", vector<Type>{type_int, type_float}, type_float}, "exp");
    ft.add(Signature{"^", vector<Type>{type_float, type_int}, type_float}, "exp");
    ft.add(Signature{"^", vector<Type>{type_float, type_float}, type_float}, "exp");
    
    ft.add(Signature{"%", vector<Type>{type_int, type_int}, type_int}, "mod");

    ft.add(Signature{"++", vector<Type>{type_int}, type_int}, "increment");
    ft.add(Signature{"--", vector<Type>{type_int}, type_int}, "decrement");

    ft.add(Signature{"u-", vector<Type>{type_int}, type_int}, "unary_minus");
    ft.add(Signature{"u-", vector<Type>{type_float}, type_float}, "unary_minus");

    ft.add(Signature{"and", vector<Type>{type_bool, type_bool}, type_bool}, "logic_and");
    ft.add(Signature{"or", vector<Type>{type_bool, type_bool}, type_bool}, "logic_or");
    ft.add(Signature{"!", vector<Type>{type_bool, type_bool}, type_bool}, "logic_not");

    ft.add(Signature{"==", vector<Type>{type_bool, type_bool}, type_bool}, "equals");
    ft.add(Signature{"==", vector<Type>{type_int, type_int}, type_bool}, "equals");
    ft.add(Signature{"==", vector<Type>{type_char, type_char}, type_bool}, "equals");
    ft.add(Signature{"==", vector<Type>{type_byte, type_byte}, type_bool}, "equals");

    ft.add(Signature{"!=", vector<Type>{type_bool, type_bool}, type_bool}, "neq");
    ft.add(Signature{"!=", vector<Type>{type_int, type_int}, type_bool}, "neq");
    ft.add(Signature{"!=", vector<Type>{type_char, type_char}, type_bool}, "neq");
    ft.add(Signature{"!=", vector<Type>{type_byte, type_byte}, type_bool}, "neq");

    ft.add(Signature{">", vector<Type>{type_int, type_int}, type_bool}, "gt");
    ft.add(Signature{">", vector<Type>{type_byte, type_byte}, type_bool}, "gt");
    ft.add(Signature{">", vector<Type>{type_float, type_float}, type_bool}, "gt");

    ft.add(Signature{">=", vector<Type>{type_int, type_int}, type_bool}, "geq");
    ft.add(Signature{">=", vector<Type>{type_byte, type_byte}, type_bool}, "geq");
    ft.add(Signature{">=", vector<Type>{type_float, type_float}, type_bool}, "geq");

    ft.add(Signature{"<", vector<Type>{type_int, type_int}, type_bool}, "lt");
    ft.add(Signature{"<", vector<Type>{type_byte, type_byte}, type_bool}, "lt");
    ft.add(Signature{"<", vector<Type>{type_float, type_float}, type_bool}, "lt");

    ft.add(Signature{"<=", vector<Type>{type_int, type_int}, type_bool}, "leq");
    ft.add(Signature{"<=", vector<Type>{type_byte, type_byte}, type_bool}, "leq");
    ft.add(Signature{"<=", vector<Type>{type_float, type_float}, type_bool}, "leq");

    // cout << ft.rtype("+", vector<Type>{type_int, type_int}).valid << endl;
    // cout << ft.rtype("+", vector<Type>{type_int, type_float}).valid << endl;
    // cout << ft.rtype("+", vector<Type>{type_float, type_float}).valid << endl;
    // cout << ft.rtype("+", vector<Type>{type_float, type_int}).valid << endl;
}

// list of builtin entities
void init_entity_table(){
    // list of valid entities - structs, enums, primitive_types
    entity_table.add("int", PRIMITIVE_TYPE);
    entity_table.add("float", PRIMITIVE_TYPE);
    entity_table.add("none", PRIMITIVE_TYPE);
    entity_table.add("bool", PRIMITIVE_TYPE);
    entity_table.add("byte", PRIMITIVE_TYPE);
    entity_table.add("char", PRIMITIVE_TYPE);
    entity_table.add("string", PRIMITIVE_TYPE);
    // built-in functions automatically added to entity table
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

// initialize built-in types
void init_type_table(){
    // name, size (bytes), supertype, copyable, sized, destructor
    type_none = type_table.insert( TypeInfo{"none", sizeof($none), PRIMITIVE_TYPE, vector<field>{}, true, true, false} );
    vector<field> Int_fields = {field{"MAX", type_int}, field{"MIN", type_int}};
    type_int = type_table.insert( TypeInfo{"int", sizeof($int), PRIMITIVE_TYPE, Int_fields, true, true, false} );
    type_float = type_table.insert( TypeInfo{"float", sizeof($float), PRIMITIVE_TYPE, vector<field>{}, true, true, false} );
    type_byte = type_table.insert( TypeInfo{"byte", sizeof($byte), PRIMITIVE_TYPE, vector<field>{}, true, true, false} );
    type_bool = type_table.insert( TypeInfo{"bool", sizeof($bool), PRIMITIVE_TYPE, vector<field>{}, true, true, false} );
    type_char = type_table.insert( TypeInfo{"char", sizeof($char), PRIMITIVE_TYPE, vector<field>{}, true, true, false} );
    type_function = type_table.insert( TypeInfo{"Function", 2*sizeof(int*), PRIMITIVE_TYPE, vector<field>{}, true, true, true} );
    type_string = type_table.insert(TypeInfo{"string", sizeof($string), PRIMITIVE_TYPE, vector<field>{}, false, true, false});
}

void startup(){
    
    init_type_table();
    init_function_table();
    init_entity_table();
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
    print("Startup complete");
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



