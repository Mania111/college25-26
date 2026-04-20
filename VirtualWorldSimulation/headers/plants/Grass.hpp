#ifndef GRASS_HPP
#define GRASS_HPP

#include "Plant.hpp"

class Grass : public Plant {
public:
    Grass(int x, int y, World* world);

    virtual char draw() const override;
    virtual std::string getName() const override;

protected:
    virtual Organism* createChild(int x, int y) override;
};

#endif
