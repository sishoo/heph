#pragma once

#include <vector>



template <typename T> 
class Mat4 {
public:
    T data[16];

    Mat4() {
        data = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    }

    static Mat4 transposed(Mat4 * const mat);
};

template <typename T> 
class MatN {
    T *data;
    
};



int main()
{

    return 0;
}