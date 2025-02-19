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
///////////////////////////////////////
// Methods for symboltable
///////////////////////////////////////


#include "symboltable.cpp"

///////////////
// Methods for printing expressions
//////////////
void printExpression(const Expression& e){
    Expr_value expr = e.expr;
    cout << e.type << " : ";
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
        cout << f.param_types.at(i) << " " << f.param_names.at(i);
        if(i != len - 1){
            cout << ", ";
        }
    }
    cout << ") -> " << f.return_type << "\n";
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