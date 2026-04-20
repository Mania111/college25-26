#ifndef HUMAN_HPP
#define HUMAN_HPP

#include "Animal.hpp"

class Human : public Animal {
private:
    int abilityTurnsLeft;
    int cooldownTurnsLeft;

public:
    Human(int x, int y, World* world);

    virtual void action() override;
    virtual void collision(Organism* attacker) override;
    virtual char draw() const override;
    virtual std::string getName() const override;

    int getAbilityTurnsLeft() const;
    int getCooldownTurnsLeft() const;
    void setAbilityTurnsLeft(int turns);
    void setCooldownTurnsLeft(int turns);

protected:
    virtual Organism* createChild(int x, int y) override;

private:
    void tryActivateAbility();
    bool isAbilityActive() const;
};

#endif
