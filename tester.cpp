#include "utils.cpp"
#include "builtins/arrays.cpp"

#include <iostream>
using namespace std;

// #include <csignal>
// #include <cstdlib>
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

template<typename T, size_t N>
std::array<T,N> add(std::array<T,N> x, std::array<T,N> y){
    std::array<T,N> ret;
    for(int i = 0; i < N; i++){
        ret[i] = x[i] + y[i];
    }
    return ret;
}

int main() {
    // Write C++ code here
    $array2D<int, 3, 2> myArr{1, 2, 3, 4, 5, 6};
    print(myArr.get(0, 0));
    print(myArr.get(1, 0));
    print(myArr.get(2, 0));
    print(myArr.get(2, 1));
    return 0;
}