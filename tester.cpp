// Online C++ compiler to run C++ program online
#include <iostream>
#include "utils.cpp"
using namespace std;

#include <iostream>
#include <csignal>
#include <cstdlib>

// void signalHandler(int signal) {
//     std::cerr << "Error: Segmentation fault (SIGSEGV) occurred!" << endl;
//     exit(1);  // Exit the program with a non-zero status
// }
// signal(SIGSEGV, signalHandler);

// int recurse_infinite(size_t x){
//     size_t y = x + 1;
//     if(x == SIZE_MAX){
//         return y;
//     } else {
//         return recurse_infinite(x + 1);
//     }
// }

#include <array>

void printMode(const $string& x){
    if(x.tag == $string::Tag::heap_str){
        cout << "heap string" << endl;
    } else {
        cout << "const string" << endl;
    }
}

int main() {
    // Write C++ code here
    $string y("my name is francisco montoya. You killed my father. Prepare to die", 30);
    y.print();
    printMode(y);
    y.set(0, 'M');
    y.print();
    printMode(y);

    return 0;
}