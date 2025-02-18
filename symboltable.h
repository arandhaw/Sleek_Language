// maps name of variable -> variable data
// we mainly need to know the type and region of validity
// a variable can be non-existant, declared, defined, and used
// existance - exists by the end of assignment operation, or decl
// declared - exists between decl and an assignment

// each scope has a parent scope, and child scopes they can't see
// each scope has variables, and outside variables
// decl, def, dest -- 
// -1 means happened in parent scope
// INT_MAX means hasn't happened yet
// basic algorithm: note line where variable is declared, defined, destroyed
// note type of variable
// if an variable declared outside is redeclared

struct VarInfo{
    int decl;
    int def;
    int dest;
    string type;
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
    public:
        SymbolTable *parent = nullptr;
        // int outer_line = -1;
        unordered_map<string, VarInfo> table;

        // default constructor
        SymbolTable();

        SymbolTable(SymbolTable* outer_scope);
        // check if variable is in table
        // if not in table, check parent scope recursively
        // copy data into current/intermediate scopes if not found
        // finally, return false if not found
        bool search_and_copy(string name);
        
        Var_status getStatus(string name);

        // a variable is usable if its defined but not destroyed
        // crucially, we assume lines are parsed in order
        bool is_usable(string name);

        // if type is usable, returns the type
        // otherwise returns false
        Option<string> use_var(string name);
        // get the type of a variable
        // returns false if undeclared/undefined
        Option<string> getType(string name);
        // declare a variable on a line
        // if type is empty string, then its considered unspecified
        // if variable already exists, return false
        bool declare(string name, int line);

        // declare a variable on a line
        // if type is empty string, then its considered unspecified
        // if variable already exists, return false
        bool declare_with_type(string name, int line, string type);

        // if variable doesn't exist, declare + define
        // if variable is already defined, check type
        // return 0 on success
        // return 1 if types don't match declared type 
        // return 2 if types don't match defined type
        // return 3 if variable is already destroyed
        int assign(string name, int line, string type);

        // set destruction line
        // returns false if the variable is already destroyed
        bool destroy(string name, int line);
};


