#ifndef WORLD_HPP
#define WORLD_HPP

#include <vector>
#include <string>
#include "Position.hpp"

/* ============ World Class ============

This class manages the entire simulation.

- runs turns (makeTurn())
- draws the world (drawWorld())
- stores all organisms
- stores Human input
- handles collisions indirectly
- stores messages/events

======================================== */

class Organism;
class Human;

class World {
private:
    int width;
    int height;
    int turn;
    std::vector<Organism*> organisms;
    std::vector<std::string> messages;

    int humanDx;
    int humanDy;
    bool specialAbilityRequested;

public:
    World(int width, int height);
    ~World();

    void drawWorld() const;
    void makeTurn();
    void sortOrganisms();
    void removeDeadOrganisms();

    void addOrganism(Organism* organism);
    Organism* getOrganismAt(int x, int y) const;
    Human* getHuman() const;

    bool isInside(int x, int y) const;
    bool isCellFree(int x, int y) const;
    bool findFreeNeighbor(int x, int y, Position& result) const;

    void addMessage(const std::string& message);
    void clearMessages();

    int getWidth() const;
    int getHeight() const;
    int getTurn() const;

    void setHumanMove(int dx, int dy);
    void getHumanMove(int& dx, int& dy) const;

    void requestSpecialAbility();
    bool consumeSpecialAbilityRequest();

    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
};

#endif
