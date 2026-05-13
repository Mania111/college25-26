#pragma once

/* =================================================== Treap.hpp ======================================================

    Treap declaration, node structure

    Node: id, price, priority, sum of subtree, size of subtree, left or right
    functions: getSize, update, split, merge

    All operators
    DD: split -> insert -> merge
    DELETE: split -> osolate -> delete -> merge
    RANGE SUM: -> split -> isolate -> read sum -> merge back

======================================================================================================================*/

class Treap {
private:
    // ========================================== Node struct
    struct Node {
        int id;
        int price;
        int priority;

        long long sum;
        int size;

        Node* left;
        Node* right;

        Node(int id_, int price_, int priority_);
    }; // ===================================================

    Node* root;
    // pointer to the root of the entire tree
    // every operation (add, delete, sum) starts from the root

    int randomPriority();

    int getSize(Node* node) const; // number of nodes in subtree
    long long getSum(Node* node) const; // sum of prices in subtree

    void update(Node* node);
    // recalculates after any changes in structure

    void split(Node* tree, int key, Node*& left, Node*& right); // returns results via reference parameters
    // splits tree into two
    // LEFT - all nodes with id <= key
    // RIGHT - all nodes with id > key
    Node* merge(Node* left, Node* right);
    // combines two trees into one
    // pointer because it tells you where the merge happens

    Node* find(Node* node, int id) const;
    // standard BST search
    // if id < node->id - GO LEFT
    // if id > node->id - GO RIGHT
    void updatePrice(Node* node, int id, int newPrice);
    // finds node with given id and updates the node to newprice

    void deleteTree(Node* node); // delete entire subtree (to avoid memory issues)

    // ===================================================
public:
    Treap();
    ~Treap();

    void add(int id, int price);
    void remove(int id);
    long long sumRange(int idMin, int idMax);
    int count() const;
};
