#ifndef MYLIGHTMETHODSSS_H_INCLUDED
#define MYLIGHTMETHODSSS_H_INCLUDED

#include "myLight.h"
#include "spriteSheet.h"
#include "spriteSheetImage.h"

// lightGroup is entire grid
// on sf::Image no texture
void mapImageToGroup( lightGroup& LtGrp, const spriteSheetImage& SSI, unsigned int SetNum, unsigned int FrIdx );

void mapImageToGroup( lightGroup& LtGrp, const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx );
void mapImageToGroup( lightGroup& LtGrp, const spriteSheet& SS,
            unsigned int SetNum, unsigned int FrIdx, const sf::Color& Xcolor, const myLight& SubLt );
// map to columns C0 to C0 + LtGrp.cols
void mapImageToColumn( lightGroup& LtGrp, const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx, unsigned int C0 );

// with grid data given
void mapImageToGroup( lightGroup& LtGrp, const myLight* pGrid0, unsigned int gridRows, unsigned int gridCols,
        const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx );
// lightGroupGrid is over rectangle in grid
void mapImageToGroup( lightGroupGrid& LtGG, const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx );// calls above
// groupGridRotate is over rectangle in grid
void mapImageToGroup( groupGridRotate& GGR, const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx );


//** overload which draws given color in place of transparent pixel
// with grid data given
void mapImageToGroup( lightGroup& LtGrp, const myLight* pGrid0, unsigned int gridRows, unsigned int gridCols,
    const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx, const sf::Color& Xcolor, const myLight& SubLt );
// lightGroupGrid is over rectangle in grid
void mapImageToGroup( lightGroupGrid& LtGG, const spriteSheet& SS,
        unsigned int SetNum, unsigned int FrIdx, const sf::Color& Xcolor, const myLight& SubLt );// calls above

// overload with transparency color given. Must substitute from saved swatch
void mapImageToGroup( groupGridMove& LtGGM, const spriteSheet& SS,
        unsigned int SetNum, unsigned int FrIdx, const sf::Color& Xcolor );

void blendImageToGroup( lightGroup& LtGrp, const spriteSheet& SS,
        unsigned int SetNumA, unsigned int FrIdxA, unsigned int SetNumB, unsigned int FrIdxB, float u );
void blendImageToGroup( lightGroupGrid& LtGG, const spriteSheet& SS,
        unsigned int SetNumA, unsigned int FrIdxA, unsigned int SetNumB, unsigned int FrIdxB, float u );

// mvDir = 'U' or 'D' not working. wrong frame presents
void shiftImageToGroup( lightGroupGrid& LtGG, const spriteSheet& SS,
        unsigned int SetNumA, unsigned int FrIdxA, unsigned int SetNumB, unsigned int FrIdxB, float u, char mvDir );

#endif // MYLIGHTMETHODSSS_H_INCLUDED
