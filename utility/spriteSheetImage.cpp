#include "spriteSheetImage.h"

spriteSheetImage::spriteSheetImage()
{
    //ctor
}

spriteSheetImage::~spriteSheetImage()
{
    //dtor
}

void spriteSheetImage::init( std::istream& is )
{
    std::string imgFname;
    getline( is, imgFname );

    Img.loadFromFile( imgFname.c_str());
    size_t numSets;
    is >> numSets;
 //   std::cout << "\nnumSets read: " << numSets;
    frSetVec.reserve(numSets);

    size_t numFrames = 0;// read in a frame set
    sf::IntRect temp;
//    for( size_t i = 0; i < numSets && inFile >> numFrames; ++i )
    for( size_t i = 0; i < numSets; ++i )
    {
        is >> numFrames;
 //       std::cout << "\nnumFrames read: " << numFrames;
        std::vector<sf::IntRect> setVec;
        setVec.reserve(numFrames);
        for( size_t j = 0; j < numFrames; ++j )
        {
            is >> temp.left >> temp.top >> temp.width >> temp.height;
     //       std::cout << "\nLt: " << Lt << " Tp: " << Tp << " W: " << W << " H: " << H;
            setVec.push_back(temp);
        }

        frSetVec.push_back( setVec );
  //      std::cout << "\nfrSetVec.size(): " << frSetVec.size();
    }

    is >> isTransparent;
    if( isTransparent )
    {
        unsigned int rd, gn, bu;
        is >> rd >> gn >> bu;
        Xcolor.r = rd;
        Xcolor.g = gn;
        Xcolor.b = bu;
        Xcolor.a = 0;
        Img.createMaskFromColor( sf::Color(rd,gn,bu) );
    }
}

sf::IntRect spriteSheetImage::getFrRect( size_t& frIdx, size_t setNum, bool frAdvance ) const
{
    if( setNum >= frSetVec.size() ) setNum = frSetVec.size() - 1;
    if( frAdvance && ++frIdx >= frSetVec[setNum].size() ) frIdx = 0;
    return frSetVec[setNum][frIdx];
}

// static
bool spriteSheetImage::loadSpriteSheets( std::vector<spriteSheetImage>& SSvec, const char* inFileName )
{
    std::ifstream finList( inFileName );
    if( !finList ) { std::cout << "\n spriteSheetImage::loadSpriteSheets(): no ssFileList"; return false; }

    std::string rootName;// directory path from file
    finList >> rootName;
    int numSS = 0; finList >> numSS;
    if( numSS < 1 ) return false;
    SSvec.clear();
    SSvec.reserve( numSS );

    std::string fileName;
    while( numSS-- > 0 && finList >> fileName )
    {
        fileName = rootName + fileName;
        std::ifstream f_in( fileName.c_str() );
        if( !f_in ) { std::cout << "\nno " << fileName; return false; }
        SSvec.push_back( spriteSheetImage( f_in ) );
        f_in.close();
    }

    finList.close();
    std::cout << "\n#SS = " << SSvec.size();
    return true;
}
