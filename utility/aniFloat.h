#ifndef ANIFLOAT_H
#define ANIFLOAT_H


class aniFloat
{
    public:
    float dist = 1.0f, speed = 1.0f;
    float tElap = 1.0f, period = 1.0f;
    // derived type aniFloatWrite writes to float value

    void setup( float Dist, float Speed, bool Start = false )
    {
        dist = Dist;
        speed = Speed;
        period = dist/Speed;

        if( period < 0.0f ) period *= -1.0f;
        if( Start ) tElap = 0.0f;
        else tElap = period;
    }

    void setSpeed( float Speed, bool Start = false )
    {
        speed = Speed;
        period = dist/Speed;
        if( period < 0.0f ) period *= -1.0f;
        if( Start ) tElap = 0.0f;
        else tElap = period;
    }

    void setDist( float Dist, bool Start = false )
    {
        dist = Dist;
        period = dist/speed;
        if( period < 0.0f ) period *= -1.0f;
        if( Start ) tElap = 0.0f;
        else tElap = period;
    }

    bool updateTime( float dt )
    {
        if( tElap < period )
        {
            tElap += dt;
            return true;// animating
        }
        return false;// not animating
    }

    float getSpeed()const
    {
        if( tElap < period )
        {
            float U = tElap/period;
            U = 6.0f*U*( 1.0f - U );// moderates speed = d( dist )/dU
            return U*speed;
        }
        return 0.0f;
    }

    float currDist()const
    {
        if( tElap < period )
        {
            float U = tElap/period;
            U = U*U*( 3.0f - 2.0f*U );//
            return U*dist;
        }
        return 0.0f;
    }

    bool Running()const { return tElap < period; }
    void start(){ tElap = 0.0f; }

    aniFloat(){}

    ~aniFloat(){}

    protected:

    private:
};

#endif // ANIFLOAT_H
