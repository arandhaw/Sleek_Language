#pragma once
#include "ast_basics.h"

/////////// Function_table
class Function_table {
    private:
        struct Func_Data {
            Signature signature;
            string cname;
        };
        unordered_multimap<string, Func_Data> table;
    public:
    
    // add to function table
    // return false if a duplicate signature exists
    // the cname is the name of the corresponding c function
    // if no name is provided, it will be automatically generated with $NAME
    bool add(Signature f);
    bool add(Signature f, const string& cname);
    // return type of function
    Option<Type> rtype(const string& name, const vector<Type>& params);

    // search the table using the name and parameters
    Option<Signature> search(const string& name, const vector<Type>& params);
    // obtain the cname "codegen name" of the function
    // precondition: the function exists
    string cname(const string& name, const vector<Expression>& args);
    // return number of function with said name
    int check(const string& name);
    
    void printAll();
} func_table;

