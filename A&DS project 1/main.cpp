#include <iostream>
#include <cstdlib>

#include "treap.hpp"

using namespace std;

/* =================================================== main.cpp ======================================================

    Main file of the project
    - takes command input and returns the results
    - header code implementation

    BANNED:
    std::map
    std::set
    std::vector
    std::string

======================================================================================================================*/

int main()
{
    Treap treap;

    char command;

    while (std::cin >> command) {
        if (command == 'A') {
            int id;
            int price;

            std::cin >> id >> price;
            treap.add(id, price);
        }
        else if (command == 'D') {
            int id;

            std::cin >> id;
            treap.remove(id);
        }
        else if (command == 'S') {
            int idMin;
            int idMax;

            std::cin >> idMin >> idMax;

            long long result = treap.sumRange(idMin, idMax);
            std::cout << result << '\n';
        }
        else if (command == 'C') {
            std::cout << treap.count() << '\n';
        }
    }

    return 0;
}