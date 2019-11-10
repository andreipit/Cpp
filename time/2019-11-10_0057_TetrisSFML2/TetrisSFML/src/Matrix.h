#pragma once
#include <SFML/System/Vector2.hpp>
#include "Lines.hpp"
#include "Object.h"

class Matrix
{
    sf::Vector2u m_size;
    Lines m_cellTable;

public:
    Matrix(sf::Vector2u size);
    void add(const Object& object);
    bool checkCollision(const Object& object);
    void draw( sf::RenderWindow& window);
    void clear();

private:
    void removeFullLines();
    void makeBorders();
};


