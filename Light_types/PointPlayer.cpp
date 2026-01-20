#include "PointPlayer.h"

void PointPlayer::assignPath( const char* fileName )
{

}

void PointPlayer::setup( uint8_t& pathX0, uint8_t& pathY0, uint8_t NumPoints, float Speed, Light PtColor )
{
    pathX = &pathX0;
    pathY = &pathY0;
    numPoints = NumPoints;
    speed = Speed;
    ptColor = PtColor;

    Start();
}

void PointPlayer::Start()
{
    currPoint = 0;
    nextPoint = 1;
    pos0.x = pos.x = pathX[0];
    pos0.y = pos.y = pathY[0];
    vec2f posf( pathX[1], pathY[1] );
    uCurr = posf - pos0;   
    currLength = uCurr.mag();
    uCurr /= currLength;
    pos = pos0 + ( 1.0f + fadeLength )*uCurr;// start ahead
    uPrev = uCurr;    
    vel = ( posf - pos0 )*( speed/currLength );
    isPlaying = true;
}

bool PointPlayer::update( float dt )// true if animating
{
    if( !isPlaying ) return false;
    // update position
    pos += vel*dt;
    if( ( pos - pos0 ).mag() >= currLength )// change leg
    {
        currPoint = nextPoint;
        nextPoint = ( 1 + currPoint )%numPoints;
        uPrev = uCurr;
        pos0.x = pos.x = pathX[currPoint];
        pos0.y = pos.y = pathY[currPoint];
        vec2f posf( pathX[nextPoint], pathY[nextPoint] );
        uCurr = posf - pos0;   
        currLength = uCurr.mag();
        uCurr /= currLength;
        vel = ( posf - pos0 )*( speed/currLength );
    }

    return true;
}

void PointPlayer::draw()const
{
    if( !pLt0 ) return;
    if( !isPlaying ) return;

    int r = pos.y, c = pos.x;
    // check bounds?
    if( r >= 0 && r < gridRows && c >= 0 && c < gridCols )
        pLt0[ r*gridCols + c ] = ptColor;

    // the trail
    vec2f trailIter = pos, iterDir = uCurr;
    vec2f prevPos = pos0;// easy init
    uint8_t currPointIter = currPoint;// easy init
    // calculate uPrev per leg back. It is the new iterDir
    float trailDist = 0.0f;// go to fadeLength
  
    while( trailDist < fadeLength )
    {
        trailDist += 0.5f;
        trailIter -= 0.5f*iterDir;
        // may back around multiple corners
        if( iterDir.dot( trailIter - prevPos ) < 0.0f )// prevPos slightly over shot by trailIter
        {
            trailIter = prevPos;// at the corner
            // retard currPointIter = point moving back towards
            if( currPointIter > 0 ) --currPointIter;
            else currPointIter = numPoints - 1;
            // new prevPos
            prevPos.x = pathX[ currPointIter ];
            prevPos.y = pathY[ currPointIter ];

            iterDir = trailIter - prevPos;
            iterDir /= iterDir.mag();
        }

        int r = trailIter.y, c = trailIter.x;
        // check bounds?
        if( r >= 0 && r < gridRows && c >= 0 && c < gridCols )
        {
            
            float U = trailDist/fadeLength;
            float W = 1.0f - U;
            if( W <= 0.0f ) break;// fully faded
            int n = r*gridCols + c;
            float fr =  W*ptColor.r + U*pLt0[n].r;
            float fg =  W*ptColor.g + U*pLt0[n].g;
            float fb =  W*ptColor.b + U*pLt0[n].b;
            Light blendLt( fr, fg, fb );
            pLt0[n] = blendLt;
        }            
    }
}

/*
void PointPlayer::draw()const
{
    if( !pLt0 ) return;
    if( !isPlaying ) return;

    int r = pos.y, c = pos.x;
    // check bounds?
    if( r >= 0 && r < gridRows && c >= 0 && c < gridCols )
        pLt0[ r*gridCols + c ] = ptColor;

    // the trail
    vec2f trailIter = pos, iterDir = uCurr;
    float trailDist = 0.0f;// go to fadeLength
    bool firstLeg = true;
    while( trailDist < fadeLength )
    {
        trailDist += 0.5f;
        trailIter -= 0.5f*iterDir;
        if( firstLeg && uCurr.dot( trailIter - pos0 ) < 0.0f )// last point slightly over shot
        {
            firstLeg = false;
            trailIter = pos0;
            iterDir = uPrev;
        }

        int r = trailIter.y, c = trailIter.x;
        // check bounds?
        if( r >= 0 && r < gridRows && c >= 0 && c < gridCols )
        {
            
            float U = trailDist/fadeLength;
            float W = 1.0f - U;
            if( W <= 0.0f ) break;// fully faded
            int n = r*gridCols + c;
            float fr =  W*ptColor.r + U*pLt0[n].r;
            float fg =  W*ptColor.g + U*pLt0[n].g;
            float fb =  W*ptColor.b + U*pLt0[n].b;
            Light blendLt( fr, fg, fb );
            pLt0[n] = blendLt;
        }            
    }
}
*/