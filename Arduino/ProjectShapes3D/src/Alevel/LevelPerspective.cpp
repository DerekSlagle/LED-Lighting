#include "LevelPerspective.h"

void LevelPerspective::setup()
{
  MenuLine::setupInOut( *pDisplay, pActButt, pMenuButt, pRotEncButt, *pRotEncDelta );
  Shape::bindToGrid( Target_LG.pLt0, Target_LG.rows, Target_LG.cols );
  setup3D();

  // a page
  FL_CamPosX.setupBase( "Campos X: " );// head
  FL_CamPosX.setupFloat( persPt::camPos.x, -1000.0f, 1000.0f );
  FL_CamPosX.inScale = 0.5f;
  FL_CamPosX.pNextLine = &FL_CamPosY;

  FL_CamPosY.setupBase( "Campos Y: " );
  FL_CamPosY.setupFloat( persPt::camPos.y, -1000.0f, 1000.0f );
  FL_CamPosY.inScale = 0.5f;
  FL_CamPosY.pNextLine = &FL_CamPosZ;

  FL_CamPosZ.setupBase( "Campos Z: " );
  FL_CamPosZ.setupFloat( persPt::camPos.z, -1000.0f, 1000.0f );
  FL_CamPosZ.inScale = 0.5f;
  FL_CamPosZ.pNextLine = &FL_Yaw;

  FL_Yaw.setupBase( "Heading: " );
  compassHeading = persPt::changeCamDir_mat( 0.0f, 0.0f, 0.0f );// initial value
  FL_Yaw.setupFloat( compassHeading, -200.0f, 200.0f );
  FL_Yaw.inScale = 0.087266f;// pi/36 radians = 5 degree increments
  FL_Yaw.pNextLine = &FL_Pitch;

  FL_Pitch.setupBase( "Pitch: " );
  pitchAngle = 57.296f*asinf( persPt::zuCam.dot( persPt::yHat ) );// initial value in degrees
  FL_Pitch.setupFloat( pitchAngle, -90.0f, 90.0f );
  FL_Pitch.inScale = 0.087266f;// pi/36 radians = 5 degree increments
  FL_Pitch.pNextLine = &FL_Roll;

  FL_Roll.setupBase( "Roll: " );
  rollAngle = 57.296f*asinf( persPt::xuCam.dot( persPt::yHat ) );// initial value in degrees
  FL_Roll.setupFloat( rollAngle, -90.0f, 90.0f );
  FL_Roll.inScale = 0.087266f;// pi/36 radians = 5 degree increments
  FL_Roll.pNextLine = &FL_Z0;

  FL_Z0.setupBase( "Z0: " );
  persPt::Z0 = 40.0f;
  FL_Z0.setupFloat( persPt::Z0, 10.0f, 200.0f );
  FL_Z0.inScale = 0.5f;
  FL_Z0.pNextLine = nullptr;

  thePage.setup( " ** Perspect 3D **", FL_CamPosX );  
  if( !ignoreInput ) updateDisplay();
}

void LevelPerspective::setup3D()
{
  persPt::Yh = 16.0f;
//  persPt::Z0 = 32.0f;
  persPt::X0 = 16.0f;
  persPt::camPos = vec3f(0.0f,10.0f,0.0f);
  for( int n = 0; n < numShapes; ++n ) pPersShape[n] = pPersSorted[n] = nullptr;
  // setup circles
  circleArr[0].init( vec3f( -40.0f, 0.0f, 140.0f ), 4.0f, Light( 120, 0, 20 ), vec3f() );
  circleArr[1].init( vec3f( -20.0f, 0.0f, 100.0f ), 4.0f, Light( 100, 60, 40 ), vec3f() );
  circleArr[2].init( vec3f( 0.0f, 0.0f, 60.0f ), 4.0f, Light( 80, 0, 60 ), vec3f() );
  circleArr[3].init( vec3f( 20.0f, 0.0f, 100.0f ), 4.0f, Light( 60, 40, 80 ), vec3f() );
  circleArr[4].init( vec3f( 40.0f, 0.0f, 140.0f ), 4.0f, Light( 40, 0, 100 ), vec3f() );
  // assign pointers
  for( int n = 0; n < numCircles; ++n ) pPersShape[n] = circleArr + n;
}

void LevelPerspective::processInput()
{
    if( ignoreInput ) return;

    if( thePage.update() )
    {
        // follow up on value changes
        if( MenuLine::pRotEncDelta && *MenuLine::pRotEncDelta != 0 )
        {
          int RotEncDelta = *MenuLine::pRotEncDelta;
            if( thePage.pCurrLine == &FL_Yaw )
            {
              float dAy = FL_Yaw.inScale*RotEncDelta;
              compassHeading = persPt::changeCamDir_mat( dAy, 0.0f, 0.0f );// initial value
              thePage.updateDisplay();
            }
            else if( thePage.pCurrLine == &FL_Pitch )
            {
              float dAp = FL_Pitch.inScale*RotEncDelta;
              compassHeading = persPt::changeCamDir_mat( 0.0f, dAp, 0.0f );
              pitchAngle = 57.296f*asinf( persPt::zuCam.dot( persPt::yHat ) );// in degrees
              thePage.updateDisplay();
            }
            else if( thePage.pCurrLine == &FL_Roll )
            {
              float dAr = FL_Roll.inScale*RotEncDelta;
              compassHeading = persPt::changeCamDir_mat( 0.0f, 0.0f, dAr );
              pitchAngle = 57.296f*asinf( persPt::zuCam.dot( persPt::yHat ) );// in degrees
              thePage.updateDisplay();
            }
        }
    }
}

void LevelPerspective::update( float dt )
{
   if( !Target_LG.pLt0 ) return;
   processInput();   
   
  numToDraw = 0;
  for( int n = 0; n < numShapes; ++n )
  {
    if( !pPersShape[n] ) break;// got them all
    pPersShape[n]->update(dt);// not moving
    if( pPersShape[n]->doDraw && numToDraw < numShapes )
    {
      pPersSorted[ numToDraw ] = pPersShape[n];
      ++numToDraw;
    }
  }
  // sort? later
    
    return;
}

void LevelPerspective::draw()const
{
    if( !Target_LG.pLt0 ) return;
    // draw background
  const Light skyColor( 0, 0, 20 );
  const Light groundColor( 10, 10, 10 );
  vec2f pt0, pt1;
  persPt::updateHorizonLine( pt0, pt1 );
  float m = ( pt1.y - pt0.y )/( pt1.x - pt0.x );// slope
  for( int c = 0; c < Target_LG.cols; ++c )
  {
    for( int r = 0; r < Target_LG.rows; ++r )
    {
      int rLn = pt0.y + m*( c - pt0.x );// r0 + m*(c-c0)
      if( r < rLn ) Target_LG.pLt0[ r*Target_LG.cols + c ] = skyColor;
      else Target_LG.pLt0[ r*Target_LG.cols + c ] = groundColor;
    }
  }
  
  for( int n = 0; n < numToDraw; ++n )
  {
    pPersSorted[n]->draw();
  }
}


void LevelPerspective::updateDisplay()const
{
    String msg = thePage.draw();
    // write
    pDisplay->clear();
    pDisplay->printAt( 0, 0, msg.c_str(), 1 );
    pDisplay->show();
}