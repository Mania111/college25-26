#ifndef WOLF_HPP
#define WOLF_HPP

#include "Animal.hpp"

class Wolf : public Animal {
public:
    Wolf(int x, int y, World* world);

    virtual char draw() const override;
    virtual std::string getName() const override;

protected:
    virtual Organism* createChild(int x, int y) override;
};

#endif
