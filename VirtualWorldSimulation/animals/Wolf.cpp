#include "animals/Wolf.hpp"
#include "World.hpp"

Wolf::Wolf(int x, int y, World* world)
    : Animal(9, 5, x, y, world) {}

char Wolf::draw() const {
    return 'W';
}

std::string Wolf::getName() const {
    return "Wolf";
}

Organism* Wolf::createChild(int x, int y) {
    return new Wolf(x, y, world);
}
