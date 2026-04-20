#include "Animal.hpp"
#include "World.hpp"
#include <cstdlib>

// ============================================= Animal Class =============================================

Animal::Animal(int strength, int initiative, int x, int y, World* world)
    : Organism(strength, initiative, x, y, world) {}

Animal::~Animal() {}

void Animal::action() {
    int dx[4] = {0, 0, -1, 1};
    int dy[4] = {-1, 1, 0, 0};

    int dir = std::rand() % 4;
    int newX = getX() + dx[dir];
    int newY = getY() + dy[dir];

    if (!world->isInside(newX, newY)) {
        return;
    }

    Organism* occupant = world->getOrganismAt(newX, newY);
    if (occupant == nullptr) {
        setPosition(newX, newY);
    } else {
        occupant->collision(this);
    }
}

void Animal::collision(Organism* attacker) {
    if (isSameSpecies(attacker)) {
        tryBreedWith(attacker);
        return;
    }

    if (attacker->getStrength() >= this->getStrength()) {
        world->addMessage(attacker->getName() + " killed " + this->getName() +
                          " at (" + std::to_string(getX()) + "," + std::to_string(getY()) + ")");
        this->kill();
        attacker->setPosition(getX(), getY());
    } else {
        world->addMessage(this->getName() + " killed " + attacker->getName() +
                          " at (" + std::to_string(getX()) + "," + std::to_string(getY()) + ")");
        attacker->kill();
    }
}

void Animal::tryBreedWith(Organism* partner) {
    Position freePos;
    if (world->findFreeNeighbor(getX(), getY(), freePos)) {
        Organism* child = createChild(freePos.x, freePos.y);
        world->addOrganism(child);
        world->addMessage(getName() + " bred a new " + getName() +
                          " at (" + std::to_string(freePos.x) + "," + std::to_string(freePos.y) + ")");
    }
}
