// Online C++ compiler to run C++ program online
#include <iostream>
#include "utils.cpp"
using namespace std;

struct MyInt {
    int value;
    MyInt(int value) : value(value) {
        cout << "created " << value << endl;
    }
    MyInt(MyInt const& copy){
        value = copy.value;
        cout << "copy created " << value << endl;
    }

    MyInt(){
        value = 0;
    }
    ~MyInt() {
        cout << "destroyed " << value << endl;
    }
};

std::ostream& operator<< (std::ostream& out, const MyInt& v) {
    cout << v.value;
    return out;
}

int main() {
    // Write C++ code here
    print("hello, world");
    // Result<None, int> test{None{}};
    // Result<MyInt, double> A{MyInt(49)};
    // Result<string, int> D{""};
    // Result<MyInt, double> B = {3.14};
    // Result<MyInt, double> C = A;
    // cout << A.valid() << endl;
    // cout << B.valid() << endl;
    // cout << A.data() << endl;
    // cout << B.error() << endl;
    // cout << B.error() << endl;
    // cout << B.data() << endl;
    // cout << C.data() << endl;
    // print("Hello there");
    
    return 0;
}