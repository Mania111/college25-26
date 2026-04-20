#include "animals/Fox.hpp"
#include "World.hpp"
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <random>

Fox::Fox(int x, int y, World* world)
    : Animal(3, 7, x, y, world) {}

void Fox::action() {
    std::vector<Position> moves = {
        {getX(), getY() - 1},
        {getX(), getY() + 1},
        {getX() - 1, getY()},
        {getX() + 1, getY()}
    };

    static std::mt19937 rng(std::random_device{}());
    std::shuffle(moves.begin(), moves.end(), rng);

    for (const Position& p : moves) {
        if (!world->isInside(p.x, p.y)) continue;

        Organism* occupant = world->getOrganismAt(p.x, p.y);
        if (occupant == nullptr) {
            setPosition(p.x, p.y);
            return;
        }
        if (occupant->getStrength() <= getStrength()) {
            occupant->collision(this);
            return;
        }
    }

    world->addMessage("Fox stayed in place to avoid stronger organism");
}

char Fox::draw() const {
    return 'F';
}

std::string Fox::getName() const {
    return "Fox";
}

Organism* Fox::createChild(int x, int y) {
    return new Fox(x, y, world);
}
