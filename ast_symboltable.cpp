#pragma once

#include "ast_symboltable.h"
// TypeTable - all types in current module
// holds types as unordered map: str -> type_info object
// operations: add, check, info

///////////////////////////////////////////////////////////////////////////////////////////////////


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
        if(parent->search_and_copy(name) == true){
            // bring entry into current scope
            VarInfo vi = parent->table[name];
            vi.decl = -1;
            if(vi.def != INT_MAX){
                vi.def = -1;
            }
            if(vi.dest != INT_MAX){
                vi.dest = -1;
            }
            table.insert({name, vi});
            return true;
        } else {
            return false;
        }
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
    } else if(vi.type_defined){
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
Option<Type> SymbolTable::use_var(string name){
    if(is_usable(name)){
        return Option<Type>(table[name].type);
    } else {
        return Option<Type>(false);
    }
}

// get the type of a variable
// returns false if undeclared/undefined
Option<Type> SymbolTable::getType(string name){
    if(search_and_copy(name)==true){
        VarInfo& vi = table[name];
        if(vi.type_defined)
            return Option<Type>{vi.type};
        else {
            return Option<Type>(false);
        }

    } else {
        return Option<Type>{false};
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
    vi.type_defined = false;
    table.insert({name, vi});
    {
        VarInfo vi = table[name];
        print(vi.decl, vi.def, vi.dest);
    }
    return true;
}

// declare a variable on a line
// if type is empty string, then its considered unspecified
// if variable already exists, return false
bool SymbolTable::declare_with_type(string name, int line, Type type){
    if(table.count(name)==1){
        return false;
    }
    VarInfo vi;
    vi.decl = line;
    vi.def = INT_MAX;
    vi.dest = INT_MAX;
    vi.type_defined = true;
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
int SymbolTable::assign(string name, int line, Type type){
    bool exists = search_and_copy(name);
    if(!exists){
        VarInfo vi;
        vi.decl = line;
        vi.def = line;
        vi.dest = INT_MAX;
        vi.type = type;
        vi.type_defined = true;
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
    {
        VarInfo vi = table[name];
        print(vi.decl, vi.def, vi.dest);
    }
    return 0;
}

// set destruction line
// returns 0 if sucessful 
// return 1 if the variable is already destroyed
// returns 2 if the variable doesn't exist
int SymbolTable::destroy(string name, int line){
    bool exists = search_and_copy(name);
    if(!exists){
        return 2;
    } 
    VarInfo& vi = table[name];
    if(vi.dest != INT_MAX){
        return 1;
    }
    vi.dest = line;
    {
        VarInfo vi = table[name];
        print(vi.decl, vi.def, vi.dest);
    }
    return 0;
}



