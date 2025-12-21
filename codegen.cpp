#pragma once

#include "codegen.h"

// globals
unordered_map<string, string> operator_fname = 
{{"or", "or"}, {"and", "and"}, {"==", "equal"}, 
// {"!="}, {"<="}, {">="}, {"<"}, {">"}},
{"+", "add"}, {"*", "mul"}, {"-", "sub"}, {"/", "div"},
{"//", "idiv"}, {"mod", "%"},
{"^", "exp"}, {"iexp", "**"},
{"!", "not"}, {"negate", "u-"},
{"&", "deref"}, {"++", "increment"}, {"--", "decrement"} };
// {"(", "[", "."}};


// any C program is composed of functions, unions, structs, enums, global variables
// C++ adds classes, templates, namespaces/modules 
void Codegen::genProgram(const Program& program){
    out("#include \"builtins.cpp\"");
    raw("\n");
    declareFunctions(program.functions);
    raw('\n');
    genMain(program.main);
    raw("\n");
    for(auto& f : program.functions){
        raw("\n");
        genFunction(f.second);
        raw("\n");
    }
}

void Codegen::declareFunctions(const unordered_map<string, Function>& functions){
    for(auto& f : functions){
        const Function& func = f.second;
        raw('\n');
        raw('$', func.return_type.to_string(), ' ', func.name, '(');
        int num_params = func.param_names.size();
        
        for(int i = 0; i < num_params - 1; i++){
            raw('$', func.param_types[i].to_string(), ' ', '$', func.param_names[i], ", ");
        }
        raw('$', func.param_types[num_params-1].to_string(), ' ', '$', func.param_names[num_params-1], ");");
        raw('\n');
    }
}

void Codegen::genFunction(const Function& func){
    raw('$', func.return_type.to_string(), ' ', '$' + func.name, '(');
    int num_params = func.param_names.size();
    
    for(int i = 0; i < num_params - 1; i++){
        raw('$', func.param_types[i].to_string(), ' ', func.param_names[i], ", ");
    }
    raw('$' + func.param_types[num_params-1].to_string(), ' ', func.param_names[num_params-1], "){\n");
    indent();
    genCodeblock(func.code);
    dindent();
    line("}");
}

void Codegen::genMain(const Function& main){
    line("int main(){");
    indent();
    genCodeblock(main.code);
    line("return 0;");
    dindent();
    line("}");
}

void Codegen::genCodeblock(const Codeblock& cb){
    for(int line_num = 1; line_num <= cb.lines.size(); line_num++){
        Line i = cb.lines[line_num-1];
        if(holds_alternative<Basic_Line>(i)){
            genBasicLine(line_num, get<Basic_Line>(i), cb.vars);
        } else if(holds_alternative<Codeblock>(i)){
            line("{");
            indent();
            genCodeblock(get<Codeblock>(i));
            dindent();
            line("}");
        } else if(holds_alternative<DeclVars>(i)){
            tab();
            DeclVars& x = get<DeclVars>(i); 
            for(Token& tok : x.variables){
                const string& name = tok.value;
                const string& type = cb.vars.table.at(name).type.to_string();
                raw('$', type, ' ', name, ';', ' ');
            }
            raw("\n");
        } else if(holds_alternative<DeleteVars>(i)){
            // TODO
        } else if(holds_alternative<IfBlock>(i)){
            genIfBlock(get<IfBlock>(i));
        } else if(holds_alternative<ReturnLine>(i)){
            ReturnLine rl = get<ReturnLine>(i);
            if(rl.noneReturn){
                line("return;");
            } else {
                tout("return ");
                genExpression(rl.expr);
                out(";\n");
            }
        }
    }
    // delete all unused variables
}

void Codegen::genIfBlock(const IfBlock& ifBlock){
    tout("if(");
    genExpression(ifBlock.conditions[0]);
    raw(".v){\n");
    indent();
    genCodeblock(ifBlock.branches[0]);
    dindent();
    tout("}");
    int num_elif = ifBlock.conditions.size() - 1;
    for(int i = 1; i <= num_elif; i++){
        raw(" else if(");
        genExpression(ifBlock.conditions[i]);
        raw(".v){\n");
        indent();
        genCodeblock(ifBlock.branches[i]);
        dindent();
        tout("}");
    }
    if(ifBlock.hasElse){
        raw(" else {\n");
        indent();
        genCodeblock(ifBlock.branches[num_elif + 1]);
        dindent();
        tout("}");
    }
    // TODO: deal with memory stuff (deleting variables)
    raw("\n");
}

void Codegen::genBasicLine(int line_num, const Basic_Line& line, const SymbolTable& vars){
    tab();
    if(holds_alternative<Expression>(line)){
        genExpression(get<Expression>(line));
    } else if(holds_alternative<Assignment>(line)){
        Assignment a = get<Assignment>(line);
        if(holds_alternative<Variable>(a.left.expr)){
            Variable x = get<Variable>(a.left.expr);
            string name = x.name.value;
            const VarInfo& info = vars.table.at(name);
            if(info.decl == line_num){
                raw('$', info.type.to_string(), ' ', x.name.value);
            } else {
                raw(x.name.value);
            }
        } else {
            genExpression(a.left);
        }
        
        raw(" ", a.op.value, " ");
        genExpression(a.right);
    } else if(holds_alternative<Multi_Assignment>(line)){
        Multi_Assignment ma =  get<Multi_Assignment>(line);
        for(auto& i : ma.left){
            genExpression(i);
            out(" = ");
        }
        genExpression(ma.right);
    } else if(holds_alternative<Empty>(line)){}
    raw(";\n");
}


void Codegen::genExpression(const Expression& e){
    Expr_value expr = e.expr;
    if(holds_alternative<Literal>(expr)){
        Token lit = get<Literal>(expr).value;
        
        switch(lit.type){
            case(INT_LITERAL): 
                raw("$int{", lit.value,"}");
            break;      
            case(FLOAT_LITERAL):         //contains decimal or scientific notation
                raw("$float{", lit.value,"}");
            break;
            case(CHAR_LITERAL):       // single quotes ''
            break;
            case(BOOL_LITERAL):       // true or false
                raw("$bool{", lit.value,"}"); 
            break;
            case(STRING_LITERAL):   
                raw("$string{", lit.value, ", ", lit.value.length()-2, "}");
            break;
            default:
            cout << "Deep error in codegen: literal unrecognized" << endl;
        }
        
    } else if(holds_alternative<Variable>(expr)){
        out(get<Variable>(expr).name.value);
    } else if(holds_alternative<Function_call>(expr)){

        Function_call ex = get<Function_call>(expr);
        string cname = func_table.cname(ex.name.value, ex.args);
        raw(cname);
        int num_args = ex.args.size();
        if(num_args != 0){
            out("(");
            for(int i = 0; i < num_args - 1; i++){
                genExpression(ex.args[i]);
                out(", ");
            }
            genExpression(ex.args[num_args - 1]);
            out(")");
        } else {
            raw(ex.name.value, "()");
        }
    }
    // else if(holds_alternative<Array_index>(expr)){
    //     Array_index ex = get<Array_index>(expr);
    //     cout << ex.name.value << "(";
    //     for(auto i : ex.args){
    //         printExpression(i);
    //         cout << ", ";
    //     }
    //     cout << "\b\b";
    //     cout << ")";
    else if(holds_alternative<Binary_expr>(expr)){
        Binary_expr bin = get<Binary_expr>(expr);
        string func_name = func_table.cname(bin.op.value, 
                        vector<Expression>{*bin.left, *bin.right});
        raw(func_name, "("); genExpression(*bin.left); 
        raw(", "); genExpression(*bin.right); raw(")");
    }
    else if(holds_alternative<Unary_expr>(expr)){
        Unary_expr unary = get<Unary_expr>(expr);
        string func_name = func_table.cname(unary.op.value, 
            vector<Expression>{*unary.middle});
        raw(func_name, "("); genExpression(*unary.middle); raw(")");
    }
    else {
        out("(work in progress)");
    }
}
