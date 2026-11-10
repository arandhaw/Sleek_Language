#pragma once
#include "function_table.h"

bool Function_table::add(Signature f){
    if(entity_table.table.count(f.name) == 0){
        entity_table.add(f.name, FUNCTION);
    }
    Option<Signature> duplicate = search(f.name, f.param_types);
    if(duplicate.valid == true){
        print("Error in functiontable.add", f.name, f.param_types);
        return false;
    }
    table.insert({f.name, Func_Data{f, string("$" + f.name)}});
    return true;
}
// add to function table
// return false if a duplicate signature exists
bool Function_table::add(Signature f, const string& cname){
    Option<Signature> duplicate = search(f.name, f.param_types);
    if(duplicate.valid == true){
        print("Error in functiontable.add", f.name, f.param_types);
        return false;
    }
    table.insert({f.name, Func_Data{f, cname}});
    return true;
}

// return type of function
Option<Type> Function_table::rtype(const string& name, const vector<Type>& params){
    Option<Signature> res = search(name, params);
    Option<Type> ret(true);
    if(res.valid == false){ ret.valid = false; return ret; }
    else {
        ret.result = res.result.return_type;
        return ret;
    }
}

// search the table using the name and parameters
Option<Signature> Function_table::search(const string& name, const vector<Type>& params){
    Option<Signature> result(false);
    auto it = table.equal_range( name );
    for(auto i = it.first; i != it.second; ++i){
        vector<Type> i_params = i->second.signature.param_types;
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
            result.result = i->second.signature;
            result.valid = true;
            break;
        }
    }
    return result;
}
// return number of function with said name
int Function_table::check(const string& name){
    return table.count(name);
}

string Function_table::cname(const string& name, const vector<Expression>& args){
    Option<string> result(false);
    auto it = table.equal_range( name );
    for(auto i = it.first; i != it.second; ++i){
        vector<Type> i_params = i->second.signature.param_types;
        if(args.size() != i_params.size()){
            continue;
        }
        // compare each parameter type
        bool equal = true;
        for(int i = 0; i < args.size(); i++){
            if(args[i].type != i_params[i]){
                equal = false;
                break;
            }
        }
        if(equal){
            result.result = i->second.cname;
            result.valid = true;
            break;
        }
    }
    if(result.valid == false){
        cout << "Deep Error in codegen - function not found" << endl;
    }
    return result.result;
}

void Function_table::printAll(){
    cout << "Function table:" << endl;
    for(pair<string, Func_Data> i : table){
        cout << i.second.signature.name << "(";
        for(auto& j : i.second.signature.param_types){
            cout << j.to_string() << ", ";
        }
        cout << "\b\b) -> " << i.second.signature.return_type.to_string() << endl;
    }
}

