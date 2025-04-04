#include "lightGroupSSI.h"

lightGroupSSI::lightGroupSSI()
{
    //ctor
}

lightGroupSSI::~lightGroupSSI()
{
    //dtor
}

void lightGroupSSI::init( const spriteSheetImage& rSSI, unsigned int SetNum, myLight* pGrid, myLight* p_Lt0,
        unsigned int GridRows, unsigned int GridCols, unsigned int FrDelay, float Scale )
{
    pSSI = &rSSI;
    setNum = SetNum;
    frDelay = FrDelay;
    scale = Scale;
    frIdx = 0;
    delayCnt = 0;
    doAnimate = false;// play pause?
    srcRect = pSSI->getFrRect( 0, setNum );
    lightGroupGrid::init( p_Lt0, scale*srcRect.height, scale*srcRect.width, pGrid, GridRows, GridCols );

    std::cout << "\n rows = " << rows << " cols = " << cols;

    unsigned int row0 = getRow0(), col0 = getCol0();
    if( col0 + cols + gridCols*( row0 + rows ) > gridRows*gridCols )
        std::cout << "\n grid size exceeded. Crash imminent!";
}

void lightGroupSSI::update( float dt )// update frIdx, srcRect, delayCnt, tElap?
{
    if( !doAnimate ) return;// srcRect is up to date

    if( frDelay == 1 )
    {
        srcRect = pSSI->getFrRect( frIdx, setNum, true );
        return;
    }

    if( ++delayCnt >= frDelay )
    {
        delayCnt = 0;
        srcRect = pSSI->getFrRect( frIdx, setNum, true );
    }
}

void lightGroupSSI::update()// update frIdx, srcRect, delayCnt only
{
    if( !doAnimate ) return;// srcRect is up to date

    if( frDelay == 1 )
    {
        srcRect = pSSI->getFrRect( frIdx, setNum, true );
        return;
    }

    if( ++delayCnt >= frDelay )
    {
        delayCnt = 0;
        srcRect = pSSI->getFrRect( frIdx, setNum, true );
    }
}

void lightGroupSSI::draw()const// general: branches to below
{
    if( scale == 1.0f )
    {
    //    return;

        if( pSSI->isTransparent )
            return drawTrueTransparent();
        return drawTrue();
    }
    else if( pSSI->isTransparent )
        return drawTransparent();

    // fall through is not transparent && scale != 1
    sf::Color imgColor;

    const sf::Image& rImg = pSSI->getImage();

    for( unsigned int c = 0; c < cols; ++c )
    {
        for( unsigned int r = 0; r < rows; ++r )
        {
            imgColor = rImg.getPixel ( srcRect.left + c/scale, srcRect.top + r/scale );
            get_pLt(r,c)->init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

// private methods
// called by draw() when it is time
void lightGroupSSI::drawTransparent()const
{
    const sf::Image& rImg = pSSI->getImage();
    sf::Color imgColor;

    for( unsigned int c = 0; c < cols; ++c )
    {
        for( unsigned int r = 0; r < rows; ++r )
        {
            imgColor = rImg.getPixel ( srcRect.left + c/scale, srcRect.top + r/scale );
            if( imgColor.a > 0 )
                get_pLt(r,c)->init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

void lightGroupSSI::drawTrue()const// scale = 1
{
    if( !pGrid0 )
    {
        std::cout << "\n NULL pGrid0";
        return;
    }

    if( !pLt0 )
    {
        std::cout << "\n NULL pLt0";
        return;
    }

    const sf::Image& rImg = pSSI->getImage();
    sf::Color imgColor;
    for( unsigned int c = 0; c < cols; ++c )
    {
        for( unsigned int r = 0; r < rows; ++r )
        {
            imgColor = rImg.getPixel ( srcRect.left + c, srcRect.top + r );
         //   if( !get_pLt(r,c) ) std::cout << "\n NULL get_pLt(r,c)";
         //   else get_pLt(0,0)->init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
            get_pLt(r,c)->init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

void lightGroupSSI::drawTrueTransparent()const// scale = 1
{
 //   std::cout << "\n drawTrueTransparent()";
 //   return;

    const sf::Image& rImg = pSSI->getImage();
    sf::Color imgColor;
    for( unsigned int c = 0; c < cols; ++c )
    {
        for( unsigned int r = 0; r < rows; ++r )
        {
            imgColor = rImg.getPixel ( srcRect.left + c/scale, srcRect.top + r/scale );
            if( imgColor.a > 0 )
                get_pLt(r,c)->init( imgColor.r, imgColor.g, imgColor.b, 1.0f );
        }
    }
}

void lightGroupSSI::nextFrame( bool rndRobin )// assigns srcRect
{
    unsigned frCnt = pSSI->getFrCount(setNum);
    if( frCnt == 1 ) return;

    if( rndRobin )
        srcRect = pSSI->getFrRect( frIdx, setNum, true );
    else if( frIdx < frCnt - 1 )
        srcRect = pSSI->getFrRect( ++frIdx, setNum );
}

void lightGroupSSI::prevFrame( bool rndRobin )
{
    unsigned frCnt = pSSI->getFrCount(setNum);
    if( frCnt == 1 ) return;

    if( frIdx > 0 )
        srcRect = pSSI->getFrRect( --frIdx, setNum );
    else if( rndRobin )
    {
        frIdx = 0;
        srcRect = pSSI->getFrRect( 0, setNum );
    }
}

void lightGroupSSI::nextFrameSet( bool rndRobin )// assigns srcRect
{
    const unsigned numSets = pSSI->numSets();
    if( numSets == 1 ) return;

    frIdx = 0;

    if( setNum < numSets - 1 )
        srcRect = pSSI->getFrRect( 0, ++setNum );
    else if( rndRobin )
    {
        setNum = 0;
        srcRect = pSSI->getFrRect( 0, 0 );
    }
}

void lightGroupSSI::prevFrameSet( bool rndRobin )
{
    const unsigned numSets = pSSI->numSets();
    if( numSets == 1 ) return;

    frIdx = 0;

    if( setNum > 0 )
        srcRect = pSSI->getFrRect( 0, --setNum );
    else if( rndRobin )
    {
        setNum = 0;
        srcRect = pSSI->getFrRect( 0, 0 );
    }
}
