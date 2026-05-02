#include "treap.hpp"
#include <cstdlib>

/* =================================================== treap.cpp ======================================================

    Implements Treap from treap.hpp declaration

    - treap represents a dynamic list of tasks
    - it allows insertion by position using split and merge
    - it allows deletion by task id by first finding its position

    functions:
    1. split(tree, k, left, right)
    - separates tree into left(first k elements) and right(remaining elements)
    2. merge(left, right) - joins Treaps, preserves list order
    3. insertAt(position, task) - splits tree at position, then inserts new node between split parts
    4. removeById(id) - removes node using two splits
    5. copyToArray() - treap -> convert -> normal array of tasks

======================================================================================================================*/

Treap::Treap() {
    root = nullptr;
}

Treap::~Treap() {
    deleteTree(root);
}

int Treap::getSize(TreapNode* node) {
    if (node == nullptr) return 0;
    return node->size;
}

void Treap::update(TreapNode* node) {
    if (node == nullptr) return;
    node->size = 1 + getSize(node->left) + getSize(node->right);
}

TreapNode* Treap::createNode(Task task) {
    TreapNode* node = new TreapNode;

    node->task = task;
    node->priority = rand();
    node->size = 1;
    node->left = nullptr;
    node->right = nullptr;

    return node;
}

void Treap::split(TreapNode* node, int leftSize, TreapNode*& left, TreapNode*& right) {
    if (node == nullptr) {
        left = nullptr;
        right = nullptr;
        return;
    }

    int currentLeftSize = getSize(node->left);

    if (leftSize <= currentLeftSize) {
        split(node->left, leftSize, left, node->left);
        right = node;
        update(right);
    } else {
        split(node->right, leftSize - currentLeftSize - 1, node->right, right);
        left = node;
        update(left);
    }
}

TreapNode* Treap::merge(TreapNode* left, TreapNode* right) {
    if (left == nullptr) return right;
    if (right == nullptr) return left;

    if (left->priority > right->priority) {
        left->right = merge(left->right, right);
        update(left);
        return left;
    } else {
        right->left = merge(left, right->left);
        update(right);
        return right;
    }
}

int Treap::size() {
    return getSize(root);
}

void Treap::insertAt(int position, Task task) {
    if (position < 0) position = 0;
    if (position > size()) position = size();

    TreapNode* newNode = createNode(task);

    TreapNode* left = nullptr;
    TreapNode* right = nullptr;

    split(root, position, left, right);
    root = merge(merge(left, newNode), right);
}

int Treap::findPositionById(TreapNode* node, int id, int add) {
    if (node == nullptr) return -1;

    int leftResult = findPositionById(node->left, id, add);
    if (leftResult != -1) return leftResult;

    int currentPosition = add + getSize(node->left);

    if (node->task.id == id) {
        return currentPosition;
    }

    return findPositionById(node->right, id, currentPosition + 1);
}

void Treap::removeById(int id) {
    int position = findPositionById(root, id, 0);

    if (position == -1) {
        return;
    }

    TreapNode* left = nullptr;
    TreapNode* middle = nullptr;
    TreapNode* right = nullptr;

    split(root, position, left, middle);
    split(middle, 1, middle, right);

    deleteTree(middle);

    root = merge(left, right);
}

void Treap::copyToArray(TreapNode* node, Task* arr, int& index) {
    if (node == nullptr) return;

    copyToArray(node->left, arr, index);

    arr[index] = node->task;
    index++;

    copyToArray(node->right, arr, index);
}

void Treap::copyToArray(Task* arr) {
    int index = 0;
    copyToArray(root, arr, index);
}

void Treap::deleteTree(TreapNode* node) {
    if (node == nullptr) return;

    deleteTree(node->left);
    deleteTree(node->right);

    delete node;
}
