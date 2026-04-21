
/* ============ VirtualWorldSimulator - World.cpp ============

This file is responsible for managing adding all organism files to the program code.
It manages the entire simulation.

- runs turns (makeTurn())
- draws the world (drawWorld())
- stores all organisms
- stores Human input
- handles collisions indirectly
- stores messages/events

============================================================== */

#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>

// --------------------------------------------------------- headers

#include "Human.hpp"
#include "World.hpp"
#include "Organism.hpp"
#include "Graphics.hpp"
#include "animals/Wolf.hpp"
#include "animals/Sheep.hpp"
#include "animals/Fox.hpp"
#include "animals/Turtle.hpp"
#include "animals/Antelope.hpp"
#include "plants/Grass.hpp"
#include "plants/SowThistle.hpp"
#include "plants/Guarana.hpp"
#include "plants/Belladonna.hpp"
#include "plants/Hogweed.hpp"

// ============================================= World Class =============================================

// --------------------------------------------------------- board & game

World::World(int width, int height)
    : width(width), height(height), turn(0), humanDx(0), humanDy(0), specialAbilityRequested(false) {}

int World::getWidth() const {
    return width;
}

int World::getHeight() const {
    return height;
}

int World::getTurn() const {
    return turn;
}

void World::makeTurn() {
    clearMessages();
    sortOrganisms();

    size_t currentSize = organisms.size();
    for (size_t i = 0; i < currentSize; i++) {
        if (organisms[i]->isAlive()) {
            organisms[i]->action();
        }
    }

    removeDeadOrganisms();

    for (Organism* org : organisms) {
        org->increaseAge();
    }

    turn++;
}

void World::drawWorld() const {
    std::cout << "Student: Marta Fryczke, Index: s208184\n";
    std::cout << "Turn: " << turn << "\n\n";

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Organism* org = getOrganismAt(x, y);
            if (org) {
                printColoredTile(org->draw());
            } else {
                printEmptyTile();
            }
        }
        std::cout << '\n';
    }

    std::cout << "\nEvents:\n";
    if (messages.empty()) {
        std::cout << "- none\n";
    } else {
        for (const std::string& msg : messages) {
            std::cout << "- " << msg << '\n';
        }
    }
}

// --------------------------------------------------------- Organisms

World::~World() {
    for (Organism* org : organisms) {
        delete org;
    }
}

void World::addOrganism(Organism* organism) {
    organisms.push_back(organism);
}

Organism* World::getOrganismAt(int x, int y) const {
    for (Organism* org : organisms) {
        if (org->isAlive() && org->getX() == x && org->getY() == y) {
            return org;
        }
    }
    return nullptr;
}

void World::sortOrganisms() {
    std::sort(organisms.begin(), organisms.end(),
        [](Organism* a, Organism* b) {
            if (a->getInitiative() != b->getInitiative()) {
                return a->getInitiative() > b->getInitiative();
            }
            return a->getAge() > b->getAge();
        }
    );
}

void World::removeDeadOrganisms() {
    auto it = organisms.begin();
    while (it != organisms.end()) {
        if (!(*it)->isAlive()) {
            delete *it;
            it = organisms.erase(it);
        } else {
            ++it;
        }
    }
}

// --------------------------------------------------------- Human

Human* World::getHuman() const {
    for (Organism* org : organisms) {
        if (org->isAlive() && org->getName() == "Human") {
            return static_cast<Human*>(org);
        }
    }
    return nullptr;
}

// --------------------------------------------------------- tile management

bool World::isInside(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

bool World::isCellFree(int x, int y) const {
    return getOrganismAt(x, y) == nullptr;
}

bool World::findFreeNeighbor(int x, int y, Position& result) const {
    std::vector<Position> neighbors = {
        {x, y - 1},
        {x, y + 1},
        {x - 1, y},
        {x + 1, y}
    };

    static std::mt19937 rng(std::random_device{}());
    std::shuffle(neighbors.begin(), neighbors.end(), rng);

    for (const Position& p : neighbors) {
        if (isInside(p.x, p.y) && isCellFree(p.x, p.y)) {
            result = p;
            return true;
        }
    }
    return false;
}

// --------------------------------------------------------- messages

void World::addMessage(const std::string& message) {
    messages.push_back(message);
}

void World::clearMessages() {
    messages.clear();
}

// --------------------------------------------------------- player input & special ability

void World::setHumanMove(int dx, int dy) {
    humanDx = dx;
    humanDy = dy;
}

void World::getHumanMove(int& dx, int& dy) const {
    dx = humanDx;
    dy = humanDy;
}

void World::requestSpecialAbility() {
    specialAbilityRequested = true;
}

bool World::consumeSpecialAbilityRequest() {
    bool requested = specialAbilityRequested;
    specialAbilityRequested = false;
    return requested;
}

// --------------------------------------------------------- saving + loading

bool World::saveToFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        return false;
    }

    out << width << ' ' << height << ' ' << turn << '\n';
    out << humanDx << ' ' << humanDy << '\n';

    Human* human = getHuman();
    if (human != nullptr) {
        out << human->getAbilityTurnsLeft() << ' ' << human->getCooldownTurnsLeft() << '\n';
    } else {
        out << 0 << ' ' << 0 << '\n';
    }

    int aliveCount = 0;
    for (Organism* org : organisms) {
        if (org->isAlive()) {
            aliveCount++;
        }
    }
    out << aliveCount << '\n';

    for (Organism* org : organisms) {
        if (!org->isAlive()) continue;

        out << org->getName() << ' '
            << org->getX() << ' '
            << org->getY() << ' '
            << org->getAge() << ' '
            << org->getStrength() << '\n';
    }

    return true;
}

bool World::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        return false;
    }

    for (Organism* org : organisms) {
        delete org;
    }
    organisms.clear();
    messages.clear();

    int loadedTurn;
    in >> width >> height >> loadedTurn;
    turn = loadedTurn;

    in >> humanDx >> humanDy;

    int humanAbilityTurns;
    int humanCooldownTurns;
    in >> humanAbilityTurns >> humanCooldownTurns;

    int count;
    in >> count;

    Human* loadedHuman = nullptr;

    for (int i = 0; i < count; i++) {
        std::string type;
        int x, y, age, strength;

        in >> type >> x >> y >> age >> strength;

        Organism* org = nullptr;

        if (type == "Human") org = new Human(x, y, this);
        else if (type == "Wolf") org = new Wolf(x, y, this);
        else if (type == "Sheep") org = new Sheep(x, y, this);
        else if (type == "Fox") org = new Fox(x, y, this);
        else if (type == "Turtle") org = new Turtle(x, y, this);
        else if (type == "Antelope") org = new Antelope(x, y, this);
        else if (type == "Grass") org = new Grass(x, y, this);
        else if (type == "SowThistle") org = new SowThistle(x, y, this);
        else if (type == "Guarana") org = new Guarana(x, y, this);
        else if (type == "Belladonna") org = new Belladonna(x, y, this);
        else if (type == "Hogweed") org = new Hogweed(x, y, this);

        if (org != nullptr) {
            org->setStrength(strength);

            for (int j = 0; j < age; j++) {
                org->increaseAge();
            }

            addOrganism(org);

            if (type == "Human") {
                loadedHuman = static_cast<Human*>(org);
            }
        }
    }

    if (loadedHuman != nullptr) {
        loadedHuman->setAbilityTurnsLeft(humanAbilityTurns);
        loadedHuman->setCooldownTurnsLeft(humanCooldownTurns);
    }

    addMessage("World loaded from file: " + filename);
    return true;
}
