//
// Created by dmitry on 23.07.2021.
//

#ifndef UNTITLED_STACKLIST_H
#define UNTITLED_STACKLIST_H

template<class T>
struct StackItem {
    StackItem *prev;
    T item;
};

template<class T>
class StackList {
public:
    StackItem<T> *top;

    StackList();

    ~StackList();

    void push(T item);

    T pop();

};

#endif //UNTITLED_STACKLIST_H
