#include "Graphics.hpp"
#include <windows.h>
#include <iostream>

using std::cout;

static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void setColor(int color) {
    SetConsoleTextAttribute(hConsole, color);
}

void resetColor() {
    SetConsoleTextAttribute(hConsole, 7);
}

int getTileColor(char symbol) {
    switch (symbol) {
        case 'H': return 14 + (6 << 4); // light yellow + yellow bg - HUMAN
        case 'W': return 13 + (5 << 4); // light purple + purple bg - WOLF
        case 'S': return 15 + (7 << 4); // white + light gray - SHEEP
        case 'F': return 12 + (4 << 4); // light red + red - FOX
        case 'T': return 11 + (3 << 4); // light aqua + aqua - TURTLE
        case 'A': return 9 + (1 << 4); // light blue + blue - ANTELOPE

        case 'g':
        case 's':
        case 'u':
        case 'b':
        case 'h':
            return 2; // all plants =  green

        default: return 7;
    }
}

void printColoredTile(char symbol) {
    setColor(getTileColor(symbol));
    cout << symbol << ' ';
    resetColor();
}

void printEmptyTile() {
    setColor(8); // dark gray
    cout << ". ";
    resetColor();
}
