#include "myLightMethodsSS.h"

// lightGroup is entire grid
void mapImageToGroup( lightGroup& LtGrp, const spriteSheetImage& SSI, unsigned int SetNum, unsigned int FrIdx )
{
    if( SetNum >= SSI.numSets() ) return;
    if( FrIdx >= SSI.getFrCount( SetNum ) ) FrIdx = 0;
    sf::IntRect frRect = SSI.getFrRect( FrIdx, SetNum, false );
    float FrW = frRect.width, FrH = frRect.height;

    for( unsigned int R = 0; R < LtGrp.rows; ++R )
    {
        for( unsigned int C = 0; C < LtGrp.cols; ++C )
        {
        //    unsigned int idx = R*LtGrp.cols + C;
            unsigned int xImg = frRect.left + (FrW*C)/LtGrp.cols;
            unsigned int yImg = frRect.top + (FrH*R)/LtGrp.rows;
            sf::Color imgColor = SSI.Img.getPixel ( xImg, yImg );
        //    LtGrp.pLt0[idx].init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
            LtGrp.get_pLt(R,C)->init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

void mapImageToGroup( lightGroup& LtGrp, const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx )
{
    if( SetNum >= SS.numSets() ) return;
    if( FrIdx >= SS.getFrCount( SetNum ) ) FrIdx = 0;
    sf::IntRect frRect = SS.getFrRect( FrIdx, SetNum, false );
    float FrW = frRect.width, FrH = frRect.height;

    for( unsigned int R = 0; R < LtGrp.rows; ++R )
    {
        for( unsigned int C = 0; C < LtGrp.cols; ++C )
        {
            unsigned int idx = R*LtGrp.cols + C;
            unsigned int xImg = frRect.left + (FrW*C)/LtGrp.cols;
            unsigned int yImg = frRect.top + (FrH*R)/LtGrp.rows;
            sf::Color imgColor = SS.img.getPixel ( xImg, yImg );
            LtGrp.pLt0[idx].init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

void mapImageToGroup( lightGroup& LtGrp, const spriteSheet& SS,
            unsigned int SetNum, unsigned int FrIdx, const sf::Color& Xcolor, const myLight& SubLt )
{
    if( SetNum >= SS.numSets() ) return;
    if( FrIdx >= SS.getFrCount( SetNum ) ) FrIdx = 0;
    sf::IntRect frRect = SS.getFrRect( FrIdx, SetNum, false );
    float FrW = frRect.width, FrH = frRect.height;

    for( unsigned int R = 0; R < LtGrp.rows; ++R )
    {
        for( unsigned int C = 0; C < LtGrp.cols; ++C )
        {
            unsigned int idx = R*LtGrp.cols + C;
            unsigned int xImg = frRect.left + ( FrW*C )/LtGrp.cols;
            unsigned int yImg = frRect.top + ( FrH*R )/LtGrp.rows;
            sf::Color imgColor = SS.img.getPixel ( xImg, yImg );
            if( imgColor.r != Xcolor.r || imgColor.g != Xcolor.g || imgColor.b != Xcolor.b )
                LtGrp.pLt0[idx].init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
            else// draw SubLt
                 LtGrp.pLt0[idx] = SubLt;
        }
    }
}

// writes from column C0 only to LtGrp column 0
void mapImageToColumn( lightGroup& LtGrp, const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx, unsigned int C0 )
{
    if( SetNum >= SS.numSets() ) return;
    if( FrIdx >= SS.getFrCount( SetNum ) ) FrIdx = 0;
    sf::IntRect frRect = SS.getFrRect( FrIdx, SetNum, false );
    float FrW = frRect.width, FrH = frRect.height;
    if( C0 >= LtGrp.cols ) { std::cout << "\n LtGrp oops"; return; }
  //  if( C0 >= frRect.width ) { std::cout << "\n C0 oops"; return; }

    for( unsigned int R = 0; R < LtGrp.rows; ++R )
    {
        unsigned int xImg = frRect.left + (FrW*C0)/LtGrp.cols;
        unsigned int yImg = frRect.top + (FrH*R)/LtGrp.rows;
        sf::Color imgColor = SS.img.getPixel ( xImg, yImg );
        LtGrp.get_pLt( R, 0 )->init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
    }
}

// called by methods below
void mapImageToGroup( lightGroup& LtGrp, const myLight* pGrid0, unsigned int gridRows, unsigned int gridCols,
        const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx )
{
    if( SetNum >= SS.numSets() ) return;
    if( FrIdx >= SS.getFrCount( SetNum ) ) FrIdx = 0;
    sf::IntRect frRect = SS.getFrRect( FrIdx, SetNum, false );
    float FrW = frRect.width, FrH = frRect.height;

    for( unsigned int R = 0; R < LtGrp.rows; ++R )
    {
        for( unsigned int C = 0; C < LtGrp.cols; ++C )
        {
            unsigned int idx = R*gridCols + C;
            unsigned int xImg = frRect.left + ( FrW*C )/LtGrp.cols;
            unsigned int yImg = frRect.top + ( FrH*R )/LtGrp.rows;
            sf::Color imgColor = SS.img.getPixel ( xImg, yImg );
            LtGrp.pLt0[idx].init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

void mapImageToGroup( lightGroupGrid& LtGG, const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx )
{
    mapImageToGroup( LtGG, LtGG.pGrid0, LtGG.gridRows, LtGG.gridCols, SS, SetNum, FrIdx );
}

void mapImageToGroup( groupGridRotate& GGR, const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx )
{
    if( SetNum >= SS.numSets() ) return;
    if( FrIdx >= SS.getFrCount( SetNum ) ) FrIdx = 0;
    sf::IntRect frRect = SS.getFrRect( FrIdx, SetNum, false );
    float FrW = frRect.width, FrH = frRect.height;
    unsigned int R=0, C=0, Rrot=0, Crot=0;

    for( unsigned int k = 0; k < GGR.rowVec.size(); ++k )
    {
        if( GGR.Rotate( k, R, C, Rrot, Crot ) )
        {

            unsigned int xImg = frRect.left + ( FrW*C )/GGR.cols;
            unsigned int yImg = frRect.top + ( FrH*R )/GGR.rows;
            sf::Color imgColor = SS.img.getPixel ( xImg, yImg );
            unsigned int idx = Rrot*GGR.gridCols + Crot;
            GGR.pLt0[idx].init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

// overload with transparency color
void mapImageToGroup( lightGroup& LtGrp, const myLight* pGrid0, unsigned int gridRows, unsigned int gridCols,
    const spriteSheet& SS, unsigned int SetNum, unsigned int FrIdx, const sf::Color& Xcolor, const myLight& SubLt )
{
    if( SetNum >= SS.numSets() ) return;
    if( FrIdx >= SS.getFrCount( SetNum ) ) FrIdx = 0;
    sf::IntRect frRect = SS.getFrRect( FrIdx, SetNum, false );
    float FrW = frRect.width, FrH = frRect.height;

    for( unsigned int R = 0; R < LtGrp.rows; ++R )
    {
        for( unsigned int C = 0; C < LtGrp.cols; ++C )
        {
            unsigned int idx = R*gridCols + C;
            unsigned int xImg = frRect.left + ( FrW*C )/LtGrp.cols;
            unsigned int yImg = frRect.top + ( FrH*R )/LtGrp.rows;
            sf::Color imgColor = SS.img.getPixel ( xImg, yImg );
            if( imgColor.r != Xcolor.r || imgColor.g != Xcolor.g || imgColor.b != Xcolor.b )
                LtGrp.pLt0[idx].init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
            else// draw SubLt
                 LtGrp.pLt0[idx] = SubLt;
        }
    }
}

void mapImageToGroup( lightGroupGrid& LtGG, const spriteSheet& SS,
        unsigned int SetNum, unsigned int FrIdx, const sf::Color& Xcolor, const myLight& SubLt )
{
    mapImageToGroup( LtGG, LtGG.pGrid0, LtGG.gridRows, LtGG.gridCols, SS, SetNum, FrIdx, Xcolor, SubLt );
}

// with transparent color
void mapImageToGroup( groupGridMove& LtGGM, const spriteSheet& SS,
        unsigned int SetNum, unsigned int FrIdx, const sf::Color& Xcolor )
{
    if( SetNum >= SS.numSets() ) return;
    if( FrIdx >= SS.getFrCount( SetNum ) ) FrIdx = 0;
    sf::IntRect frRect = SS.getFrRect( FrIdx, SetNum, false );
    float FrW = frRect.width, FrH = frRect.height;

    for( unsigned int R = 0; R < LtGGM.rows; ++R )
    {
        for( unsigned int C = 0; C < LtGGM.cols; ++C )
        {
            unsigned int idx = R*LtGGM.gridCols + C;
            unsigned int xImg = frRect.left + ( FrW*C )/LtGGM.cols;
            unsigned int yImg = frRect.top + ( FrH*R )/LtGGM.rows;
            sf::Color imgColor = SS.img.getPixel ( xImg, yImg );

            if( imgColor.r != Xcolor.r || imgColor.g != Xcolor.g || imgColor.b != Xcolor.b )
                LtGGM.pLt0[idx].init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
            else// draw from swatch
                 LtGGM.pLt0[idx] = LtGGM.swatchVec[ R*LtGGM.cols + C ];
        }
    }
}

// for entire  grid
void blendImageToGroup( lightGroup& LtGrp, const spriteSheet& SS,
        unsigned int SetNumA, unsigned int FrIdxA, unsigned int SetNumB, unsigned int FrIdxB, float u )
{
    if( SetNumA >= SS.numSets() || SetNumB >= SS.numSets() ) return;
    if( FrIdxA >= SS.getFrCount( SetNumA ) ) FrIdxA = 0;
    if( FrIdxB >= SS.getFrCount( SetNumB ) ) FrIdxB = 0;
    const sf::IntRect frRectA = SS.getFrRect( FrIdxA, SetNumA, false );
    const sf::IntRect frRectB = SS.getFrRect( FrIdxB, SetNumB, false );
    // frames must have same width and height
    if( ( frRectA.width != frRectB.width ) || ( frRectA.height != frRectB.height ) ) return;
    const float FrW = frRectA.width, FrH = frRectA.height;

    for( unsigned int R = 0; R < LtGrp.rows; ++R )
    {
        for( unsigned int C = 0; C < LtGrp.cols; ++C )
        {
            unsigned int wImg = (FrW*C)/LtGrp.cols;
            unsigned int xImg = frRectA.left + wImg;
            unsigned int hImg = (FrH*R)/LtGrp.rows;
            unsigned int yImg = frRectA.top + hImg;
            sf::Color imgColorA = SS.img.getPixel ( xImg, yImg );
            xImg = frRectB.left + wImg;
            yImg = frRectB.top + hImg;
            sf::Color imgColorB = SS.img.getPixel ( xImg, yImg );
            float v = 1.0f - u;
            unsigned int rd = imgColorA.r*v + imgColorB.r*u;
            unsigned int gn = imgColorA.g*v + imgColorB.g*u;
            unsigned int bu = imgColorA.b*v + imgColorB.b*u;
            unsigned int idx = R*LtGrp.cols + C;
            LtGrp.pLt0[idx].init( rd, gn, bu, 1.0f );
        }
    }
}

// for sub grid
void blendImageToGroup( lightGroupGrid& LtGG, const spriteSheet& SS,
        unsigned int SetNumA, unsigned int FrIdxA, unsigned int SetNumB, unsigned int FrIdxB, float u )
{
    if( SetNumA >= SS.numSets() || SetNumB >= SS.numSets() ) return;
    if( FrIdxA >= SS.getFrCount( SetNumA ) ) FrIdxA = 0;
    if( FrIdxB >= SS.getFrCount( SetNumB ) ) FrIdxB = 0;
    const sf::IntRect frRectA = SS.getFrRect( FrIdxA, SetNumA, false );
    const sf::IntRect frRectB = SS.getFrRect( FrIdxB, SetNumB, false );
    // frames must have same width and height
    if( ( frRectA.width != frRectB.width ) || ( frRectA.height != frRectB.height ) ) return;
    const float FrW = frRectA.width, FrH = frRectA.height;

    for( unsigned int R = 0; R < LtGG.rows; ++R )
    {
        for( unsigned int C = 0; C < LtGG.cols; ++C )
        {
            unsigned int wImg = (FrW*C)/LtGG.cols;
            unsigned int xImg = frRectA.left + wImg;
            unsigned int hImg = (FrH*R)/LtGG.rows;
            unsigned int yImg = frRectA.top + hImg;
            sf::Color imgColorA = SS.img.getPixel ( xImg, yImg );
            xImg = frRectB.left + wImg;
            yImg = frRectB.top + hImg;
            sf::Color imgColorB = SS.img.getPixel ( xImg, yImg );
            float v = 1.0f - u;
            unsigned int rd = imgColorA.r*v + imgColorB.r*u;
            unsigned int gn = imgColorA.g*v + imgColorB.g*u;
            unsigned int bu = imgColorA.b*v + imgColorB.b*u;
            unsigned int idx = R*LtGG.gridCols + C;
            LtGG.pLt0[idx].init( rd, gn, bu, 1.0f );
        }
    }
}

void shiftImageToGroup( lightGroupGrid& LtGG, const spriteSheet& SS,
        unsigned int SetNumA, unsigned int FrIdxA, unsigned int SetNumB, unsigned int FrIdxB, float u, char mvDir )
{
    if( SetNumA >= SS.numSets() || SetNumB >= SS.numSets() ) return;
    if( FrIdxA >= SS.getFrCount( SetNumA ) ) FrIdxA = 0;
    if( FrIdxB >= SS.getFrCount( SetNumB ) ) FrIdxB = 0;
    const sf::IntRect frRectA = SS.getFrRect( FrIdxA, SetNumA, false );
    const sf::IntRect frRectB = SS.getFrRect( FrIdxB, SetNumB, false );
    // frames must have same width and height
    if( ( frRectA.width != frRectB.width ) || ( frRectA.height != frRectB.height ) ) return;
    const float FrW = frRectA.width, FrH = frRectA.height;

    // new frame cols grow with u = enter from left ( 1-u = enter from right? )
 //   float v = 1.0f - u;

    unsigned int Csplit = u*LtGG.cols;// horizontal movement
    unsigned int Rsplit = 0;// vary instead for vertical movement

    sf::IntRect frRect1 = frRectA, frRect2 = frRectB;// assign to A or B
    if( mvDir == 'R' || mvDir == 'L' )
    {
        if( mvDir == 'L' )
        {
            u = 1.0f - u;
            frRect1 = frRectB;
            frRect2 = frRectA;
        }

        Csplit = u*LtGG.cols;// horizontal movement
        Rsplit = 0;// vary instead for vertical movement
    }
    else if( mvDir == 'U' || mvDir == 'D' )// not working wrong frames
    {
        if( mvDir == 'U' )
        {
            u = 1.0f - u;
        }

        Csplit = 0;// horizontal movement
        Rsplit = u*LtGG.rows;// vary instead for vertical movement
    }


    for( unsigned int R = 0; R < LtGG.rows; ++R )
    {
        unsigned int xImg=0, yImg=0, Crel=0;
        unsigned int Rrel = R < Rsplit ? LtGG.rows + R - Rsplit : R - Rsplit;

        for( unsigned int C = 0; C < LtGG.cols; ++C )
        {
            if( C < Csplit )
            {
                Crel = LtGG.cols + C - Csplit;
                xImg = frRect1.left + (FrW*Crel)/LtGG.cols;// was A
                yImg = frRect1.top + (FrH*Rrel)/LtGG.rows;

            }
            else
            {
                Crel = C - Csplit;
                xImg = frRect2.left + (FrW*Crel)/LtGG.cols;// was B
                yImg = frRect2.top + (FrH*Rrel)/LtGG.rows;
            }

            sf::Color imgColor = SS.img.getPixel ( xImg, yImg );
            LtGG.pLt0[ R*LtGG.gridCols + C ].init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

