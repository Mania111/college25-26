#include "plants/Hogweed.hpp"
#include "World.hpp"
#include "Organism.hpp"

Hogweed::Hogweed(int x, int y, World* world)
    : Plant(10, x, y, world) {}

void Hogweed::action() {
    int dx[4] = {0, 0, -1, 1};
    int dy[4] = {-1, 1, 0, 0};

    for (int i = 0; i < 4; i++) {
        int nx = getX() + dx[i];
        int ny = getY() + dy[i];

        if (!world->isInside(nx, ny)) continue;

        Organism* victim = world->getOrganismAt(nx, ny);
        if (victim && victim->getName() != "Hogweed") {
            victim->kill();
            world->addMessage("Hogweed killed " + victim->getName() +
                              " near (" + std::to_string(getX()) + "," + std::to_string(getY()) + ")");
        }
    }

    Plant::action();
}

void Hogweed::collision(Organism* attacker) {
    if (attacker->getName() == "CyberSheep") {
        world->addMessage("CyberSheep ate Hogweed safely");
        this->kill();
        attacker->setPosition(getX(), getY());
    } else {
        world->addMessage(attacker->getName() + " ate Hogweed and died");
        attacker->kill();
        this->kill();
    }
}

char Hogweed::draw() const {
    return 'h';
}

std::string Hogweed::getName() const {
    return "Hogweed";
}

Organism* Hogweed::createChild(int x, int y) {
    return new Hogweed(x, y, world);
}
