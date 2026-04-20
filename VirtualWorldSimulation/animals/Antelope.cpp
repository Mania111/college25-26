#include "animals/Antelope.hpp"
#include "World.hpp"
#include <cstdlib>

Antelope::Antelope(int x, int y, World* world)
    : Animal(4, 4, x, y, world) {}

void Antelope::action() {
    int dx[4] = {0, 0, -2, 2};
    int dy[4] = {-2, 2, 0, 0};

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

void Antelope::collision(Organism* attacker) {
    if (!isSameSpecies(attacker) && (std::rand() % 100) < 50) {
        Position freePos;
        if (world->findFreeNeighbor(getX(), getY(), freePos)) {
            setPosition(freePos.x, freePos.y);
            world->addMessage("Antelope escaped from fight");
            return;
        }
    }

    Animal::collision(attacker);
}

char Antelope::draw() const {
    return 'A';
}

std::string Antelope::getName() const {
    return "Antelope";
}

Organism* Antelope::createChild(int x, int y) {
    return new Antelope(x, y, world);
}
