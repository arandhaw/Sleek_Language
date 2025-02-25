#include "codegen.h"

// any C program is composed of functions, unions, structs, enums, global variables
// C++ adds classes, templates, namespaces/modules 
void Codegen::genProgram(const Program& program){
    out("#include \"builtins.cpp\"");
    out();
    genMain(program.main);
    out();
}

void Codegen::genFunction(const Function& func){
    return;
}

void Codegen::genMain(const Function& main){
    out("int main(){");
    indent();
    tab(); out("printf(\"Hello world!\");");
    tab(); out("return 0;");
    dindent();
    tab(); out("}");
}

