#include "plants/Belladonna.hpp"
#include "World.hpp"

Belladonna::Belladonna(int x, int y, World* world)
    : Plant(99, x, y, world) {}

void Belladonna::collision(Organism* attacker) {
    world->addMessage(attacker->getName() + " ate Belladonna and died");
    attacker->kill();
    this->kill();
}

char Belladonna::draw() const {
    return 'b';
}

std::string Belladonna::getName() const {
    return "Belladonna";
}

Organism* Belladonna::createChild(int x, int y) {
    return new Belladonna(x, y, world);
}
