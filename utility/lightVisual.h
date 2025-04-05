#ifndef LIGHTVISUAL_H
#define LIGHTVISUAL_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "myLight.h"

class lightVisual
{
    public:
 //   static std::vector<sf::Vertex> allLtsVec;

    const myLight* pLight = nullptr;// use for single lights
    const lightGroup* pGroup = nullptr;// use for a group
    std::vector<sf::Vertex> vtxVec;// for entire group
    unsigned int numLts = 0;// useful in methods

    // for a single light
    void init( const myLight* p_Light, float posX, float posY, sf::Vector2f Sz );
    lightVisual( const myLight* p_Light, float posX, float posY, sf::Vector2f Sz )
    { init( p_Light, posX, posY, Sz ); }
    // group is entire grid. Rectangular layout
    void init( lightGroup& r_Group, float posX, float posY, float dPosX, float dPosY, sf::Vector2f LtSz );
    lightVisual( lightGroup& r_Group, float posX, float posY, float dPosX, float dPosY, sf::Vector2f LtSz )
    { init( r_Group, posX, posY, dPosX, dPosY, LtSz ); }
    // Circular layout. rows = 1 only
    void init( lightGroup& r_Group, float ctrPosX, float ctrPosY, float Radius, sf::Vector2f LtSz );
    lightVisual( lightGroup& r_Group, float ctrPosX, float ctrPosY, float Radius, sf::Vector2f LtSz )
    { init( r_Group, ctrPosX, ctrPosY, Radius, LtSz ); }

    sf::FloatRect getRect()const;
    sf::Vector2f getLtSize()const;
    bool isOver( float x, float y )const;

    void update();
    void draw( sf::RenderTarget& RT ) const;

    lightVisual(){}
    ~lightVisual(){}
};

#endif // LIGHTVISUAL_H
