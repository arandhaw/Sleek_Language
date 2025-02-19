#include "codegen.h"

// any C program is composed of functions, unions, structs, enums, global variables
// C++ adds classes, templates, namespaces/modules 
void Codegen::genProgram(const Program& program){
    out("#include \"builtins.cpp\"");
    line();
    tab();
    out("int main(){"); 
    out("printf(\"Hello world!\");");
    dtab();
    out("}");
    line();
}

