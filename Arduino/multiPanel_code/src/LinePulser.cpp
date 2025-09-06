#include "LinePulser.h"

bool LinePulser::update( float dt )// true if animating
{
    if( !isPlaying ) return false;

    if( onePulse ) updatePulse(dt);
    else updateWave(dt);

    return isPlaying;
}

// for each process
void LinePulser::updatePulse( float dt )
{
    bool LtAssigned = false;// pattern ends when no Light is assigned
    tElap += dt;

    float Dist = speed*tElap;
    vec2f Qa = Pa + Dist*Sa*Ta, Qb = Pb + Dist*Sb*Tb;
    float L = ( Qb - Qa ).mag();// length of pulse

    // temp: check for sign of life
  //  pLt0[ gridCols*(int)Pa.y + (int)Pa.x ] = Light(255,255,255);
  //  pLt0[ gridCols*(int)Pb.y + (int)Pb.x ] = Light(255,155,255);
  //  return;

    for( float x = 0.0f; x < L; x += 1.0f )// along length of pulse
    {
        vec2f R = Qa + x*Tp;// point on pulse center
        for( float w = -0.5f*pulseWidth; w <= 0.5f*pulseWidth; w += 1.0f )// bottom to top
        {
            vec2f Z = R + w*Tw;
            // check if on grid. x = col, y = row
            if( (int)Z.x < 0 ) continue;// left of grid
            if( (int)Z.x >= gridCols ) continue;// right of grid

            if( (int)Z.y < 0 ) continue;// above grid
            if( (int)Z.y >= gridRows ) continue;// below grid
        
            // will write
            int r = (int)Z.y, c = (int)Z.x;

            float fadeU = 1.0f;// no fade        
            if( Dist > rDist )
            {
                fadeU = static_cast<float>( rDist + fadeDist - Dist )/static_cast<float>( rDist + fadeDist );
                if( fadeU < 0.01f ) continue;// last frame over step
            }

            float U = 1.0f - 2.0f*w/pulseWidth;
            if( w < 0.0f ) U = 1.0f + 2.0f*w/pulseWidth;
            U *= Amp*fadeU*U;
            float fadeIn = 1.0f - U;
            Light& currLt = pLt0[ r*gridCols + c ];
            // interpolate
            float fr = U*hiLt.r + fadeIn*currLt.r;
            float fg = U*hiLt.g + fadeIn*currLt.g;
            float fb = U*hiLt.b + fadeIn*currLt.b;

            currLt = Light( fr, fg, fb );
            LtAssigned = true;
        }// across pulseWidth
    }// along pulse from Pa to Pb

    if( LtAssigned && !isVisible ) 
        isVisible = true;// has reached the grid

    if( isVisible && ( !LtAssigned || ( Dist >= rDist + fadeDist ) ) )
        isPlaying = false;// animation complete
}

void LinePulser::updateWave( float dt )
{
    bool LtAssigned = false;// pattern ends when no Light is assigned
    tElap += dt;   

    float Dist = speed*tElap;
    if( Dist > rDist + fadeDist ) Dist = rDist + fadeDist;// limit value
    
    float rotFreq = 3.1416f*speed/pulseWidth;// wavelength = 2 x ringWidth
    float K = 3.1416f/pulseWidth;
    float wMin = 0.0f;
     if( !isRadiating )
     {
        stopTime += dt;
        wMin = speed*stopTime;
        if( wMin > Dist ) { isPlaying = false; return; }
     }

    for( float w = wMin; w <= Dist; w += 1.0f )// bottom to top
    {
        vec2f Qa = Pa + w*Sa*Ta, Qb = Pb + w*Sb*Tb;
        float L = ( Qb - Qa ).mag();// length of pulse

        for( float x = 0.0f; x < L; x += 1.0f )// along length of pulse
        {
            vec2f Z = Qa + x*Tp;// point on pulse center            
           
            // check if on grid. x = col, y = row
            if( (int)Z.x < 0 ) continue;// left of grid
            if( (int)Z.x >= gridCols ) continue;// right of grid

            if( (int)Z.y < 0 ) continue;// above grid
            if( (int)Z.y >= gridRows ) continue;// below grid        

            float fadeU = 1.0f;// no fade        
            if( Dist > rDist )
            {
                fadeU = static_cast<float>( rDist + fadeDist - Dist )/static_cast<float>( rDist + fadeDist );
                if( fadeU < 0.01f ) continue;// last frame over step
            }

            float U = -Amp*sinf( K*w - direction*rotFreq*tElap );// traveling wave     
            U *= fadeU;// apply fade
            float fadeIn = ( U > 0.0f ) ? 1.0f - U : 1.0f + U;
            int r = (int)Z.y, c = (int)Z.x;
            Light& currLt = pLt0[ r*gridCols + c ];
            // interpolate
            float fr = fadeIn*currLt.r;
            float fg = fadeIn*currLt.g;
            float fb = fadeIn*currLt.b;
            if( U > 0.0f )
            {
                fr += U*hiLt.r;
                fg += U*hiLt.g;
                fb += U*hiLt.b;
            }
            else
            {
                fr -= U*loLt.r;// - because U < 0
                fg -= U*loLt.g;
                fb -= U*loLt.b;
            }

            currLt = Light( fr, fg, fb );
            LtAssigned = true;            
        }// along pulse from Pa to Pb
    }// outward from wMin to Dist

    if( LtAssigned && !isVisible ) isVisible = true;

    if( isVisible && !LtAssigned )// animation complete
        isPlaying = false;
}