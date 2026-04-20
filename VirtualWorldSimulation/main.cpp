
/* ============ This is the main file of the project - VirtualWorldSimulator ============

This file is responsible for compiling all pieces of code into one file that runs the whole code.

*/

using namespace std;
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>

// --------------------------------------------------------- headers

#include "World.hpp"
#include "Human.hpp"
#include "animals/Wolf.hpp"
#include "animals/Sheep.hpp"
#include "animals/Fox.hpp"
#include "animals/Turtle.hpp"
#include "animals/Antelope.hpp"
#include "plants/Grass.hpp"
#include "plants/SowThistle.hpp"
#include "plants/Guarana.hpp"
#include "plants/Belladonna.hpp"
#include "plants/Hogweed.hpp"

// ============================================= saved files list =============================================

vector<string> listSaveFiles(const string& folder) {
    vector<string> files;

    if (!filesystem::exists(folder)) {
        filesystem::create_directory(folder);
    }

    for (const auto& entry : filesystem::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path().string());
        }
    }
    return files;
}

// ============================================= main =============================================

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    World world(10, 10); // --------------------------------------------------------- board size

    // --------------------------------------------------------- starting positions
    world.addOrganism(new Human(5, 5, &world));

    world.addOrganism(new Wolf(2, 2, &world));
    world.addOrganism(new Wolf(3, 2, &world));
    world.addOrganism(new Sheep(1, 1, &world));
    world.addOrganism(new Sheep(2, 1, &world));
    world.addOrganism(new Fox(4, 1, &world));
    world.addOrganism(new Turtle(8, 8, &world));
    world.addOrganism(new Antelope(0, 4, &world));

    world.addOrganism(new Grass(6, 6, &world));
    world.addOrganism(new Grass(7, 1, &world));
    world.addOrganism(new SowThistle(2, 8, &world));
    world.addOrganism(new Guarana(7, 7, &world));
    world.addOrganism(new Belladonna(0, 9, &world));
    world.addOrganism(new Hogweed(9, 0, &world));

// ============================================= board printing =============================================
    int input;

    do {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

        world.drawWorld();

        // --------------------------------------------------------- controls and ability status
        cout << "\nControls:\n";
        cout << "movement - arrow keys\n";
        cout << "x - Human ability, n - next turn, q - quit\n";
        cout << "k - save world, l - load world\n";

        Human* human = world.getHuman();
        if (human != nullptr) {
            if (human->getAbilityTurnsLeft() > 0) {
                cout << "Human ability: ACTIVE (" << human->getAbilityTurnsLeft() << " turns left)\n";
            } else if (human->getCooldownTurnsLeft() > 0) {
                cout << "Human ability: COOLDOWN (" << human->getCooldownTurnsLeft() << " turns left)\n";
            } else {
                cout << "Human ability: READY\n";
            }
        }

        // --------------------------------------------------------- symbols on map meanings
        cout << "\nWorld-map Legend:\n";
        cout << "ANIMALS: H - Human, W - Wolf, S - Sheep, F - Fox, T - Turtle, A - Antelope;\n";
        cout << "PLANTS: g - Grass, s - Sowthistle, u - Guarana, b - Belladonna, h - Hogweed;\n";

        cout << "\nEnter command: ";

        // --------------------------------------------------------- player input
        input = _getch();

        if (input == 0 || input == 224) {
            int arrow = _getch();

            if (arrow == 72) { // Up
                world.setHumanMove(0, -1);
                world.makeTurn();
            } else if (arrow == 80) { // Down
                world.setHumanMove(0, 1);
                world.makeTurn();
            } else if (arrow == 75) { // Left
                world.setHumanMove(-1, 0);
                world.makeTurn();
            } else if (arrow == 77) { // Right
                world.setHumanMove(1, 0);
                world.makeTurn();
            }
        } else if (input == 'x' || input == 'X') {
            world.requestSpecialAbility();
            world.makeTurn();
        } else if (input == 'n' || input == 'N') {
            world.makeTurn();
        } else if (input == 'k' || input == 'K') { // --------------------------------------------------------- file: saving
            string folder = "saves";
            if (!filesystem::exists(folder)) {
                filesystem::create_directory(folder);
            }

            string filename = folder + "/save_" + to_string(std::time(nullptr)) + ".txt";

            if (world.saveToFile(filename)) {
                cout << "\nSaved to: " << filename << "\n";
            } else {
                cout << "\nSave failed.\n";
            }
            _getch();

        } else if (input == 'l' || input == 'L') { // --------------------------------------------------------- file: loading
            vector<string> saves = listSaveFiles("saves");

            system("cls");
            cout << "Saved files:\n\n";

            if (saves.empty()) {
                cout << "No save files found.\n";
                _getch();
            } else {
                for (size_t i = 0; i < saves.size(); i++) {
                    cout << i + 1 << ". " << saves[i] << '\n';
                }

                cout << "\nChoose file number: ";
                int choice;
                cin >> choice;

                if (choice >= 1 && choice <= static_cast<int>(saves.size())) {
                    if (world.loadFromFile(saves[choice - 1])) {
                        cout << "\nLoad successful.\n";
                    } else {
                        cout << "\nLoad failed.\n";
                    }
                } else {
                    cout << "\nInvalid choice.\n";
                }
                _getch();
            }
        }

    } while (input != 'q' && input != 'Q'); // --------------------------------------------------------- exit game

    return 0;
}
