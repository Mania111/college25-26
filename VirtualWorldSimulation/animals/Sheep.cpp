#include "animals/Sheep.hpp"
#include "World.hpp"

Sheep::Sheep(int x, int y, World* world)
    : Animal(4, 4, x, y, world) {}

char Sheep::draw() const {
    return 'S';
}

std::string Sheep::getName() const {
    return "Sheep";
}

Organism* Sheep::createChild(int x, int y) {
    return new Sheep(x, y, world);
}
