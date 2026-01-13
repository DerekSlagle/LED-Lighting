#include "persPt.h"

float persPt::X0 = 1000.0f, persPt::Yh = 300.0f;// view point in window
float persPt::Z0 = 400.0f;// for perspective scaling of x and y coordinates
vec3f persPt::camPos = vec3f(0.0f,0.0f,0.0f);// camera position in scene
vec3f persPt::zuCam = vec3f(0.0f,0.0f,1.0f);// direction pointed
vec3f persPt::yuCam = vec3f(0.0f,1.0f,0.0f);    // unit vector "up" (camera orientation)
vec3f persPt::xuCam = vec3f(1.0f,0.0f,0.0f);    // unit vector "to right"
const vec3f persPt::xHat = persPt::xuCam;
const vec3f persPt::yHat = persPt::yuCam;
const vec3f persPt::zHat = persPt::zuCam;
float persPt::angle = 0.0f;// yaw
float persPt::pitchAngle = 0.0f;// pitch
float persPt::rollAngle = 0.0f;// roll

// static functions
// get the window position from the vec3f position in the scene
vec2f persPt::get_xyw( vec3f Pos )
{
    vec3f Rcp = Pos - camPos;// from camera to Pos
    float U = Rcp.dot( zuCam );// projection along zuCam = "z - Zcam"
    vec3f Rcp_perp = Rcp - U*zuCam;// component of Rcp in plane perpendicular to zuCam
    vec3f L = ( Z0/U )*Rcp_perp;// perspective transform applied
    if( U < 0.0f ) L *= -1.0f;
//    vec3f L = ( Z0/Rcp.mag() )*Rcp_perp;// perspective transform applied
    return vec2f( X0 + L.dot( xuCam ), Yh - L.dot( yuCam ) );
}

float persPt::changeCamDir( float dAy, float dAp, float dAr )// returns compass heading
{
    return changeCamDir_mat( dAy, dAp, dAr );// matrix based method

    // this code works fine too
    zuCam = zuCam*cosf(dAy) + xuCam*sinf(dAy);// yaw
    xuCam = yuCam.cross( zuCam );

    zuCam = zuCam*cosf(dAp) - yuCam*sinf(dAp);// pitch
    yuCam = zuCam.cross( xuCam );

    yuCam = yuCam*cosf(dAr) + xuCam*sinf(dAr);// roll
    yuCam /= yuCam.mag();// renormalize: this has proven necessary - NEW
    xuCam = yuCam.cross( zuCam );

    // compass heading
    vec3f Rh = zuCam - zuCam.y*yHat;// component of zuCam in x,z plane
    Rh /= Rh.mag();// normalize
    float a = acosf( Rh.dot( zHat ) );
    if( Rh.dot( xHat ) < 0.0f ) a *= -1.0f;
    return a*180.0f/3.14159f;
}


float persPt::changeCamDir_mat( float dAy, float dAp, float dAr )// returns compass heading
{
    float M[3][3];
    float SnAy = sinf(dAy), CsAy = cosf(dAy);
    float SnAp = sinf(dAp), CsAp = cosf(dAp);
    float SnAr = sinf(dAr), CsAr = cosf(dAr);

    M[0][0] = CsAr*CsAy - SnAr*SnAp*SnAy;    M[0][1] = -SnAr*CsAp;   M[0][2] = -CsAr*SnAy - SnAy*SnAp*SnAr;
    M[1][0] = SnAr*CsAy + SnAy*SnAp*CsAr;    M[1][1] = CsAr*CsAp;    M[1][2] = -SnAr*SnAy + SnAy*SnAp*CsAr;
    M[2][0] = CsAp*SnAy;                     M[2][1] = -SnAp;        M[2][2] = CsAp*CsAy;
    vec3f xcOld = xuCam, ycOld = yuCam, zcOld = zuCam;

    xuCam     = M[0][0]*xcOld + M[0][1]*ycOld + M[0][2]*zcOld;
    yuCam     = M[1][0]*xcOld + M[1][1]*ycOld + M[1][2]*zcOld;
    zuCam = M[2][0]*xcOld + M[2][1]*ycOld + M[2][2]*zcOld;

    // compass heading
    vec3f Rh( zuCam.x, 0.0f, zuCam.z );// = zuCam - zuCam.y*yHat;// component of zuCam in x,z plane
    Rh /= Rh.mag();// normalize
    float a = acosf( Rh.dot( zHat ) );
    if( Rh.dot( xHat ) < 0.0f ) a *= -1.0f;
    return a*180.0f/3.14159f;
}

void persPt::calibrateCameraAxes()
{
    persPt::zuCam /= persPt::zuCam.mag();
    persPt::xuCam = persPt::yuCam.cross( persPt::zuCam );
    persPt::xuCam /= persPt::xuCam.mag();
//    persPt::yuCam = persPt::xuCam.cross( persPt::zuCam );
    persPt::yuCam = persPt::zuCam.cross( persPt::xuCam );
}

// array of persPt pointers
void persPt::sortByDistance( persPt** pPtVec, size_t k )// sort only 1st k elements
{
    for( size_t iterCurr = 0; iterCurr + 1 < k; ++ iterCurr )
    {
        if( !pPtVec[iterCurr]->doDraw ) continue;// just leave it in place and move to the next

        size_t iterMax = iterCurr;
        for( size_t i = iterCurr + 1; i < k; ++i )
        {
            if( !pPtVec[iterCurr]->doDraw ) continue;
//            if( pPtVec[i]->getDistance() > pPtVec[iterMax]->getDistance() )
            if( persPt::compare( pPtVec[i], pPtVec[iterMax] ) )
                iterMax = i;
        }

        if( iterMax > iterCurr )// swap values
        {
            persPt* pPtTemp = pPtVec[ iterCurr ];
            pPtVec[ iterCurr ] = pPtVec[ iterMax ];
            pPtVec[ iterMax ] = pPtTemp;
        }
    }
}

/*
bool persPt::init_stat( std::istream& is )
{
    if( !is ) return false;
    is >> persPt::Z0;
    is >> persPt::camPos.x >> persPt::camPos.y >> persPt::camPos.z;
    is >> persPt::zuCam.x >> persPt::zuCam.y >> persPt::zuCam.z;
    zuCam /= zuCam.mag();
    persPt::xuCam = persPt::yHat.cross( persPt::zuCam );
    persPt::xuCam /= persPt::xuCam.mag();
    persPt::yuCam = persPt::zuCam.cross( persPt::xuCam );
    return true;
}
*/

float persPt::set_rw( vec3f Pos, float R, CircleShape& rCS )
{
    float U = ( Pos - persPt::camPos ).dot( persPt::zuCam );
    if( U < 0.0f ) U *= -1.0f;
    R *= persPt::Z0/U;
    rCS.Radius = R;
    rCS.pos = get_xyw( Pos );
    return R;
}

float persPt::zScale( vec3f P )// Z0/Z
{
    return Z0/zuCam.dot( P - camPos );
}

void persPt::updatePOI( float R, float angleSpeed, float dt )
{
    vec3f Pc = camPos + R*zuCam;
    zuCam = zuCam*cosf( angleSpeed*dt ) - xuCam*sinf( angleSpeed*dt );// yaw
 //   vec3f xuCam0 = xuCam;
    xuCam = yuCam.cross( zuCam );
    camPos = Pc - R*zuCam;
 //   camPos += ( xuCam + xuCam0 )*R*angleSpeed*dt;
}

void persPt::pitchCam( float dAngle )
{
    zuCam = zuCam*cosf(dAngle) - yuCam*sinf(dAngle);// pitch
    zuCam /= zuCam.mag();
    yuCam = zuCam.cross( xuCam );
}

void persPt::yawCam( float dAngle )
{
    zuCam = zuCam*cosf(dAngle) + xuCam*sinf(dAngle);// yaw
    zuCam /= zuCam.mag();
    xuCam = yuCam.cross( zuCam );
}

void persPt::rollCam( float dAngle )
{
    yuCam = yuCam*cosf(dAngle) + xuCam*sinf(dAngle);// roll
    yuCam /= yuCam.mag();
    xuCam = yuCam.cross( zuCam );
}

void persPt::keepXuLevel()// counter roll induced by pitching and yawing
{
    xuCam = yHat.cross( zuCam );
    float xuCamMag = xuCam.mag();
    if( xuCamMag > 0.1f ) xuCam /= xuCamMag;
    else persPt::xuCam = xHat;

    persPt::yuCam = zuCam.cross( xuCam );
}

void persPt::updateHorizonLine( vec2f& pt0, vec2f& pt1 )
{
    // direction toward horizon
    float zcDotY = zuCam.dot( yHat );
    vec3f hu = zuCam - zcDotY*yHat;
    float huMag = hu.mag();
    if( huMag < 0.01f ) hu = zHat;
    else hu /= huMag;
    vec3f hu2 = yHat.cross( hu );// along horizon
    // horizon line
    pt0 = persPt::get_xyw( persPt::camPos + persPt::Z0*( hu - hu2 ) );// left end
    pt1 = persPt::get_xyw( persPt::camPos + persPt::Z0*( hu + hu2 ) );// right end
}

// regular methods
void persPt::pitch( float dAngle )
{
    Zu = Zu*cosf(dAngle) - Yu*sinf(dAngle);// pitch
    Zu /= Zu.mag();
    Yu = Zu.cross( Xu );
}

void persPt::yaw( float dAngle )
{
    Zu = Zu*cosf(dAngle) + Xu*sinf(dAngle);// yaw
    Zu /= Zu.mag();
    Xu = Yu.cross( Zu );
}

void persPt::roll( float dAngle )
{
    Yu = Yu*cosf(dAngle) + Xu*sinf(dAngle);// roll
    Yu /= Yu.mag();
    Xu = Yu.cross( Zu );
}

void persPt::update_doDraw()
{
    doDraw = false;
    float U = persPt::zuCam.dot( pos - persPt::camPos );
    if( U < Rbound ) return;// don't draw objects that are behind the camera
    vec2f winPos = get_xyw();
    float dim = Rbound*persPt::Z0/U;
    if( winPos.x + dim < 0.0f ) return;// left of window
    if( winPos.x - dim > 2.0f*persPt::X0 ) return;// right of window
    if( winPos.y + dim < 0.0f ) return;// above window
    if( winPos.y - dim > 2.0f*persPt::Yh ) return;// below window
    doDraw = true;
}


// same as persBall::hitFree, but with r1 = 0 and OK if isMoving
bool persPt::hit( vec3f posA, vec3f posB, vec3f& P, vec3f& vu )const
{
    vec3f rA = posA - pos, rB = posB - pos;
    if( rB.mag() > 3.0f*( Rbound ) ) return false;// too far away
    vec3f Tu = posB - posA; Tu /= Tu.mag();
    vec3f Tn = rB.cross( Tu ); Tn /= Tn.mag();
    vec3f T1 = Tu.cross( Tn );
    float r2 = Rbound;
//    if( rB.mag() > 3.0f*( r1 + r2 ) ) return false;// too far away
    float h = rA.dot( T1 );
    if(  h >= r2 ) return false;// miss. ball centers are too far apart
    float w = sqrtf( r2*r2 - h*h );
    P = h*T1 - w*Tu;
    if( Tu.dot( P - rA ) < 0.0f || Tu.dot( P - rB ) > 0.0f ) return false;// posA and posB are on the same side of P
    // It's a hit!
    vec3f pu = P/P.mag();
    P += pos;// for return
    vec3f qu = Tn.cross( pu );
    vu = (Tu.dot(qu))*qu - (Tu.dot(pu))*pu;
    return true;
}

bool persPt::hit_image( float xHit, float yHit )const
{
    vec2f winPos = persPt::get_xyw( pos );
    float SF = persPt::Z0/persPt::zuCam.dot( pos - persPt::camPos );// scale factor
    if( ( xHit - winPos.x )*( xHit - winPos.x ) + ( yHit - winPos.y )*( yHit - winPos.y ) < SF*SF*Rbound*Rbound ) return true;
    return false;
}

bool persPt::isSighted()const// in line with zuCam?
{
    vec3f sep = pos - persPt::camPos;
    float s = persPt::zuCam.dot( sep );
    vec3f h = sep - persPt::zuCam*s;
    return Rbound > h.mag();
}