#ifndef ANIMAL_HPP
#define ANIMAL_HPP

#include "Organism.hpp"

class Animal : public Organism {
public:
    Animal(int strength, int initiative, int x, int y, World* world);
    virtual ~Animal();

    virtual void action() override;
    virtual void collision(Organism* attacker) override;

protected:
    virtual Organism* createChild(int x, int y) = 0;
    void tryBreedWith(Organism* partner);
};

#endif
