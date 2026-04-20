#include "plants/Guarana.hpp"
#include "World.hpp"

Guarana::Guarana(int x, int y, World* world)
    : Plant(0, x, y, world) {}

void Guarana::collision(Organism* attacker) {
    attacker->setStrength(attacker->getStrength() + 3);
    world->addMessage(attacker->getName() + " ate Guarana and gained +3 strength");
    this->kill();
    attacker->setPosition(getX(), getY());
}

char Guarana::draw() const {
    return 'u';
}

std::string Guarana::getName() const {
    return "Guarana";
}

Organism* Guarana::createChild(int x, int y) {
    return new Guarana(x, y, world);
}
