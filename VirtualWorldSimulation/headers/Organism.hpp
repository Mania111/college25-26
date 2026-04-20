#ifndef ORGANISM_HPP
#define ORGANISM_HPP

#include <string>
#include "Position.hpp"

class World;

class Organism {
protected:
    int strength;
    int initiative;
    int age;
    bool alive;
    Position pos;
    World* world;

public:
    Organism(int strength, int initiative, int x, int y, World* world);
    virtual ~Organism();

    virtual void action() = 0;
    virtual void collision(Organism* attacker) = 0;
    virtual char draw() const = 0;
    virtual std::string getName() const = 0;

    int getStrength() const;
    void setStrength(int strength);

    int getInitiative() const;
    int getAge() const;
    void increaseAge();

    int getX() const;
    int getY() const;
    void setPosition(int x, int y);

    bool isAlive() const;
    void kill();

    bool isSameSpecies(const Organism* other) const;
};

#endif
