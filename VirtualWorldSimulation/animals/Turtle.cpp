#include "animals/Turtle.hpp"
#include "World.hpp"
#include <cstdlib>

Turtle::Turtle(int x, int y, World* world)
    : Animal(2, 1, x, y, world) {}

void Turtle::action() {
    int roll = std::rand() % 100;
    if (roll < 75) {
        world->addMessage("Turtle stayed in place");
        return;
    }

    Animal::action();
}

void Turtle::collision(Organism* attacker) {
    if (attacker->getStrength() < 5) {
        world->addMessage("Turtle reflected attack from " + attacker->getName());
        return;
    }

    Animal::collision(attacker);
}

char Turtle::draw() const {
    return 'T';
}

std::string Turtle::getName() const {
    return "Turtle";
}

Organism* Turtle::createChild(int x, int y) {
    return new Turtle(x, y, world);
}
