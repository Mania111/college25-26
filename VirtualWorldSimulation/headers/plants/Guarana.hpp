#ifndef GUARANA_HPP
#define GUARANA_HPP

#include "Plant.hpp"

class Guarana : public Plant {
public:
    Guarana(int x, int y, World* world);

    virtual void collision(Organism* attacker) override;
    virtual char draw() const override;
    virtual std::string getName() const override;

protected:
    virtual Organism* createChild(int x, int y) override;
};

#endif
