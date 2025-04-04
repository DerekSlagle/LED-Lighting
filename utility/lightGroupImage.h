#ifndef LIGHTGROUPIMAGE_H
#define LIGHTGROUPIMAGE_H

#include <SFML/Graphics.hpp>
#include "myLight.h"

// inherited pLt0 for use to read from sf::Image, not another source
class lightGroupImage : public lightGroupGrid
{
    public:
    const sf::Image* pImg = nullptr;
    sf::IntRect srcRect;
    float scale = 1.0f;
    bool isTransparent = true;

    void draw()const;// general scale and transparency
    virtual myLight getLight( unsigned int Row, unsigned int Col )const;


    void init( const sf::Image& rImg, myLight* pLt, sf::IntRect SceRect,
            myLight* pGrid, unsigned int GridRows, unsigned int GridCols, float Scale = 1.0f );

    lightGroupImage( const sf::Image& rImg, myLight* pLt, sf::IntRect SrcRect,
            myLight* pGrid, unsigned int GridRows, unsigned int GridCols, float Scale = 1.0f )
    { init( rImg, pLt, SrcRect, pGrid, GridRows, GridCols, Scale ); }


    lightGroupImage(){}
    virtual ~lightGroupImage(){}

    protected:

    private:
    // called by draw()
    void drawTransparent()const;
    void drawTrue()const;// scale = 1
    void drawTrueTransparent()const;// scale = 1
};

#endif // LIGHTGROUPIMAGE_H
