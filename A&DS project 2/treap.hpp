#ifndef TREAP_HPP
#define TREAP_HPP

#include "task.hpp"

/* =================================================== treap.hpp ======================================================

    Declares Treap structure used to store the current ordered list of tasks
    Cant use ready STL containters - treap required

    1. insertAt(position, task) - inserts task at selected position in list
    2. removeById(id)
    3. size() - returns current number of tasks
    4. copyToArray(array) - copies current task list into a normal dynamic array

    This is an implicit Treap:
    - order is based on position in the list
    - each node stores subtree size
    - random priority keeps the tree balanced on average

======================================================================================================================*/

struct TreapNode {
    Task task;
    int priority;
    int size;

    TreapNode* left;
    TreapNode* right;
};

class Treap {
private:
    TreapNode* root;

    int getSize(TreapNode* node);
    void update(TreapNode* node);

    TreapNode* createNode(Task task);

    void split(TreapNode* node, int leftSize, TreapNode*& left, TreapNode*& right);
    TreapNode* merge(TreapNode* left, TreapNode* right);

    int findPositionById(TreapNode* node, int id, int add);

    void copyToArray(TreapNode* node, Task* arr, int& index);
    void deleteTree(TreapNode* node);

public:
    Treap();
    ~Treap();

    int size();

    void insertAt(int position, Task task);
    void removeById(int id);

    void copyToArray(Task* arr);
};

#endif
