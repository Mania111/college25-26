#ifndef FOX_HPP
#define FOX_HPP

#include "Animal.hpp"

class Fox : public Animal {
public:
    Fox(int x, int y, World* world);

    virtual void action() override;
    virtual char draw() const override;
    virtual std::string getName() const override;

protected:
    virtual Organism* createChild(int x, int y) override;
};

#endif
