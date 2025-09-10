#pragma once

#include <string>
#include <unordered_map>
#include <variant>

#include "utils.cpp"

using namespace std;

// represent types in the program
// note: type_id is a unique identifier for each type
// in order to be instantiated, the type_id must be obtained from type table
// (implicit copy constructor allows you to copy types)

struct Type {
    size_t type_id; // unique identifier for every concrete type
    string to_string() const;   // get string from type_table

    // implement == and !=
    bool operator==(const Type& other) const {
        return type_id == other.type_id;
    }
    bool operator!=(const Type& other) const {
        return type_id != other.type_id;
    }
};

std::ostream& operator<<(std::ostream& os, const Type& obj) {
    os << obj.to_string();
    return os;
}

/////////// Symboltable //////////////////
struct VarInfo {
    int decl;
    int def;
    int dest;
    bool type_defined;
    Type type;
    VarInfo();
};

enum Var_status {
    MISSING,
    DECLARED_NO_TYPE,
    DECLARED_W_TYPE, 
    DEFINED, 
    DESTROYED
};

// each scope has a symbol table
// it is filled out line by line
// it also has a parent, which may be null
// A variable can be used on the line after it is defined to the line it is destroyed
class SymbolTable {
    private:
        
    public:
        SymbolTable *parent = nullptr;
        Type returnType;
        bool hasReturn = false;
        // should only be read, modifications should be done through functions
        unordered_map<string, VarInfo> table;
        
        // default constructor, no outer scope
        SymbolTable();

        // set parent pointer to outer_scope
        SymbolTable(SymbolTable* outer_scope);

        // check if variable is in table
        // if not in table, check parent scope recursively
        // copy data into current/intermediate scopes if not found
        // finally, return false if not found
        bool search_and_copy(string name);
        
        // get the current status of a variable 
        // (assume lines are parsed in order)
        Var_status getStatus(string name);
        
        // a variable is usable if its defined but not destroyed
        // crucially, we assume lines are parsed in order
        bool is_usable(string name);

        // check if type is usable
        // returns the type if usable
        Option<Type> use_var(string name);
        
        // get the type of a variable
        // returns false if undeclared/undefined
        Option<Type> getType(string name);

        // declare a variable on a line
        // if type is empty string, then its considered unspecified
        // if variable already exists, return false
        bool declare(string name, int line);

        // declare a variable on a line
        // if type is empty string, then its considered unspecified
        // if variable already exists, return false
        bool declare_with_type(string name, int line, Type type);

        // if variable doesn't exist, declare + define
        // if variable is already defined, check type
        // return 0 on success
        // return 1 if types don't match declared type 
        // return 2 if types don't match defined type
        // return 3 if variable is already destroyed
        int assign(string name, int line, Type type);

        // set destruction line
        // returns 0 if sucessful 
        // return 1 if the variable is already destroyed
        // returns 2 if the variable doesn't exist
        int destroy(string name, int line);
};



/////////// Entity Table ///////////////////
enum Entity {
    UNDEFINED,
    FUNCTION, 
    STRUCT, 
    ENUM, 
    VARIANT,
    PRIMITIVE_TYPE,
};

string EntityNames[] = {"undefined", "function", "struct", 
        "enum", "variant", "primitive_type"};
int EntityNamesLength = sizeof(EntityNames)/sizeof(string);

string toString(Entity x){
    if(x < 0 || x >= EntityNamesLength){
        return "DEEP ERROR - unknown entity";
    }
    return EntityNames[x];
}

class EntityTable {
    private:
        unordered_map<string, Entity> table;
    public:
        bool add(string x, Entity y){
            if(table.count(x) != 0){
                return false;
            }
            table.insert({x, y});
            return true;
        }

        Entity get(string x){
            if(table.count(x) == 0){
                return UNDEFINED;
            } else {
                return table[x];
            }
        }
};

struct Type_ast;

struct Named_type {
};

struct Tuple_type {
    vector<Type_ast> elements;
};

struct Array_type {
    size_t array_size;
    vector<size_t> shape;
    Type_ast *type_ptr;
};

struct Type_ast {
    variant<Named_type, Tuple_type, Array_type> info;
};

struct TypeInfo {
    string name;
    size_t size;
    Entity supertype;
    Type_ast structure;
    bool copyable;
    bool sized;
    bool destructor;
};

// check if 
class TypeTable {
    private:
        size_t type_id_counter;
        unordered_map<size_t, TypeInfo> table;
        unordered_map<string, size_t> str_to_typeid;
    public:
        bool contains(string type_str){
            return str_to_typeid.count(type_str);
        }
        bool contains(Type t){
            return table.count(t.type_id);
        }

        Type insert(const TypeInfo& ti){
            // already exists in type table
            if(str_to_typeid.count(ti.name) == 1){
                return Type{str_to_typeid.at(ti.name)};
            } else {
                table.insert({type_id_counter, ti});
                str_to_typeid.insert({ti.name, type_id_counter});
                type_id_counter++;
                return Type{type_id_counter-1};
            }
        }

        TypeInfo get_info(Type t){
            return table.at(t.type_id);
        }

        Type get_type(string x){
            return Type{ str_to_typeid.at(x) };
        }

        string get_string(Type t){
            return table.at(t.type_id).name;
        }
};

TypeTable type_table;

// methods of type class
string Type::to_string() const {
    if(!type_table.contains(*this)){
        return "UNKNOWN-TYPE";
    }
    return type_table.get_string(*this);
}

/// For struct table
struct struct_element {
    string type;
    string field;
};

struct StructInfo {
    vector<struct_element> elements;
    bool size_known;
    bool flag_for_algo;
};
// this will be used in struct_table

