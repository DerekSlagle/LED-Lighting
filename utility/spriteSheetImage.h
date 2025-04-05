#ifndef SPRITESHEETIMAGE_H
#define SPRITESHEETIMAGE_H

#include <SFML/Graphics.hpp>
#include<vector>
#include<iostream>
#include<fstream>

class spriteSheetImage
{
    public:
    sf::Image Img;
    const sf::Image& getImage()const { return Img; }
    bool isTransparent = false;
    sf::Color Xcolor;// if above

    sf::IntRect getFrRect( size_t& frIdx, size_t setNum, bool frAdvance ) const;
    sf::IntRect getFrRect( size_t frIdx, size_t setNum ) const { return frSetVec[setNum][frIdx]; }
    size_t numSets() const { return frSetVec.size(); }
    size_t getFrCount( size_t setNum )const { return frSetVec[setNum].size(); }

    void init( std::istream& is );
    spriteSheetImage( std::istream& is ){ init(is); }

    static bool loadSpriteSheets( std::vector<spriteSheetImage>& SSvec, const char* inFileName );

    spriteSheetImage();
    virtual ~spriteSheetImage();

    protected:
    std::vector< std::vector<sf::IntRect> > frSetVec;
    private:
};

#endif // SPRITESHEETIMAGE_H
