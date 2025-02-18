// TypeTable - all types in current module
// holds types as unordered map: str -> type_info object
// operations: add, check, info

// FunctionTable - all functions in current module
// what we might have: built-in functions, user defined functions in sleek
// user defined functions in C++, C - we need a way to translate types
// string -> 

// enum class Supertype {
//     PRIMITIVE
// };

// struct Type_info {
//     string codegen_name;
//     Supertype supertype;
//     Type_info(){}
//     Type_info(string codegen_name, Supertype st) : 
//         codegen_name(codegen_name), supertype(st){}
// };

// // list of available types
// struct Type_table {
//     private:
//         unordered_map<string, Type_info> table;
//     public:
//         bool add(string name, Type_info type_info){
//             if(table.count(name)){
//                 cout << "type already exists" << endl;
//                 return false;
//             } else {
//                 table[name] = type_info;
//                 return true;
//             }
//         }
//         Option<Type_info> search(string name){
//             if(check(name)){
//                 return Option(table.at(name));
//             }
//             return Option<Type_info>();
//         }

//         bool check(string name){
//             return table.count(name);
//         }
// };

// enum Function_type {
//     BUILTIN,
//     USER_DEFINED
// };

// struct Function_info {
//     Function_type type;
// };


///////////////////////////////////////////////////////////////////////////////////////////////////
// function table

// add to function table
// return false if a duplicate signature exists
bool Function_table::add(Signature f){
    if(f.return_type == ""){
        f.return_type = "none";
    }
    Option<Signature> duplicate = search(f.name, f.param_types);
    if(duplicate.valid == true){
        print("Error in functiontable.add", f.name, f.param_types);
        return false;
    }
    table.insert({f.name, f});
    return true;
}
// return type of function
Option<string> Function_table::rtype(string name, vector<string> params){
    Option<Signature> res = search(name, params);
    Option<string> ret(true);
    if(res.valid == false){ ret.valid = false; return ret; }
    else {
        ret.result = res.result.return_type;
        return ret;
    }
}

// search the table using the name and parameters
Option<Signature> Function_table::search(string name, vector<string> params){
    Option<Signature> result(false);
    auto it = table.equal_range( name );
    for(auto i = it.first; i != it.second; ++i){
        vector<string> i_params = i->second.param_types;
        if(params.size() != i_params.size()){
            continue;
        }
        // compare each parameter type
        bool equal = true;
        for(int i = 0; i < params.size(); i++){
            if(params[i] != i_params[i]){
                equal = false;
                break;
            }
        }
        if(equal){
            result.result = i->second;
            result.valid = true;
            break;
        }
    }
    return result;
}
// return number of function with said name
int Function_table::check(string name){
    return table.count(name);
}

void Function_table::printAll(){
    cout << "Function table:" << endl;
    for(pair<string, Signature> i : table){
        cout << i.second.name << "(";
        for(auto& j : i.second.param_types){
            cout << j << ", ";
        }
        cout << "\b\b) -> " << i.second.return_type << endl;
    }
}

// enum Var_state {
//     LOCAL, 
//     OUTER
// };

// struct Var_data {
//     string type;
//     size_t line_decl;
//     size_t line_def;
//     size_t line_dest;
// };
// maps name of variable -> variable data
// we mainly need to know the type and region of validity
// a variable can be non-existant, declared, defined, and used
// existance - exists by the end of assignment operation, or decl
// declared - exists between decl and an assignment
// x = increment(x) + multiply_by_2(x)   # assignment order matters now!!! - either throw error or decide rule
// repeated variables get $# attached to the end
// struct Symbol_table {
//     //parent scope
//     unordered_map<string, Var_data> locals;
// };
Function_table make_ft(){
    // Type_table tt;
    // tt.add("int", Type_info("int", Supertype::PRIMITIVE));
    // tt.add("float", Type_info("double", Supertype::PRIMITIVE));
    // tt.add("bool", Type_info("bool", Supertype::PRIMITIVE));
    Function_table ft;

    ft.add(Signature{"+", vector<string>{"int", "int"}, "int"});
    ft.add(Signature{"+", vector<string>{"float", "int"}, "float"});
    ft.add(Signature{"+", vector<string>{"int", "float"}, "float"});
    ft.add(Signature{"+", vector<string>{"float", "float"}, "float"});
    ft.add(Signature{"pound", vector<string>{"int"}, "float"});
    ft.add(Signature{"round", vector<string>{"float"}, "int"});
    ft.add(Signature{"rms", vector<string>{"float", "float"}, "float"});
    ft.add(Signature{"print", vector<string>{"int"}, "none"});
    ft.add(Signature{"print", vector<string>{"float"}, "none"});
    ft.add(Signature{"print", vector<string>{"char"}, ""});
    ft.add(Signature{"print", vector<string>{"bool"}, "none"});

    // cout << ft.rtype("+", vector<string>{"int", "int"}).valid << endl;
    // cout << ft.rtype("+", vector<string>{"int", "float"}).valid << endl;
    // cout << ft.rtype("+", vector<string>{"float", "float"}).valid << endl;
    // cout << ft.rtype("+", vector<string>{"float", "int"}).valid << endl;
    return ft;
}


VarInfo::VarInfo(){
    decl = INT_MAX;
    def = INT_MAX;
    dest = INT_MAX;
}

SymbolTable::SymbolTable(){}

SymbolTable::SymbolTable(SymbolTable* outer_scope){
    parent = outer_scope;
    //outer_line = line;
}
// check if variable is in table
// if not in table, check parent scope recursively
// copy data into current/intermediate scopes if not found
// finally, return false if not found
bool SymbolTable::search_and_copy(string name){
    if(table.count(name) == 1)
        return true;
    else if(parent == nullptr)
        return false;
    else {
        // if(parent->search_and_copy(name) == true){
        //     // bring entry into current scope
        //     VarInfo vi = parent->table[name];
        //     vi.decl = -1;
        //     if(vi.def != INT_MAX){
        //         vi.def = -1;
        //     }
        //     if(vi.dest != INT_MAX){
        //         vi.dest = -1;
        //     }
        //     table.insert({name, vi});
        //     return true;
        // } else {
        //     return false;
        // }
        return false;
    }
}
// a variable can be used if
// it is declared, defined, but not destroyed
// def < line <= dest, type must exist
// bool is_usable(string name, int line){
//     bool exists = search_and_copy(name);
//     if(exists == false){
//         return false;
//     }
//     VarInfo vi = table[name];
//     if(vi.type == ""){
//         return false;
//     }
//     if(vi.def < line && line <= vi.dest){
//         return true;
//     } else {
//         return false;
//     } 
// }

Var_status SymbolTable::getStatus(string name){
    if(search_and_copy(name) == false){
        return MISSING;
    }
    VarInfo vi = table[name];
    if(vi.dest != INT_MAX){
        return DESTROYED;
    } else if(vi.def != INT_MAX){
        return DEFINED;
    } else if(vi.type == ""){
        return DECLARED_NO_TYPE;
    } else {
        return DECLARED_W_TYPE;
    }
}

// a variable is usable if its defined but not destroyed
// crucially, we assume lines are parsed in order
bool SymbolTable::is_usable(string name){
    Var_status status = getStatus(name);
    return status == DEFINED;
}

// if type is usable, returns the type
// otherwise returns false
Option<string> SymbolTable::use_var(string name){
    if(is_usable(name)){
        return Option<string>(table[name].type);
    } else {
        return Option<string>(false);
    }
}

// get the type of a variable
// returns false if undeclared/undefined
Option<string> SymbolTable::getType(string name){
    if(search_and_copy(name)==true){
        string type = table[name].type;
        if(type != "")
            return Option<string>{type};
        else {
            return Option<string>(false);
        }

    } else {
        return Option<string>{false};
    }
}
// declare a variable on a line
// if type is empty string, then its considered unspecified
// if variable already exists, return false
bool SymbolTable::declare(string name, int line){
    if(table.count(name)==1){
        return false;
    }
    VarInfo vi;
    vi.decl = line;
    vi.def = INT_MAX;
    vi.dest = INT_MAX;
    table.insert({name, vi});
    return true;
}

// declare a variable on a line
// if type is empty string, then its considered unspecified
// if variable already exists, return false
bool SymbolTable::declare_with_type(string name, int line, string type){
    if(table.count(name)==1){
        return false;
    }
    VarInfo vi;
    vi.decl = line;
    vi.def = INT_MAX;
    vi.dest = INT_MAX;
    vi.type = type;
    table.insert({name, vi});
    return true;
}

// if variable doesn't exist, declare + define
// if variable is already defined, check type
// return 0 on success
// return 1 if types don't match declared type 
// return 2 if types don't match defined type
// return 3 if variable is already destroyed
int SymbolTable::assign(string name, int line, string type){
    bool exists = search_and_copy(name);
    if(!exists){
        VarInfo vi;
        vi.decl = line;
        vi.def = line;
        vi.dest = INT_MAX;
        vi.type = type;
        table.insert({name, vi});
        return 0;
    } 

    Var_status status = getStatus(name);
    VarInfo& vi = table[name];
    switch(status){
        case DECLARED_NO_TYPE: 
            vi.def = line;
            vi.type = type;
        break;
        case DECLARED_W_TYPE:
            vi.def = line;
            if(vi.type != type){
                return 1;
            }
        break;
        case DEFINED:
            if(vi.type != type){
                return 2;
            }
        break;
        case DESTROYED:
            return 3;
        break;
        default: 
            print("compiler error - missing var");
            return 4;
    }
    return 0;
}

// set destruction line
// returns false if the variable is already destroyed
bool SymbolTable::destroy(string name, int line){
    VarInfo& vi = table[name];
    if(vi.dest != INT_MAX){
        return false;
    }
    vi.dest = line;
    return true;
}



