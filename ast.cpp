#pragma once

#include "ast.h"
////////////////////
// class function definitions
////////////////////

Unary_expr::Unary_expr(Token op, Expression middle) {
    this->op = op;
    this->middle = new Expression;  // allocate memory for an Expression
    *(this->middle) = middle;       // copy passed arguement
}

Binary_expr::Binary_expr(Token op, Expression left, Expression right){
    this->op = op;
    this->left = new Expression;  // allocate memory for an Expression
    *(this->left) = left;       // copy passed arguement
    this->right = new Expression;  // allocate memory for an Expression
    *(this->right) = right;       // copy passed arguement
}


Function_call::Function_call(Token name) {
    this->name = name;
}

Array_index::Array_index(Token name) {
    this->name = name;
}
/// function table
// function table

bool Function_table::add(Signature f){
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

//// print type
// void printType(const Type_ast& t){
//     if(HAS<Named_type>(t.info)){
//         const Named_type& nt = get<Named_type>(t.info);
//         print_raw(nt.name);
//     } else if(HAS<Array_type>(t.info)){
//         const Array_type& arr = get<Array_type>(t.info);
//         print_raw("[");
//         printType(*arr.type_ptr);
//         for(int i = 0; i < arr.shape.size(); i++){
//             print_raw(", ");
//             print_raw(arr.shape[i]);
//         }
//         print_raw("]");
//     } else if(HAS<Tuple_type>(t.info)){
//         const Tuple_type& tup = get<Tuple_type>(t.info);
//         print_raw("(");
//         print_raw(tup.elements[0]);
//         if(tup.elements.size() == 1){
//             print_raw(",");
//         }
//         for(int i = 1; i < tup.elements.size(); i++){
//             print_raw(", ");
//             printType(tup.elements[i]);
//         }
//         print_raw(")");
//     }
// }

///////////////
// Methods for printing expressions
//////////////
void printExpression(const Expression& e){
    Expr_value expr = e.expr;
    cout << e.type.to_string() << " : ";
    if(holds_alternative<Literal>(expr)){
        cout << get<Literal>(expr).value.value;
    } else if(holds_alternative<Variable>(expr)){
        cout << get<Variable>(expr).name.value;
    } else if(holds_alternative<Function_call>(expr)){
        Function_call ex = get<Function_call>(expr);
        if(ex.args.size() != 0){
            cout << ex.name.value << "(";
            for(auto i : ex.args){
                printExpression(i);
                cout << ", ";
            }
            cout << "\b\b";
            cout << ")";
        } else {
            cout << ex.name.value << "()";
        }
    } else if(holds_alternative<Array_index>(expr)){
        Array_index ex = get<Array_index>(expr);
        cout << ex.name.value << "(";
        for(auto i : ex.args){
            printExpression(i);
            cout << ", ";
        }
        cout << "\b\b";
        cout << ")";
    } else if(holds_alternative<Binary_expr>(expr)){
        Binary_expr ex = get<Binary_expr>(expr);
        if(ex.op.value == "("){
            printExpression(*ex.left);
            cout << "(";
            printExpression(*ex.right);
            cout << ")";
        } else if (ex.op.value == "["){
            printExpression(*ex.left);
            cout << "[";
            printExpression(*ex.right);
            cout << "]";
        } else {
            cout << "(";
            printExpression(*ex.left);
            cout << " " << ex.op.value << " ";
            printExpression(*ex.right);
            cout << ")";
        }
    } else if(holds_alternative<Unary_expr>(expr)){
        Unary_expr ex = get<Unary_expr>(expr);
        cout << "(";
        cout << ex.op.value;
        printExpression(*ex.middle);
        cout << ")";
    }
}

void printBasicLine(const Basic_Line& line){
    if(holds_alternative<Expression>(line)){
        printExpression(get<Expression>(line));
    } else if(holds_alternative<Assignment>(line)){
        Assignment a = get<Assignment>(line);
        printExpression(a.left);
        cout << " " << a.op.value << " ";
        printExpression(a.right);
    } else if(holds_alternative<Multi_Assignment>(line)){
        Multi_Assignment ma =  get<Multi_Assignment>(line);
        for(auto& i : ma.left){
            printExpression(i);
            cout << " = ";
        }
        printExpression(ma.right);
    } else if(holds_alternative<Empty>(line)){
        cout << "Empty";
    }
    cout << endl;
}

void printCodeblock(const Codeblock& cb){
    print("{");
    for(const Line& line : cb.lines){
        if(holds_alternative<Basic_Line>(line)){
            printBasicLine(get<Basic_Line>(line));
        } else if(holds_alternative<Codeblock>(line)){
            printCodeblock(get<Codeblock>(line));
        }
    }
    print("Symboltable:");
    for(auto& i : cb.vars.table){
        print(i.first, ':', i.second.decl, i.second.def, i.second.dest);
    }
    print("}");
}

void printFunction(const Function& f){
    cout << f.name << "(";
    int len = f.param_names.size();
    for(int i = 0; i < len; i++){
        cout << f.param_types.at(i).to_string() << " " << f.param_names.at(i);
        if(i != len - 1){
            cout << ", ";
        }
    }
    print(") ->", f.return_type.to_string());
    printCodeblock(f.code);
}

void printProgram(const Program& p){
    printFunction(p.main);
    for(auto f = p.functions.begin(); f != p.functions.end(); f++){
        printFunction(f->second);
    }
}

// int main(){
//     return 0;
// }