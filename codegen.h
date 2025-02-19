#include "parser.cpp"
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include "utils.cpp"


class Codegen {
    private:
        string output_file;
        std::ofstream file_stream;
        bool file_open;
        int indent = 0;
        string br = "\n";
    public:

    void genProgram(const Program& p);

    Codegen(){}
    // prepare file stream
    // return false if can't be opened
    bool init(string output_file){
        this->output_file = output_file;
        file_stream.open(output_file);
        if (file_stream.is_open()) {
            file_open = true;
            return true;
        } else {
            file_open = false;
            return false;
        }
    }
    
    ~Codegen(){
        if(file_open){
            file_stream.close();
        }
    }

    // print a newline
    void line(){
        file_stream << br;
    }

    // add a tab - the indent is increased
    void tab(){
        indent++;
        if(indent){
            br = br + "  ";
        }
    }
    // delete a tab - the indent is decreased
    void dtab(){
        indent--;
        if(indent < 0){
            cout << "Error - negative indent" << endl;
            indent = 0;
            return;
        } else {
            br.pop_back();
            br.pop_back();
        }
    }

    template<typename T, typename...A>
    void out(const T& first, const A&...args) {
        file_stream << first;
        ((file_stream << " ", file_stream << args), ...);
        file_stream << br;
    }

    //print each arguement (except first two)
    //@sep is printed between each arguement
    //@end is printed at the end
    template<typename T, typename...A>
    void custom(const char* sep, const char* end, const T& first, const A&...args) {
        file_stream << first;
        ((file_stream << sep, file_stream << args), ...);
        file_stream << end;
    }

    // print each arguement
    // no spaces/newline added in
    template<typename...A>
    void raw(const A&...args) {
        ((file_stream << args), ...);
    }
};

// int main(){
//     using Codegen;
//     Codegen cg("output.c");
//     cg.init()
//     cg.out("hello, world!", "My name is Armaan");
//     cg.out("This is file writer!");
//     cg.indent++;
//     cg.out("y = 0;")
//     cg.lb
//     br()
    
//     return 0;
// }