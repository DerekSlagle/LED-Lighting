#include "ImageSourceBuffer.h"

ImageSourceBuffer::ImageSourceBuffer()
{
    //ctor
}

ImageSourceBuffer::~ImageSourceBuffer()
{
    //dtor
}

void ImageSourceBuffer::init( const sf::Image& rSrcImg, sf::IntRect SrcRect, sf::Image& rDestImg, sf::IntRect DestRect )
{
    pSrcImg = &rSrcImg;
    srcRect = SrcRect;

    pDestImg = &rDestImg;
    destRect = DestRect;

    // for now with scale = 1.0f only
    destRect.width = srcRect.width;
    destRect.height = srcRect.height;
}

void ImageSourceBuffer::draw()const
{
 //   std::cout << "\nISB.draw() ";
    sf::Color imgColor;
    const int destSzX = static_cast<int>( pDestImg->getSize().x );
    const int destSzY = static_cast<int>( pDestImg->getSize().y );

    for( int r = 0; r < srcRect.height; ++r )
    {
        if( r + destRect.top < 0 || r + destRect.top >=  destSzY ) continue;// no wrap

        for( int c = 0; c < srcRect.width; ++c )
        {
            if( c +  destRect.left < 0 || c + destRect.left >=  destSzX ) continue;// column bound check no wrap
            imgColor = pSrcImg->getPixel ( srcRect.left + c, srcRect.top + r );
            pDestImg->setPixel( c + destRect.left, r + destRect.top, imgColor );

         //   if( imgColor.r != 0 ) std::cout << "red! ";
         //   if( imgColor.g != 0 ) std::cout << "green! ";
         //   if( imgColor.b != 0 ) std::cout << "blue! ";
        }
    }
}
