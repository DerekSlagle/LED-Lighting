#include "lightGroupImage.h"


 void lightGroupImage::init( const sf::Image& rImg, myLight* pLt, sf::IntRect SrcRect,
            myLight* pGrid, unsigned int GridRows, unsigned int GridCols, float Scale )
{
    pImg = &rImg;
    scale = Scale;
    srcRect = SrcRect;
 //   const sf::Vector2u imgDim = pImg->getSize();
    lightGroupGrid::init( pLt, srcRect.height*scale, srcRect.width*scale, pGrid, GridRows, GridCols );
}

myLight lightGroupImage::getLight( unsigned int Row, unsigned int Col )const
{
    if( pImg && Row < rows && Col < cols )
    {
        sf::Color imgColor = pImg->getPixel ( srcRect.left + Col/scale, srcRect.top + Row/scale );
        return myLight( imgColor.r, imgColor.g, imgColor.b, 1.0f );
    }
    else
        return myLight();
}

void lightGroupImage::draw()const
{
    if( !( pImg && pGrid0 && pLt0 ) ) return;

    if( scale == 1.0f )
    {
        if( isTransparent )
            return drawTrueTransparent();
        return drawTrue();
    }
    else if( isTransparent )
        return drawTransparent();

    // fall through is not transparent && scale != 1
    sf::Color imgColor;
    for( unsigned int c = 0; c < cols; ++c )
    {
        for( unsigned int r = 0; r < rows; ++r )
        {
            imgColor = pImg->getPixel ( srcRect.left + c/scale, srcRect.top + r/scale );
            get_pLt(r,c)->init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

void lightGroupImage::drawTransparent()const
{
    sf::Color imgColor;
    for( unsigned int c = 0; c < cols; ++c )
    {
        for( unsigned int r = 0; r < rows; ++r )
        {
            imgColor = pImg->getPixel ( srcRect.left + c/scale, srcRect.top + r/scale );
            if( imgColor.a > 0 )
                get_pLt(r,c)->init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

void lightGroupImage::drawTrue()const
{
    sf::Color imgColor;
    for( unsigned int c = 0; c < cols; ++c )
    {
        for( unsigned int r = 0; r < rows; ++r )
        {
            imgColor = pImg->getPixel ( srcRect.left + c, srcRect.top + r );
            get_pLt(r,c)->init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

void lightGroupImage::drawTrueTransparent()const
{
    sf::Color imgColor;
    for( unsigned int c = 0; c < cols; ++c )
    {
        for( unsigned int r = 0; r < rows; ++r )
        {
            imgColor = pImg->getPixel ( srcRect.left + c/scale, srcRect.top + r/scale );
            if( imgColor.a > 0 )
                get_pLt(r,c)->init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}
