#include "plants/SowThistle.hpp"
#include "World.hpp"

SowThistle::SowThistle(int x, int y, World* world)
    : Plant(0, x, y, world) {}

void SowThistle::action() {
    for (int i = 0; i < 3; i++) {
        Plant::action();
    }
}

char SowThistle::draw() const {
    return 's';
}

std::string SowThistle::getName() const {
    return "SowThistle";
}

Organism* SowThistle::createChild(int x, int y) {
    return new SowThistle(x, y, world);
}
