#include "Organism.hpp"

// ============================================= Organism Class =============================================

// base class for all organisms

Organism::Organism(int strength, int initiative, int x, int y, World* world)
    : strength(strength), initiative(initiative), age(0), alive(true), pos{x, y}, world(world) {}

Organism::~Organism() {}

int Organism::getStrength() const {
    return strength;
}

void Organism::setStrength(int strength) {
    this->strength = strength;
}

int Organism::getInitiative() const {
    return initiative;
}

int Organism::getAge() const {
    return age;
}

void Organism::increaseAge() {
    age++;
}

int Organism::getX() const {
    return pos.x;
}

int Organism::getY() const {
    return pos.y;
}

void Organism::setPosition(int x, int y) {
    pos.x = x;
    pos.y = y;
}

bool Organism::isAlive() const {
    return alive;
}

void Organism::kill() {
    alive = false;
}

bool Organism::isSameSpecies(const Organism* other) const {
    return this->getName() == other->getName();
}
