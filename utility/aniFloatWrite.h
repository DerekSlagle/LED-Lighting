#ifndef ANIFLOATWRITE_H
#define ANIFLOATWRITE_H

#include "aniFloat.h"

class aniFloatWrite : public aniFloat
{
    public:
    // inherited members
    // float dist = 1.0f, speed = 1.0f;
    // float tElap = 1.0f, period = 1.0f;

    // added property: write to float value
    float* pY = nullptr;// may use without
    float Y0 = 0.0f, Yf = 1.0f;
    // use with a int* instead
    int* pInt = nullptr;// use 1 or another

    void setup( float* py, float y0, float yf, float Speed, bool Start = false )
    {
	    pY = py;
        pInt = nullptr;
        Y0 = y0;
        Yf = yf;
        dist = Yf - Y0;
        speed = Speed;
        period = dist/Speed;

        if( period < 0.0f ) period *= -1.0f;
        if( Start ) start();
        else tElap = period;
    }

    void setup( int* p_Int, int y0, int yf, float Speed, bool Start = false )
    {
	    pY = nullptr;
        pInt = p_Int;
        Y0 = (float)y0;
        Yf = (float)yf;

        dist = Yf - Y0;
        speed = Speed;
        period = dist/Speed;

        if( period < 0.0f ) period *= -1.0f;
        if( Start ) start();
        else tElap = period;
    }

    void setSpeed( float Speed, bool Start = false )
    {
        speed = Speed;
        period = dist/Speed;
        if( period < 0.0f ) period *= -1.0f;
        if( Start ) start();
        else tElap = period;
    }

    void setDist( float Dist, bool Start = false )
    {
        dist = Dist;
        period = dist/speed;
        if( period < 0.0f ) period *= -1.0f;
        if( Start ) start();        
        else tElap = period;
    }

    // does not assign position. May use with pY = nullptr
    float currPosition()const { return Y0 + currDist(); }

    bool Running()const { return tElap < period; }

    void start()
    { 
        tElap = 0.0f;
        if( pY ) *pY = Y0;
        else if( pInt ) *pInt = (int)Y0;
    }

    bool update( float dt )
    {
        if( !( pY || pInt ) ) return false;
        if( updateTime(dt) )
        {
            if( pY ) *pY = Y0 + currDist();
            else if( pInt ) *pInt = (int)( Y0 + currDist() );
            
            if( !Running() )// last frame
            {
                if( pY ) *pY = Yf;
                else if( pInt ) *pInt = (int)Yf;
            }
            return true;
        }

        return false;
    }

    aniFloatWrite(){}

    ~aniFloatWrite(){}

    protected:

    private:
};

#endif // ANIFLOATWRITE_H
