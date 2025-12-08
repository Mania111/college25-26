using namespace std;
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <conio.h>
#include <cstdlib> // for system("cls") - clear output
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <windows.h>

// global variables
int Px;
int Py;
int BoardSizeX;
int BoardSizeY;
int currentSpeed;
int minSpeed;
int maxSpeed;
int timer = 0;
int timeLeft;
// STARS - - - - -
double starSpawnRate;
int starsToCatch; // amount of stars to catch in order to win the game
int starsCaught = 0;
vector<int>starsX = {};
vector<int>starsY = {};
vector<int>starsSpeed = {};
// HUNTERS - - - - -
double hunterSpawnRate;
int hunterMaxWidth;
int hunterMaxHeight;
vector<int>huntersX = {};
vector<int>huntersY = {};
vector<int>huntersDirectionsX = {};
vector<int>huntersDirectionsY = {};
vector<int>huntersSpeed = {}; // const
vector<int>huntersBounces = {}; // when bounce reaches 0 , disappear
vector<int>huntersHeights = {};
vector<int>huntersLengths = {};
// - - - - - -
int playerLifeForce; // health points of Player
int swallowFrame = 0; // for 0 print "v" , for 1 print "-" , for 2 print "^" for 3 print "-" and keep cycling
char lastMovementMade = 's'; // for constant swallow motion
int level = 1;
vector<vector<int>> occ; // OCCUPANCY GRID
int damageDecline; // damage decline setting -> can be 0 (off) or 1 (on)
// damage decline makes it so that Hunters deal more damage when the Player is on low health
int declineStart; // the moment the decline starts working

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // console handle
// Reusable function to set color
void setColor(int color) {
    SetConsoleTextAttribute(hConsole, color);
}

// Function to trim unnecessary spaces in config.txt in case those appear
void trim(char* str) {
    int len = strlen(str);
    while (len > 0 && str[len - 1] == ' ') {
        str[len - 1] = '\0';
        len--;
    }
}

void loadConfig(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Could not open %s\n", filename);
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char key[99];
        double value;
        if (sscanf(line, " %99[^=]=%lf", key, &value) == 2) {
            trim(key);
            if (strcmp(key, "BoardSizeX") == 0) BoardSizeX = value;
            else if (strcmp(key, "BoardSizeY") == 0) BoardSizeY = value;
            else if (strcmp(key, "starsToCatch") == 0) starsToCatch = value;
            else if (strcmp(key, "playerLifeForce") == 0) playerLifeForce = value;
            else if (strcmp(key, "starSpawnRate") == 0) starSpawnRate = value;
            else if (strcmp(key, "hunterSpawnRate") == 0) hunterSpawnRate = value;
            else if (strcmp(key, "minSpeed") == 0) minSpeed = value;
            else if (strcmp(key, "maxSpeed") == 0) maxSpeed = value;
            else if (strcmp(key, "damageDecline") == 0) damageDecline = value;
            else if (strcmp(key, "declineStart") == 0) declineStart = value;
            else if (strcmp(key, "timeLeft") == 0) timeLeft = value;
        }
    }
    fclose(file);
}

void startingScreen() {
    string title[] = {
    R"( ____  _  _   __   __    __     __   _  _    ____  ____  __   ____  ____ )",
    R"(/ ___)/ )( \ / _\ (  )  (  )   /  \ / )( \  / ___)(_  _)/ _\ (  _ \/ ___))",
    R"(\___ \\ /\ //    \/ (_/\/ (_/\(  O )\ /\ /  \___ \  )( /    \ )   /\___ \)",
    R"((____/(_/\_)\_/\_/\____/\____/ \__/ (_/\_)  (____/ (__)\_/\_/(__\_)(____/)"
    };

    int numLines = sizeof(title) / sizeof(title[0]);
    for (int i = 0; i < numLines; i++) {
        cout << title[i] << endl;
    }
    cout << "           Press Any Key To Play";

    getch();
    system("cls");
}

void endingScreen() {
    system("cls");

    string endingTitle[] = {
        R"(  ___   __   _  _  ____     __   _  _  ____  ____ )",
        R"( / __) / _\ ( \/ )(  __)   /  \ / )( \(  __)(  _ \)",
        R"(( (_ \/    \/ \/ \ ) _)   (  O )\ \/ / ) _)  )   /)",
        R"( \___/\_/\_/\_)(_/(____)   \__/  \__/ (____)(__\_))"
    };

    int numLines = sizeof(endingTitle) / sizeof(endingTitle[0]);
    for (int i = 0; i < numLines; i++) {
        cout << endingTitle[i] << endl;
    }
    cout << "\n       Player Score: " << (timeLeft-timer) * starsCaught << endl;
    cout << "         Thank you for Playing!\n";
    cout << "\n       Press any key to exit...";
    getch();
}


void printBoard(){    // print current board
    for ( int y = 0; y <= BoardSizeY+1; y++ ) {
        for ( int x = 0; x <= BoardSizeX+1; x++) {
            setColor(8);
            if (y == 0 || y == BoardSizeY+1) { // top or bottom
                if (x==0 || x==BoardSizeX+1) { // left or right corner
                    cout << "o";
                }
                else cout << "-";
                continue;
            }
            if (x==0) {
                cout << "|";
                continue;
            }
            if (x==BoardSizeX+1) {
                cout << "|                                      ";
                continue;
            }
            if (x==Px && y==Py) {
                if (playerLifeForce>10) {
                    setColor(11);
                }
                else if (playerLifeForce <= 10 && playerLifeForce > 5)
                    setColor(14);
                else setColor(4);
                if (swallowFrame%4==0) {
                    cout << "v";
                }
                else if (swallowFrame%4==1) {
                    cout << "-";
                }
                else if (swallowFrame%4==2) {
                    cout << "^";
                }
                else {
                    cout << "-";
                }
                continue;
            }
            bool drewSomethingHere = false;

            for (int i = 0; i < int(starsX.size()); i++) {
                if (starsX[i] == x && starsY[i] == y) {
                    setColor(6);
                    cout << "*";
                    drewSomethingHere = true;
                    break;
                }
            }
            if (drewSomethingHere) {
                continue; // skips hunter loop if star drawn
            }

            for (int i = 0; i < int(huntersX.size()); i++) {
                int hx = huntersX[i];
                int hy = huntersY[i];
                int hlen = huntersLengths[i];
                int hh = huntersHeights[i];

                if (x >= hx && x < hx + hlen && y >= hy && y < hy + hh) {
                    if (huntersSpeed[i] == 1) {
                        setColor(13);
                    }
                    else if (huntersSpeed[i] == 2) {
                        setColor(9);
                    }
                    if (huntersSpeed[i] == 3) {
                        setColor(12);
                    }
                    cout << "#";
                    drewSomethingHere = true;
                    break;
                }
            }

            if (!drewSomethingHere)
                cout << " ";
        }
        cout << endl;
    }
    cout << endl;
    swallowFrame++;
}

void printStatus() {
    setColor(7); // default grey
    cout << "----------------";
    setColor(3); // aqua
    cout << " STATUS ";
    setColor(7);
    cout << "----------------" << endl;

    setColor(15); // bright white
    cout << "Speed | Level | Time | Stars | Life Force" << endl;

    cout << "  ";
    setColor(12); // red
    cout << currentSpeed << "       ";
    setColor(14); // yellow
    cout << level << "     ";
    setColor(10); // green
    cout << timer <<"/"<< timeLeft << "    ";
    setColor(11); // light cyan
    cout << starsCaught << "/" << starsToCatch << "        ";
    setColor(9); // default grey
    cout << playerLifeForce << " " << endl;

    setColor(7);
    cout << "----------------------------------------" << endl;
}

void spawnStars() {
    if ((double)rand() / RAND_MAX < starSpawnRate) { // star gets created
        int starSpawnLocation = (rand() % BoardSizeX)+1;
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
    for (int i = 0; i < int(starsX.size()); i++) {
        if (starsX[i] >= 1 && starsX[i] <= BoardSizeX &&
            starsY[i] >= 1 && starsY[i] <= BoardSizeY)
        {
            occ[starsY[i]][starsX[i]] = 2;
        }
    }
}

void checkIfStarCaught() {
    if (occ[Py][Px] == 2) { // star in player's current cell
        starsCaught++;

        // remove all stars that currently occupy player's position
        for (int i = starsX.size() - 1; i >= 0; i--) {
            if (starsX[i] == Px && starsY[i] <= Py && starsY[i] + starsSpeed[i] >= Py) {
                starsX.erase(starsX.begin()+i);
                starsY.erase(starsY.begin()+i);
                starsSpeed.erase(starsSpeed.begin()+i);
            }
        }
    }
}

void spawnHunters() {
    // Base spawn chance
    double spawnChance = hunterSpawnRate;
    if (timer > (timeLeft / 2)) spawnChance += 0.3; // increase spawn after halfway

    if ((double)rand() / RAND_MAX < spawnChance) {

        int hunterBounces = (timer > (timeLeft / 2)) ? ((rand() % 10) + 3) : ((rand() % 5) + 1);
        huntersBounces.push_back(hunterBounces);

        int side = rand() % 4;
        int hx, hy;
        switch (side) {
            case 0: hx = 1; hy = (rand() % (BoardSizeY - 2)) + 1; break; // left
            case 1: hx = BoardSizeX; hy = (rand() % (BoardSizeY - 2)) + 1; break; // right
            case 2: hx = (rand() % (BoardSizeX - 2)) + 1; hy = 1; break; // top
            case 3: hx = (rand() % (BoardSizeX - 2)) + 1; hy = BoardSizeY; break; // bottom
        }

        huntersX.push_back(hx);
        huntersY.push_back(hy);

        int hunterSpeed = (rand() % 3) + 1; // 1-3
        huntersSpeed.push_back(hunterSpeed);

        int shapeX = (rand() % hunterMaxWidth) + 1;
        int shapeY = (rand() % hunterMaxHeight) + 1;
        if (shapeX == 1 && shapeY == 1) {
            if (rand() % 2) shapeX++; else shapeY++;
        }
        huntersLengths.push_back(shapeX);
        huntersHeights.push_back(shapeY);

        // Direction toward player
        int dx = (Px > hx) - (Px < hx);
        int dy = (Py > hy) - (Py < hy);
        huntersDirectionsX.push_back(dx);
        huntersDirectionsY.push_back(dy);
    }
}

void moveHunters() {
    for (int i = (int)huntersX.size() - 1; i >= 0; i--) {
        huntersX[i] += huntersDirectionsX[i] * huntersSpeed[i];
        huntersY[i] += huntersDirectionsY[i] * huntersSpeed[i];

        // perfect bouncing = multiplying the movement vector by "-1" on axis that hit the wall !!
        // when hitting a wall:
        // vertical wall → dx *= -1  (flip horizontal movement)
        // horizontal wall → dy *= -1 (flip vertical movement)

        bool bounced = false;

        int w = huntersLengths[i];
        int h = huntersHeights[i];

        if (huntersX[i] < 1) {
            huntersX[i] = 1;
            huntersDirectionsX[i] *= -1;
            bounced = true;
        }
        else if (huntersX[i] + w - 1 > BoardSizeX) {
            huntersX[i] = BoardSizeX - (w - 1);
            huntersDirectionsX[i] *= -1;
            bounced = true;
        }
        // separate X and Y so the corners don't get confusing
        if (huntersY[i] < 1) {
            huntersY[i] = 1;
            huntersDirectionsY[i] *= -1;
            bounced = true;
        }
        else if (huntersY[i] + h - 1 > BoardSizeY) {
            huntersY[i] = BoardSizeY - (h - 1);
            huntersDirectionsY[i] *= -1;
            bounced = true;
        }

        if (bounced) {
            huntersBounces[i]--;
            if (huntersBounces[i] <= 0) {
                huntersX.erase(huntersX.begin()+i);
                huntersY.erase(huntersY.begin()+i);
                huntersSpeed.erase(huntersSpeed.begin()+i);
                huntersLengths.erase(huntersLengths.begin()+i);
                huntersHeights.erase(huntersHeights.begin()+i);
                huntersDirectionsX.erase(huntersDirectionsX.begin()+i);
                huntersDirectionsY.erase(huntersDirectionsY.begin()+i);
                huntersBounces.erase(huntersBounces.begin()+i);
                continue;
            }
        }
        int hx = huntersX[i];
        int hy = huntersY[i];

        int startY = max(1, hy);
        int endY   = min(hy + h, BoardSizeY + 1);
        int startX = max(1, hx);
        int endX   = min(hx + w, BoardSizeX + 1);

        for (int yy = startY; yy < endY; yy++)
            for (int xx = startX; xx < endX; xx++)
                occ[yy][xx] = 3;
    }
}

void checkHunterCollision() {
    if (occ[Py][Px] == 3) { // hunter overlaps player
        for (int i = 0; i < int(huntersX.size()); i++) {
            int hx = huntersX[i];
            int hy = huntersY[i];
            int w  = huntersLengths[i];
            int h  = huntersHeights[i];

            bool overlap = (Px >= hx && Px <= hx + w - 1)
                        && (Py >= hy && Py <= hy + h - 1);

            if (overlap) {
                if (damageDecline && playerLifeForce <= declineStart) {
                    playerLifeForce -= (huntersSpeed[i]*2);
                }
                else {
                    playerLifeForce -= huntersSpeed[i];
                }
                if (playerLifeForce < 0) playerLifeForce = 0;
                break;
            }
        }
    }
}

void hideCursor()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

void moveCursorToTop()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {0, 0};
    SetConsoleCursorPosition(hOut, pos);
}

int main()
{
    loadConfig("config.txt");
    Px = BoardSizeX/2;
    Py = BoardSizeY-1; // player STARTING coordinates

    if (minSpeed <= 0) {
        minSpeed = 1;
    }
    currentSpeed = minSpeed;

    if (hunterMaxWidth <= 0) hunterMaxWidth = 3;
    if (hunterMaxHeight <= 0) hunterMaxHeight = 2;
    if (hunterSpawnRate <= 0) hunterSpawnRate = 0.05; // 5% spawn chance per frame in case the entered one is zero

    srand(time(NULL)); // srand() returns random number , while time(null) gives current time in seconds -> so this "seed" will always be different

    occ.assign(BoardSizeY + 3, vector<int>(BoardSizeX + 3, 0));

    char action = 0;             // always initialized

    hideCursor();
    startingScreen();
    printBoard();
    printStatus();

    while (true) {
        moveCursorToTop();

        // Clear occupancy grid FIRST
        for (int y=0; y<=BoardSizeY+1; y++)
            for (int x=0; x<=BoardSizeX+1; x++)
                occ[y][x] = 0;

        action = 0;
        if (kbhit()) {
            action = getch();
        }

        if (action == 'w' || action == 's' || action == 'a' || action == 'd') {
            lastMovementMade = action; // detect which direction the player will move to
        }
        else if (action == 'o' && currentSpeed > minSpeed) {
            currentSpeed--;
        }
        else if (action == 'p' && currentSpeed < maxSpeed) {
            currentSpeed++;
        }

        switch (lastMovementMade) {
            case 'w':
                Py = max(1, Py - currentSpeed);
                break;
            case 's':
                Py = min(BoardSizeY, Py + currentSpeed);
                break;
            case 'a':
                Px = max(1, Px - currentSpeed);
                break;
            case 'd':
                Px = min(BoardSizeX, Px + currentSpeed);
                break;
        }
        occ[Py][Px] = 1;

        spawnStars();
        moveStars();
        checkIfStarCaught();

        spawnHunters();
        moveHunters();
        checkHunterCollision();

        timer++;

        printBoard();
        printStatus();

        Sleep(100); // delay

        if (playerLifeForce == 0 || timer >= timeLeft || starsCaught >= starsToCatch) {
            break;
        }
    }
    endingScreen();
    return 0;
}
