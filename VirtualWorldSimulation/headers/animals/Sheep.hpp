#ifndef SHEEP_HPP
#define SHEEP_HPP

#include "Animal.hpp"

class Sheep : public Animal {
public:
    Sheep(int x, int y, World* world);

    virtual char draw() const override;
    virtual std::string getName() const override;

protected:
    virtual Organism* createChild(int x, int y) override;
};

#endif
