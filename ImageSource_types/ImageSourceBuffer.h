#ifndef IMAGESOURCEBUFFER_H
#define IMAGESOURCEBUFFER_H

#include <SFML/Graphics.hpp>
#include <iostream>

class ImageSourceBuffer
{
    private:
 //   const sf::Image* pSrcImg = nullptr;
 //   sf::IntRect srcRect;

    sf::Image* pDestImg = nullptr;
//    sf::IntRect destRect;

    public:
    const sf::Image* pSrcImg = nullptr;// must be assignable
    sf::IntRect srcRect;// must be assignable
    sf::IntRect destRect;
    void draw()const;

    void init( const sf::Image& rSrcImg, sf::IntRect SrcRect, sf::Image& rDestImg, sf::IntRect DestRect );
    ImageSourceBuffer( const sf::Image& rSrcImg, sf::IntRect SrcRect, sf::Image& rDestImg, sf::IntRect DestRect )
    { init( rSrcImg, SrcRect, rDestImg, DestRect ); }

    ImageSourceBuffer();
    virtual ~ImageSourceBuffer();

    protected:
};

#endif // IMAGESOURCEBUFFER_H
