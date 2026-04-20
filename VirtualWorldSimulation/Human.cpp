#include "Human.hpp"
#include "World.hpp"

// ============================================= Human Class =============================================

Human::Human(int x, int y, World* world)
    : Animal(5, 4, x, y, world), abilityTurnsLeft(0), cooldownTurnsLeft(0) {}

void Human::action() {
    tryActivateAbility();

    int dx, dy;
    world->getHumanMove(dx, dy);

    int step = isAbilityActive() ? 2 : 1;
    int newX = getX() + dx * step;
    int newY = getY() + dy * step;

    if (!world->isInside(newX, newY)) {
        newX = getX() + dx;
        newY = getY() + dy;
    }

    if (world->isInside(newX, newY)) {
        Organism* occupant = world->getOrganismAt(newX, newY);
        if (occupant == nullptr) {
            setPosition(newX, newY);
        } else {
            occupant->collision(this);
        }
    }

    if (abilityTurnsLeft > 0) {
        abilityTurnsLeft--;
        if (abilityTurnsLeft == 0) {
            cooldownTurnsLeft = 5;
            world->addMessage("Human special ability ended");
        }
    } else if (cooldownTurnsLeft > 0) {
        cooldownTurnsLeft--;
    }
}

void Human::collision(Organism* attacker) {
    Animal::collision(attacker);
}

char Human::draw() const {
    return 'H';
}

std::string Human::getName() const {
    return "Human";
}

bool Human::isAbilityActive() const {
    return abilityTurnsLeft > 0;
}

int Human::getAbilityTurnsLeft() const {
    return abilityTurnsLeft;
}

int Human::getCooldownTurnsLeft() const {
    return cooldownTurnsLeft;
}

void Human::setAbilityTurnsLeft(int turns) {
    abilityTurnsLeft = turns;
}

void Human::setCooldownTurnsLeft(int turns) {
    cooldownTurnsLeft = turns;
}

Organism* Human::createChild(int x, int y) {
    (void)x;
    (void)y;
    return nullptr;
}

void Human::tryActivateAbility() {
    if (world->consumeSpecialAbilityRequest()) {
        if (abilityTurnsLeft == 0 && cooldownTurnsLeft == 0) {
            abilityTurnsLeft = 5;
            world->addMessage("Human special ability activated for 5 turns");
        } else {
            world->addMessage("Human special ability unavailable");
        }
    }
}
