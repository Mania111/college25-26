#ifndef SOWTHISTLE_HPP
#define SOWTHISTLE_HPP

#include "Plant.hpp"

class SowThistle : public Plant {
public:
    SowThistle(int x, int y, World* world);

    virtual void action() override;
    virtual char draw() const override;
    virtual std::string getName() const override;

protected:
    virtual Organism* createChild(int x, int y) override;
};

#endif
