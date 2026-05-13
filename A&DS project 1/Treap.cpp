#include "Treap.hpp"

#include <cstdlib>
#include <ctime>

/* =================================================== Treap.cpp ======================================================

    Treap and node structure definition

======================================================================================================================*/

Treap::Node::Node(int id_, int price_, int priority_)
    : id(id_),
      price(price_),
      priority(priority_),
      sum(price_),
      size(1), // every node is size 1 so that you can see size of subtrees
      left(nullptr), // empty pointers
      right(nullptr)
{
    // empty constructor body - all initialization already done above ^
}

Treap::Treap()
    : root(nullptr) // empty treat - the tree starts empty
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

Treap::~Treap()
{
    deleteTree(root); // destructor
}

int Treap::randomPriority()
{
    return std::rand();
    // random priority bro idk what to add here
}

int Treap::getSize(Node* node) const
{
    if (node == nullptr) {
        return 0;
    }

    return node->size;
}

long long Treap::getSum(Node* node) const
{
    if (node == nullptr) {
        return 0;
    }

    return node->sum;
}

void Treap::update(Node* node)
// updates size and sum based on the nodes children
{
    if (node == nullptr) {
        return;
    }

    node->size = 1 + getSize(node->left) + getSize(node->right);
    node->sum = node->price + getSum(node->left) + getSum(node->right);
}

void Treap::split(Node* tree, int key, Node*& left, Node*& right)
// Split definition
// LEFT - all nodes with id <= key
// RIGHT - all nodes with id > key
{
    if (tree == nullptr) {
        left = nullptr;
        right = nullptr;
        return;
    }

    if (tree->id <= key) {
        split(tree->right, key, tree->right, right);
        // if current node belongs to the left result
        // then its right subtree may still have nodes that should go to the right
        // so we split its right subtree
        left = tree;
        update(left);
    } else {
        split(tree->left, key, left, tree->left);
        // same as above
        right = tree;
        update(right);
    }
}

Treap::Node* Treap::merge(Node* left, Node* right)
// combines two trees into one
{
    if (left == nullptr) {
        return right;
    }

    if (right == nullptr) {
        return left;
    }

    if (left->priority > right->priority) {
        // if left has higher priority
        left->right = merge(left->right, right);
        update(left);
        return left;
    } else {
        // if right has higher priority
        right->left = merge(left, right->left);
        update(right);
        return right;
    }
    // returns Node* (NOT void) - because we are looking for the new root
}

Treap::Node* Treap::find(Node* node, int id) const
// searches by product id
// when finds it returns pointer, if not nullptr
// is used in add to check if product exists
{
    if (node == nullptr) {
        return nullptr;
    }

    if (id == node->id) {
        return node;
    }

    if (id < node->id) {
        return find(node->left, id);
    }

    return find(node->right, id);
}

void Treap::updatePrice(Node* node, int id, int newPrice)
// changes price of a product, updates sum and ancestors
{
    if (node == nullptr) {
        return;
    }

    if (id == node->id) {
        node->price = newPrice;
        update(node);
        return;
    }

    if (id < node->id) {
        updatePrice(node->left, id, newPrice);
    } else {
        updatePrice(node->right, id, newPrice);
    }

    update(node); // ancestors
}

// =================================================== command handling

void Treap::add(int id, int price)
{ // handles command "A id price"
    Node* existing = find(root, id);

    if (existing != nullptr) {
        // if product already exists - just change price
        updatePrice(root, id, price);
        return;
    }

    Node* newNode = new Node(id, price, randomPriority());
    // creates new node

    Node* left = nullptr;
    Node* right = nullptr;
    // these are temporary trees they store result of split below

    split(root, id, left, right);
    // left -> IDs < id
    // right -> IDs > id

    root = merge(merge(left, newNode), right);
    // puts new node between two parts from above
}

void Treap::remove(int id)
{
// handles command D id
// deletes node with this id
    Node* left = nullptr;
    Node* middle = nullptr;
    Node* right = nullptr;

    split(root, id - 1, left, middle);
    split(middle, id, middle, right);
    // left = IDs < id
    // middle = id
    // right = IDs > id

    deleteTree(middle); // delete id

    root = merge(left, right);
}

long long Treap::sumRange(int idMin, int idMax)
{
// handles command S id_min id_max
// isolates the required range
    Node* left = nullptr;
    Node* middle = nullptr;
    Node* right = nullptr;

    split(root, idMin - 1, left, middle);
    // left = IDs < idMind
    // right = IDs >= idMin
    split(middle, idMax, middle, right);
    // middle = idMin <= IDs <= idMax (carried over)
    // right = IDs > idMax

    long long result = getSum(middle);
    // every node stores subtree sum

    root = merge(left, merge(middle, right));
    // restores original tree

    return result;
}

int Treap::count() const
{
// handles command C
// returns number of products
    return getSize(root);
}

void Treap::deleteTree(Node* node)
{
// recursively deletes a tree
// 1. delete left subtree
// 2. delete right subtree
// and THEN 3. delete current node
// to keep access
    if (node == nullptr) {
        return;
    }

    deleteTree(node->left);
    deleteTree(node->right);

    delete node;
}
