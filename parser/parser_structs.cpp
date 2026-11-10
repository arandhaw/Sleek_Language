#pragma once
// #include "parser_globals.h"

// struct ParseProgram {
//     bool valid;
//     Program result;
// };

// struct Entity_start {
//     Type name;
//     Iter start;
// };

Option<StructInfo> parseStruct(const Entity_start& obj){
    Iter it = obj.start;
    Option<StructInfo> ret(true);
    bool contains_user_defined_type = false;
    while(true){
        if(it->type == EOL){
            it++;
            continue;
        } 
        if(it->value == "}"){
            break;
        }
        ParseTypeOption result = parseIncompleteType(it);
        if(result.valid == false){
            print("Error parsing struct", obj.name, "- expected type. Line:");
            printErrorLine(it);
            ret.valid = false; return ret;
        }

        Type type = result.type;
        bool user_defined_type;
        TypeInfo& t_info = type_table.get_info(type);

        print(toString(t_info.supertype));
        if(t_info.supertype == PRIMITIVE_TYPE){
            user_defined_type = false;
        } else if(t_info.supertype == STRUCT){
            user_defined_type = true;
            contains_user_defined_type = true;
        } else if(t_info.supertype == TUPLE){
            user_defined_type = true;
            contains_user_defined_type = true;
        } else {
            print("Error - array type not supported in struct (yet!)");
            ret.valid = false; return ret;
        }
        it = result.end;
        if(it->type != NAME){
            print("Error in declaration of struct", obj.name, ", field", it->value, "invalid");
        }
        ret.result.elements.push_back(struct_element{type, it->value, user_defined_type});
        it++;
        if(it->type != EOL){
            print("Error while parsing struct", obj.name, ", expected newline or }");
        }
        it++;
    }
    
    ret.result.fully_defined_type = !contains_user_defined_type;
    ret.result.flag_for_algo = false;
    return ret;
}
// recursive helper function to construct declaration order of structs/tuples
// base case: if struct is fully defined, and 
// if struct not reached, check each element of the 
bool recurse_helper(const Type& struct_type){
    print("Parsing", struct_type);
    StructInfo& data = struct_table.at(struct_type);
    // check if struct has already been visited
    if(data.flag_for_algo == true){
        if(data.fully_defined_type){
            return true;
        } else {
            print("Error, cyclic definition found in struct", struct_type);
            return false;
        }
    }
    data.flag_for_algo = true;  // indicate struct has been visited
    // recursion base case - a struct composed of base types only
    if(data.fully_defined_type){
        decl_order.push_back(struct_type);
        return true;
    }
    // recursive step - continue for structs that are fields
    for(struct_element& e: data.elements){
        Entity supertype = type_table.get_info(e.type).supertype;
        if(supertype != PRIMITIVE_TYPE){
            bool valid = recurse_helper(e.type);
            if(!valid){
                print("Error, cyclic definition in struct", struct_type);
                return false;
            }
        }
    }
    decl_order.push_back(struct_type);
    data.fully_defined_type = true;
    return true;
}
// step 0 - get list of struct names & starting point, add all to entity table and type table 
// (done before this function)
// step 1 - for each struct, parse, and mark as concrete or not concrete, add to hashmap. 
// step 2 - iterate through structs in hashmap, and construct the declaration order, using the
// concrete flags
// step 3 - update the metadata in the type table with sizes, etc.

// any unfamiliar types, flag as unfamiliar
// vector<Type> decl_order - declaration order of structs
// vector<Entity_start> list - list of structs in initial program
// struct_table - hashmap of type -> StructInfo - list of structs in program
bool parseStructs(const vector<Entity_start>& list){

    // step 1 - for each struct, parse, add to struct table
    for(const Entity_start& entity : list){
        Option<StructInfo> opt = parseStruct(entity);
        if(opt.valid == false){
            print("Error parsing struct", entity.name);
            return false;
        }
        struct_table.insert({entity.name, opt.result});
    }
    
    // step 2 - iterate through structs, construct declaration order
    // this requires a recursive helper function
    decl_order.reserve(struct_table.size());
    // decl_order.reserve(list.size());
    for(const pair<Type, StructInfo>& struct_pair : struct_table){
        // helper determines if type is concrete - if a type is not concrete, return false
        // while doing so, construct the declaration order
        bool valid = recurse_helper(struct_pair.first);
        if(!valid){
            print("Error, cycle detected in struct definitions");
            return false;
        }
    }

    // step 3 - update metadata
    for(Type name : decl_order){
        TypeInfo& data = type_table.get_info(name);
        StructInfo& info = struct_table.at(name);
        data.copyable = true;
        for(struct_element& x : info.elements){
            data.copyable = data.copyable && type_table.get_info(x.type).copyable;
        }
        data.destructor = false;
        data.sized = true;
        for(struct_element& x : info.elements){
            data.size += type_table.get_info(x.type).size;
        }
        // fill out type fields
        for(struct_element& x : info.elements){
            if(data.supertype != TUPLE)
                data.fields.push_back(field{x.field, x.type}); 
        }
    }
    return true;
}

void printStructs(){
    for(Type i : decl_order){
        if(struct_table.count(i) != 1){
            print("Error - struct", i, "not in type table");
            continue;
        }
        StructInfo& info = struct_table.at(i);
        print("struct", i, "{");
        if(type_table.contains(i)){
            size_t size = type_table.get_info(i).size;
            print("Size:", size);
        } else {
            print("Error: not in type table");
        }
        for(const struct_element& e : info.elements){
            print(e.type.to_string(), e.field);
        }
        print("}");
    }
}