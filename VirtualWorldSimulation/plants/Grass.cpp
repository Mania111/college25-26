#include "plants/Grass.hpp"
#include "World.hpp"

Grass::Grass(int x, int y, World* world)
    : Plant(0, x, y, world) {}

char Grass::draw() const {
    return 'g';
}

std::string Grass::getName() const {
    return "Grass";
}

Organism* Grass::createChild(int x, int y) {
    return new Grass(x, y, world);
}
