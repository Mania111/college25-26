#ifndef PLANT_HPP
#define PLANT_HPP

#include "Organism.hpp"

// ============================================= Plant Class =============================================

// specialization class for all plants
// spreading, fixed initiative

// =======================================================================================================

class Plant : public Organism {
public:
    Plant(int strength, int x, int y, World* world);
    virtual ~Plant();

    virtual void action() override;
    virtual void collision(Organism* attacker) override;

protected:
    virtual int getSpreadChance() const;
    virtual Organism* createChild(int x, int y) = 0;
};

#endif
