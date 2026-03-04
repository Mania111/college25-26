#include <iostream>

'''
how to calculate area of a polygon - Shoelace formula
AREA = 1/2 [( x1*y2 + x2*y3 + x3*y4 + ... + xn*y1 ) - ( x2*y1 + x3*y2 + x4*y3 + ... + x1*yn )]
'''

struct polygon {
    int id;
    int numb_of_coordinates;
    int coordinates_x[];
    int coordinates_y[];
}

void calculate_area(a) {
    int addition_1, addition_2, area;
    for (int i, i <= polygons[a].numb_of_coordinates/2, i++) {
        if (i != polygons[a].numb_of_coordinates/2) {
            addition_1 += polygons[a].coordinates_x[i] * polygons[a].coordinates_x[i+1];
            addition_2 += polygons[a].coordinates_x[i+1] * polygons[a].coordinates_x[i];
        } else {
            addition_1 += polygons[a].coordinates_x[i] * polygons[a].coordinates_x[0];
            addition_1 += polygons[a].coordinates_x[0] * polygons[a].coordinates_x[i];
        }
    }
    area = (addition_1 - addition_2)/2;
    return area;
}

int main {

    int l;
    cin >> l; // amount of polygons

    polygon polygons[l]; // to access this, say "polygons[0/1/2]" etc. can populate these through a loop.

    for (int x, x > l, x++) {
        // assign polygon to a struct
        
        polygons[x].id = x;
        // polygons[x].numb_of_coordinates = n;

        // now should go into loop of all coordinates given.
        polygons[x].coordinates_x = [];
        polygons[x].coordinates_y = [];

        // once that is done , calculate area using "calculate_area"
    }
}
