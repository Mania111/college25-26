#ifndef ANTELOPE_HPP
#define ANTELOPE_HPP

#include "Animal.hpp"

class Antelope : public Animal {
public:
    Antelope(int x, int y, World* world);

    virtual void action() override;
    virtual void collision(Organism* attacker) override;
    virtual char draw() const override;
    virtual std::string getName() const override;

protected:
    virtual Organism* createChild(int x, int y) override;
};

#endif
