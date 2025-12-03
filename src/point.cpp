#include "map_gen.hpp"

Point::Point(int x, int y)
{
    this->x = x;
    this->y = y;
}

Point::Point()
{
    x = 0;
    y = 0;
}

void Point::rotate90R()
{
    int newX = -y;
    int newY = x;

    x = newX;
    y = newY;
}

void Point::rotate90L()
{
    int newX = y;
    int newY = -x;

    x = newX;
    y = newY;
}

void Point::translate(int x, int y)
{
    this->x += x;
    this->y += y;
}

void Point::invert(bool invertX)
{
    if (invertX)
        x *= -1;
    else
        y *= -1;
}

string Point::to_string()
{
    return "{" + std::to_string(x) + ", " + std::to_string(y) + "}";
}