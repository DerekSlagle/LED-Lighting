#ifndef LVL_IMAGEMAPS_H
#define LVL_IMAGEMAPS_H

#include "Level.h"
#include "../utility/spriteSheetImage.h"
#include "../ImageSource_types/ImageSourceBuffer.h"


class lvl_ImageMaps : public Level
{
    public:

    // for starters
    std::vector<spriteSheetImage> SSIvec;
    unsigned int SSnum, SetNum[4], FrIdx[4];// to start
    unsigned int FrDelay = 6, delayCnt = 0;// to start
    sf::Image buffImage;
    ImageSourceBuffer ISB;
    sf::Image destImage;

    // for the graphics here
    sf::Texture destTexture;
    sf::Vertex destTxtRect[4];// assign texCoords to map entire texture. Note: position members determine scaling
    float Scale = 1.0f;

    // another target texture
    sf::Texture TxtA;
    sf::Vertex TxtRectA[4];// assign texCoords to map entire texture. Note: position members determine scaling
    float ScaleA = 1.0f;
    bool initTxtA( const char* fileName );
    std::vector<sf::Color> colorVec;
    void findImageColors( const spriteSheetImage& SSI, size_t SetNum, size_t FrIdx0, size_t FrIdxF );

    virtual bool init();
    virtual bool handleEvent( sf::Event& rEvent );
    virtual void update( float dt );
    virtual void draw( sf::RenderTarget& RT ) const;

    // special write bits to file
    void SSItoFile( size_t SSnum, size_t SetNum, size_t FrIdx0, size_t FrIdxF )const;

    // create sf::Image and the sprite sheet to go with it
    bool makeImageAndSSdata( const char* fileName )const;

    lvl_ImageMaps(){}
    virtual ~lvl_ImageMaps(){}

    protected:

    private:
};

#endif // LVL_IMAGEMAPS_H
