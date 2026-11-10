#pragma once
// #include "parser_globals.h"


// struct ParseBasicLine {
//     bool valid;
//     bool empty;
//     Basic_Line line;
//     Iter end;
// };

// parse a basic line (assignment/expression ended with EOL
// if line ends with EOL, end is 1 token after EOL
// calls parseSimpleExpression
ParseBasicLine parseBasicLine(SymbolTable& vars, int line, Iter it){
    ParseBasicLine ret;
    ret.valid = true;
    ret.empty = false;
    // empty line - line with EOL only, or simply nothing
    if(it->type == EOL){
        ret.valid = true;
        ret.empty = true;
        ret.line = Empty();
        ret.end = it + 1;
        return ret;
    // assignment operation like x = ...
    } else if(it->type == NAME && (it+1)->value == "="){
        ParseSimpleExpr second = parseSimpleExpr(vars, it+2);
        if(second.valid == false){
            cout << "Invalid expression in parseBasicLine" << endl;
            ret.valid = false;
            return ret;
        }
        if(HAS<Variable>(second.expr.expr) && type_table.get_info(second.expr.type).copyable == false ){
            // variable has been moved - destroy it!
            vars.destroy(second.start->value, line);
        }
        
        // Assignment a = {first.expr, second.expr, op};
        int result = vars.assign(it->value, line, second.expr.type);
        if(result != 0){
            cout << "Error in assignment operation" << endl;
            ret.valid = false; return ret;
        } 
        Expression single = {Variable{*it}, second.expr.type};
        Assignment a{single, second.expr, *(it + 1)};
        ret.line = a;
        if(second.end->type == EOL){ ret.end = second.end + 1; }
        else if(second.end->value == "}"){ ret.end = second.end; }
        else { cout << "Invalid token at end of expression" << endl; ret.valid = false; }
        return ret;

    // line like x : <type> = ...
    } else if(it->type == NAME && (it+1)->value == ":"){
        // var name
        Var_status vs = vars.getStatus(it->value);
        if(vs != MISSING){
            print("Error - variable", it->value, "already exists, you can't redeclare its type");
            ret.valid = false; return ret;
        }
        Iter var_name = it;
        // var type
        ParseTypeOption type_option = parseType(it+2);
        if(type_option.valid = false){
            print("Error - type signature invalid");
            ret.valid = false; return ret;
        } 
        it = type_option.end;
        Iter op_iter = it;
        Type type = type_option.type;
        if(it->value != "="){
            print("Error in variable assignment, expected = after type signature");
            ret.valid = false; return ret;
        }
        // parse right side of expr
        ParseSimpleExpr second = parseSimpleExpr(vars, it+1);
        if(second.valid == false){
            cout << "Invalid expression in parseBasicLine" << endl;
            ret.valid = false;
            return ret;
        }
        
        if(second.expr.type != type){
            print("Error in assignment, type does not match signature");
            print("Left side type:", type, " --- Right side type:", second.expr.type);
            ret.valid = false; return ret;
        }

        if(HAS<Variable>(second.expr.expr) && type_table.get_info(second.expr.type).copyable == false ){
            // variable has been moved - destroy it!
            vars.destroy(second.start->value, line);
        }
        
        // Assignment a = {first.expr, second.expr, op};
        int result = vars.assign(var_name->value, line, second.expr.type);
        if(result != 0){
            cout << "Error in assignment operation" << endl;
            ret.valid = false; return ret;
        } 
        Expression single = {Variable{*var_name}, second.expr.type};
        Assignment a{single, second.expr, *op_iter};
        ret.line = a;
        if(second.end->type == EOL){ ret.end = second.end + 1; }
        else if(second.end->value == "}"){ ret.end = second.end; }
        else { cout << "Invalid token at end of expression" << endl; ret.valid = false; }
        return ret;
    }
    // expression or left-side of assignment
    ParseSimpleExpr first_optional = parseSimpleExpr(vars, it);
    if(first_optional.valid == false){
        cout << "Invalid expression in parseBasicLine" << endl;
        ret.valid = false;
        return ret;
    }

    // if line has no equals sign, is expression
    // line is just <expression>
    if(first_optional.end->type == EOL){
        ret.end = first_optional.end + 1;
        ret.line = first_optional.expr;
        return ret;
    } 
    // must be assignment expression like += or -=, or error
    if(first_optional.end->type != ASSIGNMENT){
        ret.valid = false;
        cout << "Error: Expected assignment or newline at end of expression" << endl;
        return ret;
    }
    it = first_optional.end + 1;
    // parse the second half
    ParseSimpleExpr second_optional = parseSimpleExpr(vars, it);
    if(second_optional.valid == false){
        print("Invalid right side of assignment");
        ret.valid = false;
        return ret;
    }
    if(second_optional.end->type != EOL){
        print("Error in line parsing: unexpected token at end of line:", second_optional.end->toString());
        ret.valid = false;
        return ret;
    }
    // for convenience
    Token assign_op = *first_optional.end;
    Expression first = first_optional.expr;
    Expression second = second_optional.expr;

    // parse special assignment (e.g., x += y)
    if(assign_op.value != "="){
        if(assign_op.value != "+=" && assign_op.value != "-=" && assign_op.value != "*=" && assign_op.value != "/="){
            ret.valid = false; 
            print("Invalid operator", assign_op.value);
            return ret;
        }
        // separate += into = and + operations
        Token equals = assign_op; equals.value = "=";
        Token op = assign_op; op.value = assign_op.value.substr(0, 1);
        //// check validity, e.g., x += y -> check x + y is valid
        Option<Type> res = func_table.rtype(op.value, vector<Type>{first.type, second.type});
        if(res.valid == false){ 
            print("Type error - no operator defined for", first.type, assign_op.toString(), second.type); 
            ret.valid = false;
            return ret;
        }
        Type type = res.result;
        Expression ex = Expression(Binary_expr{op, first, second}, type);
        // assignment 
        int result = vars.assign(first_optional.start->value, line, second.type);
        if(result != 0){
            print("Error in assignment operation");
            ret.valid = false; return ret;
        } 
        Assignment a{first, ex, equals};
        ret.end = second_optional.end + 1;
        ret.line = a;
        return ret;
    // line like <expr> = <expr>
    } else {
        // conduct checks
        // check 1: types must match
        if(first.type != second.type){
            print("Error, types don't match in assignment operation");
            ret.valid = false; return ret;
        }
        // check 2: left side must be assignmable (l-value)
        // left side must be variable, array index, or field access 
        if(HAS<Variable>(first.expr)){
            int result = vars.assign(first_optional.start->value, line, second.type);
            if(result != 0){
                print("Error in assignment operation");
                ret.valid = false; return ret;
            } 
        } else if(HAS<Field_access>(first.expr)){
            
        } else if (HAS<Array_index>(first.expr)) {
            auto arr_index = get<Array_index>(first.expr);
            // change from assignment to expression
            ret.line = Expression(Array_set(arr_index.name, arr_index.args, second), type_none);
            ret.end = second_optional.end + 1;
            return ret;
        } else {
            print("Error - left expression is not assignable");
            ret.valid = false; return ret;
        }

        Assignment a{first, second, assign_op};
        ret.line = a;
        ret.end = second_optional.end + 1;
        return ret;
    }
    
    // else if(second.end->value == "}"){
    //     Token op = *first.end;
    //     Assignment a = {first.expr, second.expr, op};
    //     ret.end = second.end;
    //     ret.line = a;
    //     //// add to symboltable, type check
    //     return ret;
    // }
    // handle multiassignment case
    // else if (second.end->value == "="){
    //     Multi_Assignment ma;
    //     ma.left.push_back(first.expr);
    //     ma.left.push_back(second.expr);
    //     it = second.end + 1;
    //     while(true){
    //         ParseSimpleExpr next = parseSimpleExpr(vars, it);
    //         if(next.valid == false){
    //             ret.valid = false;
    //             return ret;
    //         } 
    //         if(next.end->value == "="){
    //             it = next.end + 1;
    //             ma.left.push_back(next.expr);
    //         } else if (next.end->type == EOL || next.end->value == "}"){
    //             ma.right = next.expr;
    //             ret.end = next.end;
    //             break;
    //         } else {
    //             ret.valid = false;
    //             cout << "Unexpected end to multiassignment " << next.end->toString() << endl;
    //             return ret;
    //         }
    //     }
    //     ret.line = ma;
    //     return ret;
    // }
}