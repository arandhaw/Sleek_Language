// Online C++ compiler to run C++ program online
#include <iostream>
#include "utils.cpp"
using namespace std;

#include <iostream>
#include <csignal>
#include <cstdlib>

void signalHandler(int signal) {
    if (signal == SIGSEGV) {
        std::cerr << "Error: Segmentation fault (SIGSEGV) occurred!" << std::endl;
        exit(1);  // Exit the program with a non-zero status
    }
}

int main() {
    // Register the signal handler
    signal(SIGSEGV, signalHandler);

    // Code that causes a segmentation fault (e.g., dereferencing a null pointer)
    int* ptr = nullptr;
    std::cout << *ptr << std::endl;  // This will cause a segmentation fault

    return 0;
}
