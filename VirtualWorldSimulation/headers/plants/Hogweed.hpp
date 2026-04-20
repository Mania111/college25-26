#ifndef HOGWEED_HPP
#define HOGWEED_HPP

#include "Plant.hpp"

class Hogweed : public Plant {
public:
    Hogweed(int x, int y, World* world);

    virtual void action() override;
    virtual void collision(Organism* attacker) override;
    virtual char draw() const override;
    virtual std::string getName() const override;

protected:
    virtual Organism* createChild(int x, int y) override;
};

#endif
