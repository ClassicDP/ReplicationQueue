//
// Created by dmitry on 25.07.2021.
//

#ifndef UNTITLED_DYNAMICARRAY_H
#define UNTITLED_DYNAMICARRAY_H


#include <cstdint>

template<class T>
class DynamicArray {
public:
    uint32_t size{};
    T *data;

    DynamicArray(uint32_t size);
    DynamicArray(char const *msg);

    T &operator[](uint32_t i) {
        return data[i];
    };

    ~DynamicArray();

};

template<class T>
DynamicArray<T>::DynamicArray(uint32_t size) {
    this->size = size;
    data = new T[size];
}

template<class T>
DynamicArray<T>::DynamicArray(char const *msg) {
    this->size = 0;
    while (*(msg + this->size++) != 0) {}
    data = new T[size];
    auto i = 0;
    while (*(msg + i++) != 0) *(data + i) = *(msg + i);
}

template<class T>
DynamicArray<T>::~DynamicArray() {
    delete[] data;
}


#endif //UNTITLED_DYNAMICARRAY_H
