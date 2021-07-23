
#include "StackList.h"

template<class T>
StackList<T>::StackList() {
    top = nullptr;
}

template<class T>
T StackList<T>::pop() {
    if (!top) return nullptr;
    auto item = top->item;
    auto toDelete = top;
    top = top->prev;
    delete toDelete;
    return item;
}

template<class T>
void StackList<T>::push(T item) {
    auto topPrev = top;
    top = new StackItem<T>;
    top->prev = topPrev;
    top->item = item;
}


template<class T>
StackList<T>::~StackList() {
    while (top) {
        auto toDelete = top;
        top = top->prev;
        delete toDelete;
    }
}
