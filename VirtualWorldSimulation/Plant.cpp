#include "Plant.hpp"
#include "World.hpp"
#include <cstdlib>

// ============================================= Plant Class =============================================

Plant::Plant(int strength, int x, int y, World* world)
    : Organism(strength, 0, x, y, world) {}

Plant::~Plant() {}

int Plant::getSpreadChance() const {
    return 20;
}

void Plant::action() {
    int roll = std::rand() % 100;
    if (roll >= getSpreadChance()) {
        return;
    }

    Position freePos;
    if (world->findFreeNeighbor(getX(), getY(), freePos)) {
        Organism* child = createChild(freePos.x, freePos.y);
        world->addOrganism(child);
        world->addMessage(getName() + " spread to (" + std::to_string(freePos.x) + "," + std::to_string(freePos.y) + ")");
    }
}

void Plant::collision(Organism* attacker) {
    world->addMessage(attacker->getName() + " ate " + getName() +
                      " at (" + std::to_string(getX()) + "," + std::to_string(getY()) + ")");
    this->kill();
    attacker->setPosition(getX(), getY());
}
