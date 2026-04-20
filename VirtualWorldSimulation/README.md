## 2D virtual world simulator:

* NxM sized grid
* lifeforms with different behaviours
* each organism occupies one cell from the grid at a time
* collisions remove one of the two organisms in a cell
* The simulation should be initiated with several instances of every kind of organism already placed in the game world.



The program window should include a text box
for displaying messages about the results of fights between animals, consumption of plants
and other events occurring inside the simulated world

* Student's first name, last name and index number should be displayed in the application user interface.



##### **ACTION performed in TURNS:**

* every turn, organism performs action (appropriate to their kind)
* some move, while some stay in one place
* regular animals have random movement (unlike human)



##### **The order of actions** during a single turn depends on the initiative of each organism:

* The animals with the highest initiative move first
* In case of animals with the same initiative, the order is determined by the animal's age (the oldest animal will move first)



##### **COLLISIONS:** (one of the organisms enters a cell occupied by another organism)

* one of them wins (either by killing its opponent or by reflecting the opponent's attack)
* victory depends on the strength of  organisms - the stronger organism wins (with some exceptions)
* In case of equal strength, the encounter is won by the attacker



##### **HUMAN PLAYER:**

* unlike regular animals, his movement is not random
* the direction of the human's movement is determined by the player by pressing the appropriate arrow key before the start of every round
* special ability (see Appendix A. Human special abilities), which can be activated with a separate key.
-- Once activated, the ability works for 5 turns
-- after it is automatically deactivated
-- After deactivation, the ability cannot be activated for the next 5 turns



#### **Tips for implementation:**



1. ###### Create a class named World which will manage the gameplay and life cycle of organisms
* It should contain methods such as:
-- makeTurn()
-- drawWorld()
* and fields such as:
-- organisms



2. ###### Create an abstract class named Organism which will be a base for Animals and Plants:
* basic fields:
-- strength,
-- initiative,
-- position (x,y),
-- world - reference (pointer) to the cell of the world grid in which the organism is placed
* basic methods:
-- action() - basic behavior of organism should be implemented in this method,
-- collision() - behavior of organism in case of collision with other organism,
-- draw() - method which draws or returns symbolic or graphical representation of the organism.
* Class Organism should be abstract, and should be the base class for **two other abstract classes: Plant and Animal.**



3. ###### Common behaviors for all animals should be implemented in the class Animal.
* Behavior such as:
-- base movement in method action() - every typical animal moves to a randomly selected neighboring field,
-- multiplication in method collision():
---> when two animals of the same species collide, instead of fighting with each other, both animals remain in their original positions, and next to them a new animal of their species is created.



4. ###### Class Human should extend the class Animal:
* Human does not implement random movement nor multiplication, but instead is controlled by the player
* There should be only one instance of Human on the map.



###### **Table 1 Human class properties**

|strength|initiative|action()|collision()|
|-|-|-|-|
|5|4|direction of movement corresponds to arrow keys pressed by the player|special ability Appendix A. Human|



###### **Implement 5-6 subclasses of Animal chosen from below:**

|id|animal|strength|initiative|action()|collision()|
|-|-|-|-|-|-|
|1|wolf|9|5|default for Animal|default for Animal|
|2|sheep|4|4|default for Animal|default for Animal|
|3|fox|3|7|Good sense of smell: fox will never mote to a cell occupied by a stronger organism|default for Animal|
|4|turtle|2|1|Has 75% chance to stay in the same place|Reflects attacks of animal with strength less than 5. Attacker<br />will return to the previous cell.|
|5|antelope|4|4|Has wider range of movement - 2 fields instead of 1.|Has 50% chance to escape from fight. In such case it moves to a free neighboring cell.|
|6|cyber-sheep<br />(mandatory only in the third project)|11|4|main goal = the<br />extermination of<br />Sosnowsky's hogweed. It<br />always moves towards the<br />closes hogweed and tries<br />to eat it. If there are no<br />Sosnowsky's hogweeds, it<br />behaves like a normal<br />sheep.|Eats Sosnowsky's<br />hogweed.|



###### **In class Plant implement common behaviours for all plants:**

* spreading of plant in method action() - with a certain probability the plant can "sow" a new plant on a random free neighbouring field.

Initiative for all plants is 0.



###### **Table 3 Description of plant classes Implement 5 classes of plants.**

|id|plant|strength|action()|collision()|
|-|-|-|-|-|
|1|grass|0|default for Plant|default for Plant|
|2|sow thistle|0|Performs 3 attempts at spreading each turn|default for Plant|
|3|guarana|0|default for Plant|Strength of the animal which ate it is permanently increased by 3|
|4|belladonna|99|default for Plant|Kills any animal which eats it|
|5|Sosnowsky's hogweed|10|Kills every animal in its immediate neighborhood (except cyber-sheep)|Kills any animal which eats it (except cyber-sheep)|



Create the class World which will contain objects of class Organism.

Implement turns by calling the action() method for every organism on the map and collision() for organisms on the same cell.

Remember that the order of calling the action() method depends on the initiative (and age) of the organisms.



Organisms can affect the world state (eg. by spawning new organisms).

Therefore it is necessary to pass to methods action() and collision() a reference to the instance of the World class.

Remember: class World should define as public only such methods and fields that are required by other objects. The rest of its methods and fields should be private or protected.



##### **Project 1. C++**

Visualization of the virtual world should be performed in console. Every organism should be presented as a different ASCII symbol. Pressing one key should cause a transition to the next turn, clearing the console and printing all symbols in their new states. At least one line of text in console should be used to report results of events such as fights, spawning new animals/plants and eating.



###### **Suggested marks:**



3 points:

* the world and its visualization,
* all required animals without breeding,
* all plants without sawing,
* Human which can controlled by arrow keys.



4 points, everything above, and:

* breeding of animals and sawing of plants,
* implementation of Human's special ability.



5 points:

* Implementation of saving and loading state of the world to file and from file.





NOTES:

* store all organisms in std::vector<Organism> organisms
* when drawing the world, check which organism is at each x,y
* optionally keep a helper function like getOrganismAt(x,y)



World class:

* board dimensions
* all organisms
* turn number
* event log/messages
* human input direction
* creating/removing organisms
* drawing the board
* executing turns in correct order



Turn looks like this:

1. clear previous lines
2. sort organisms by highest initiative, oldest
3. call action for each organism that is still alive
4. remove dead organisms
5. increase age of organisms
6. redraw world and print messages



Board visualization:

* turn nr
* board
* legend: all organisms
* events: listing what happened where this turn
* \-- one ASCII char per organism



drawWorld() function -> loop goes through board and draws organisms



file structure:

main.cpp



World.hpp, World.cpp

Organism.hpp, Organism.cpp

Position.hpp - coordinates tracking



Animal.hpp, Animal.cpp

Plant.hpp, Plant.cpp

Human.hpp, Human.cpp



1. Wolf.hpp, Wolf.cpp
2. Sheep.hpp, Sheep.cpp
3. Fox.hpp, Fox.cpp
4. Turtle.hpp, Turtle.cpp
5. Antelope.hpp, Antelope.cpp



1. Grass.hpp, Grass.cpp
2. SowThistle.hpp, SowThistle.cpp
3. Guarana.hpp, Guarana.cpp
4. Belladonna.hpp, Belladonna.cpp
5. Hogweed.hpp, Hogweed.cpp



