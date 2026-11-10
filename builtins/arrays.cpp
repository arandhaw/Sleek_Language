#pragma once
#include "basics.cpp"

struct $t_$int_$int$ {
  $int e0;
  $int e1;
};

struct $t_$int_$int_$int$ {
  $int e0;
  $int e1;
  $int e2;
};

template<typename T, size_t N>
class $array1D {
    public:
    T data[N];
    const static size_t size = N;
    const static size_t shape = N;
    
    size_t $size(){ return size; }

    T get(size_t x){
        if(x >= size)
            throw std::runtime_error("Read out of bounds of array");
        return data[x];
    }

    void set(size_t x, T v){
        if(x >= size)
            throw std::runtime_error("Write out of bounds of array");
        data[x] = v;
    }  
};

template<typename T, size_t N1, size_t N2>
class $array2D {
    public:
    T data[N1*N2];
    const static size_t size = N1*N2;
    // const static $t_$int_$int${N1, N2};
    

    size_t $size(){ return size; }

    T get(size_t x, size_t y){
        if(x >= N1 || y >= N2){
            cout << "oh no" << endl;
            throw std::runtime_error("Read out of bounds of array");
        }
        return data[x*N2 + y];
    }

    void set(size_t x, size_t y, T v){
        if(x >= N1 || y >= N2)
            throw std::runtime_error("Write out of bounds of array");
        data[x*N1 + y] = v;
    }  
};

template<typename T, size_t N1, size_t N2, size_t N3>
class $array3D {
    private:
        T data[N1*N2*N3];
        const static size_t size = N1*N2*N3;
        // const static $t_$int_$int_$int${N1, N2, N3};
    public:

    size_t $size(){ return size; }

    T get(size_t x, size_t y, size_t z){
        if(x >= N1 || y >= N2 || z >= N3)
            throw std::runtime_error("Read out of bounds of array");
        return data[x*N2*N3 + y*N2 + z];
    }

    void set(size_t x, size_t y, size_t z, T v){
        if(x >= N1 || y >= N2 || z >= N3)
            throw std::runtime_error("Write out of bounds of array");
        data[x*N2*N3 + y*N2 + z] = v;
    }  
};