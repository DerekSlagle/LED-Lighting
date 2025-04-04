#include "lightVisual.h"

void lightVisual::init( const myLight* p_Light, float posX, float posY, sf::Vector2f Sz )
{
    pLight = p_Light;
    pGroup = nullptr;
    vtxVec.resize(4);
    // draw as a quad
    vtxVec[0].position = sf::Vector2f( posX, posY );// cw from top left
    vtxVec[1].position = sf::Vector2f( posX + Sz.x, posY );// top right
    vtxVec[2].position = sf::Vector2f( posX + Sz.x, posY + Sz.y );// bottom right
    vtxVec[3].position = sf::Vector2f( posX, posY + Sz.y );// bottom left
    update();
     //   theLight.setFillColor( sf::Color( pLight->rd, pLight->gn, pLight->bu ) );
}

void lightVisual::init( lightGroup& r_Group, float posX, float posY, float dPosX, float dPosY, sf::Vector2f LtSz )
{
    pLight = nullptr;
    pGroup = &r_Group;
    vtxVec.resize( 4*r_Group.rows*r_Group.cols );
    numLts = r_Group.rows*r_Group.cols;

    for( unsigned int n = 0; n < numLts; ++n )
    {

        vtxVec[4*n].position.x = posX + n%pGroup->cols*( LtSz.x + dPosX );// up lt
        vtxVec[4*n].position.y = posY + n/pGroup->cols*( LtSz.y + dPosY );

        vtxVec[4*n+1].position.x = vtxVec[4*n].position.x + LtSz.x;// up rt
        vtxVec[4*n+1].position.y = vtxVec[4*n].position.y;

        vtxVec[4*n+2].position.x = vtxVec[4*n+1].position.x;// dn rt
        vtxVec[4*n+2].position.y = vtxVec[4*n].position.y + LtSz.y;

        vtxVec[4*n+3].position.x = vtxVec[4*n].position.x;// dn lt
        vtxVec[4*n+3].position.y = vtxVec[4*n+2].position.y;
    }
}

// Circular layout. rows = 1 only
void lightVisual::init( lightGroup& r_Group, float ctrPosX, float ctrPosY, float Radius, sf::Vector2f LtSz )
{

    std::cout << "\n LV.init(R)";
 //   init( lightGroup& r_Group, ctrPosX, ctrPosY, 4.0f, 0.0f, LtSz );
 //   return;

    if( r_Group.rows > 1 )// make it 1 row
    {
        r_Group.cols *= r_Group.rows;
        r_Group.rows = 1;
        std::cout << "\n rows != 1";
    }
    else if( r_Group.rows == 0 )
        std::cout << "\n rows == 0";

    pLight = nullptr;
    pGroup = &r_Group;
    vtxVec.resize( 4*r_Group.cols );
    numLts = r_Group.cols;

    float dAng = 6.2832f/r_Group.cols;
    const sf::Vector2f hfSz( LtSz.x, LtSz.y );
    for( unsigned int n = 0; n < r_Group.cols; ++n )
    {
        vtxVec[4*n].position.x = ctrPosX + Radius*cosf( n*dAng ) - hfSz.x;// up lt
        vtxVec[4*n].position.y = ctrPosY - Radius*sinf( n*dAng ) - hfSz.y;
        vtxVec[4*n+1].position.x = vtxVec[4*n].position.x + LtSz.x;// up rt
        vtxVec[4*n+1].position.y = vtxVec[4*n].position.y;
        vtxVec[4*n+2].position.x = vtxVec[4*n+1].position.x;// dn rt
        vtxVec[4*n+2].position.y = vtxVec[4*n].position.y + LtSz.y;
        vtxVec[4*n+3].position.x = vtxVec[4*n].position.x;// dn lt
        vtxVec[4*n+3].position.y = vtxVec[4*n+2].position.y;
        vtxVec[4*n+3].color = sf::Color::Blue;
        vtxVec[4*n+2].color = vtxVec[4*n+1].color = vtxVec[4*n].color = sf::Color::Magenta;
    }
}

sf::FloatRect lightVisual::getRect()const
{
    sf::FloatRect retVal;

    if( pLight )
    {
        retVal.left = vtxVec[0].position.x;
        retVal.top = vtxVec[0].position.y;
        retVal.width = vtxVec[1].position.x - vtxVec[0].position.x;
        retVal.height = vtxVec[1].position.y - vtxVec[2].position.y;
    }
    else if( pGroup )
    {
        retVal.left = vtxVec[0].position.x;
        retVal.top = vtxVec[0].position.y;
        const sf::Vertex& backVtx = vtxVec[ vtxVec.size() - 2 ];// dn rt vtx in last quad
        retVal.width = backVtx.position.x - retVal.left;
        retVal.height = backVtx.position.y - retVal.top;
    }

    return retVal;
}

sf::Vector2f lightVisual::getLtSize()const
{
    float w = vtxVec[1].position.x - vtxVec[0].position.x;
    float h = vtxVec[3].position.y - vtxVec[0].position.y;
    return sf::Vector2f( w, h );
}

bool lightVisual::isOver( float x, float y )const
{
    sf::FloatRect rect( getRect() );
    if( x < rect.left ) return false;
    if( x > rect.left + rect.width ) return false;
    if( y < rect.top ) return false;
    if( y > rect.top + rect.height ) return false;
    return true;
}

void lightVisual::update()
{
 //   unsigned int rd = 0, gn = 0, bu = 0;
    sf::Color LtClr;

    if( pGroup )
    {
     //   return;

        myLight* pLt = pGroup->pLt0;
        for( unsigned int n = 0; n < numLts; ++n )
        {
            LtClr.r = static_cast<unsigned int>( pLt->brightness*static_cast<float>( pLt->rd ) );
            LtClr.g = static_cast<unsigned int>( pLt->brightness*static_cast<float>( pLt->gn ) );
            LtClr.b = static_cast<unsigned int>( pLt->brightness*static_cast<float>( pLt->bu ) );
            vtxVec[4*n+3].color = vtxVec[4*n+2].color = vtxVec[4*n+1].color = vtxVec[4*n].color = LtClr;
            ++pLt;
        }
    }
    else if( pLight )
    {
        LtClr.r = static_cast<unsigned int>( pLight->brightness*static_cast<float>( pLight->rd ) );
        LtClr.g = static_cast<unsigned int>( pLight->brightness*static_cast<float>( pLight->gn ) );
        LtClr.b = static_cast<unsigned int>( pLight->brightness*static_cast<float>( pLight->bu ) );
        vtxVec[3].color = vtxVec[2].color = vtxVec[1].color = vtxVec[0].color = LtClr;
    //    rd *= pLight->brightness;
    //    gn *= pLight->brightness;
    //    bu *= pLight->brightness;
    //    theLight.setFillColor( sf::Color( rd, gn, bu ) );
    }
}

void lightVisual::draw( sf::RenderTarget& RT ) const
{
    if( pLight )
        RT.draw( &( vtxVec[0] ), 4, sf::Quads );
    else if( pGroup )
        RT.draw( &( vtxVec[0] ), vtxVec.size(), sf::Quads );
}
