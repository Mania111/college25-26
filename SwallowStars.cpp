using namespace std;
#include <iostream>
#include <conio.h>
#include <cstdlib> // for system("cls") - clear output
#include <vector> // learning vectors for this ^.^ yay

// config (will need to be moved to separate file later)
#define BoardSizeX 50
#define BoardSizeY 15 // board sizes
int Px = BoardSizeX/2;
int Py = BoardSizeY-1; // player STARTING coordinates
int currentSpeed = 1; // self explantory
int level = 0; // level increases every time Player collects a star
int timeLeft = 100; // when timer reaches 0, game ends
int starsToCatch = 5; // amount of stars to catch in order to win the game
int starsCaught = 0;
// maximum stars on board = 5
vector<int>starsX = {};
vector<int>starsY = {};
vector<int>starsSpeed = {};
// these correspond to one another
int lifeForce = 25; // health points of Player


void printBoard(){    // print current board
    for ( int y = 0; y <= BoardSizeY+1; y++ ) {
        for ( int x = 0; x <= BoardSizeX+1; x++) {
            if (y == 0 || y == BoardSizeY+1) { // top or bottom
                if (x==0 || x==BoardSizeX+1) { // left or right corner
                    cout << "o";
                }
                else cout << "-";
                continue;
            }
            if (x==0 || x==BoardSizeX+1) {
                cout << "|";
                continue;
            }
            if (x==Px && y==Py) {
                cout << "P";
                continue;
            }
            bool starFound = false;
            for (int i = 0; i < starsY.size(); i++) {
                if (starsX[i] == x && starsY[i] == y) {
                    cout << "*";
                    starFound = true;
                    break;
                }
            }
            if (!starFound)
                cout << " ";
        }
        cout << endl;
    }
    cout << endl;
}

void printStatus(){
    cout << "---------------- STATUS ----------------" << endl;
    cout << "Speed | Level | Time | Stars | Life Force" << endl;
    cout << "  " << currentSpeed << "       " << level << "     " << timeLeft << "       " << starsCaught << "/" << starsToCatch << "        " << lifeForce << endl;
    cout << "----------------------------------------" << endl;
}

void spawnStars() {
    int starChance = rand() % 4; // 1 , 2 or 3
    if (starChance == 3) { // star gets created
        int starSpawnLocation = (rand() % (BoardSizeX-2))+1;
        int starSpeed = (rand() % 5)+1; // 1-5
        starsX.push_back(starSpawnLocation);
        starsY.push_back(1);
        starsSpeed.push_back(starSpeed);
    }
}

void moveStars() {
    for (int i = (int)starsY.size() - 1; i >= 0; i--) {
        starsY[i] = starsY[i] + starsSpeed[i];
        if (starsY[i] > BoardSizeY) {
            starsX.erase(starsX.begin() + i);
            starsY.erase(starsY.begin() + i);
            starsSpeed.erase(starsSpeed.begin() + i);
        }
    }
}

int main()
{
    srand(time(NULL)); // srand() returns random number , while time(null) gives current time in seconds -> so this "seed" will always be different

    printBoard();
    printStatus();

    while (true) {
        char action = getch(); // wasd

        if (action == 'w' && Py > 1) { // boundries for Player
            if (Py - currentSpeed < 1) {
                Py = 1;
            }
            else Py = Py - currentSpeed;
        }
        else if (action == 's' && Py < BoardSizeY) {
            if (Py + currentSpeed > BoardSizeY) {
                Py = BoardSizeY;
            }
            else Py = Py + currentSpeed;
        }
        else if (action == 'a' && Px > 1) {
            if (Px - currentSpeed < 1) {
                Px = 1;
            }
            else Px = Px - currentSpeed;
        }
        else if (action == 'd' && Px < BoardSizeX) {
            if (Px + currentSpeed > BoardSizeX) {
                Px = BoardSizeX;
            }
            else Px = Px + currentSpeed;
        }
        else if (action == 'o' && currentSpeed > 1) {
            currentSpeed--;
        }
        else if (action == 'p' && currentSpeed < 5) {
            currentSpeed++;
        }

        spawnStars();
        moveStars();

        timeLeft = timeLeft - 1;

        system("cls");
        printBoard();
        printStatus();
    }
    return 0;
}
