#pragma once
// #include "parser_globals.h"

// struct ParseArrayLiteral {
//     bool valid;
//     Array_literal array_literal;
//     string cname;
//     Iter end;
// };

// struct SubArray {
//     bool valid;
//     Iter end;
//     vector<size_t> dims;    // dimensions of subarray (reverse order)
// };

// add a tuple to the type table
// this function assumed structs/enums have already been parsed
Type register_tuple(vector<Type> types, bool complete){
    string tuple_type_name = "(";    
    string tuple_cname = "$t";        
    Type ret; 
    for(const auto& t : types){
        if(tuple_type_name != "("){
            tuple_type_name += ",";
        }
        tuple_type_name += t.to_string();
        tuple_cname += "_" + t.cname();
    }
    tuple_type_name += ")";
    tuple_cname += "$";
    
    if(type_table.contains(tuple_type_name)){
        return type_table.get_type(tuple_type_name);
    
    } else {
        TypeInfo ti;
        ti.name = tuple_type_name;
        ti.cname = tuple_cname;
        ti.supertype = TUPLE;
        for(int i = 0; i < types.size(); i++)
            ti.fields.push_back(field{to_string(i), types[i]});
        ti.size = 0;
        if(complete){
            for(const auto& t : types)
                ti.size += type_table.get_info(t).size;
            ti.sized = true;
            ti.destructor = false;
            ti.copyable = true; 
            for(const auto& t : types)
                ti.copyable = ti.copyable && type_table.get_info(t).copyable;
        } 
        // add the type to the type table
        Type type = type_table.insert(ti);
        // tuple_table.insert({type, Tuple_type{vector<Type>{types}}});

        if(complete){
            // add the type to the declaration order
            decl_order.push_back(type);
        } 
        return type;
    } 
}


// assume i is pointing to a square bracket
ParseArrayLiteral parseArrayLiteral(SymbolTable& vars, Iter it){
    ParseArrayLiteral ret; ret.valid = true;
    SubArray result = parseArrayRecurse(vars, it, ret.array_literal.elements);
    
    if(result.valid == false){ 
        print("Error while parsing array literal");
        ret.valid = false; return ret; 
    } 
    ret.end = result.end;
    
    // reverse the dims vector
    // the recursive function determines dimensions in reverse order for efficiency
    reverse(result.dims.begin(), result.dims.end());
    ret.array_literal.dims = result.dims;
    ret.array_literal.element_type = ret.array_literal.elements[0].type;
    // check each element has same type
    for(const Expression& e : ret.array_literal.elements){
        if(e.type != ret.array_literal.element_type){
            print("Error - array element types are not all the same");
            ret.valid = false; return ret;
        }
    }
    // generate cname - for an N-D array with dimensions (n1, n2, ...)
    // name is $arrayND<type, n1, n2, ...>
    ret.cname = "$array" + to_string(ret.array_literal.dims.size()) + "D<"
            + ret.array_literal.element_type.cname();
    for(size_t dim : ret.array_literal.dims){
        ret.cname += ", " + to_string(dim);
    }
    ret.cname += ">";
    return ret;
}

SubArray parseArrayRecurse(SymbolTable& vars, Iter it, vector<Expression>& elements){
    SubArray ret;
    ret.valid = true;
    it++;
    size_t num_elements = 0;
    vector<size_t> first_element_dims;
    while(true){
        // subarray
        if(it->value == "["){
            SubArray sa = parseArrayRecurse(vars, it, elements);
            if(sa.valid == false){ ret.valid = false; return ret; }
            // check subarrays are same size
            if(num_elements == 0){
                first_element_dims = sa.dims;
            } else if(sa.dims != first_element_dims){
                print("Error - subarrays must have the same size");
                ret.valid = false; return ret;
            }
            it = sa.end;
        // unexpected closing bracket (error)
        } else if(it->value == "]"){
            if(num_elements == 0){
                print("Error: arrays with zero size are not allowed");
            } else {
                print("Error: unexpected ] while parsing array");
            }
            ret.valid = false; return ret;
        // normal element - possibly a expression
        } else {
            ParseSimpleExpr result = parseSimpleExpr(vars, it);
            if(result.valid == false){ ret.valid = false; return ret; }
            elements.push_back(result.expr);
            if(first_element_dims.size() != 0){
                print("Error - subarrays must have the same size (2)");
                ret.valid = false; return ret;
            }
            it = result.end;
        }
        num_elements++;
        // the next token should be , or ]
        if(it->type == COMMA){
            it++;
        } else if(it->value == "]"){
            ret.end = it + 1;
            break;
        } else {
            print("Error: syntax error in declaration of array");
            ret.valid = false; return ret;
        }
    }
    ret.dims = first_element_dims;
    ret.dims.push_back(num_elements);
    return ret;
}
