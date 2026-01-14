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
  FL_CamPosX.inBoost = 16.0f;
  FL_CamPosX.pNextLine = &FL_CamPosY;

  FL_CamPosY.setupBase( "Campos Y: " );
  FL_CamPosY.setupFloat( persPt::camPos.y, -1000.0f, 1000.0f );
  FL_CamPosY.inScale = 0.5f;
  FL_CamPosY.inBoost = 16.0f;
  FL_CamPosY.pNextLine = &FL_CamPosZ;

  FL_CamPosZ.setupBase( "Campos Z: " );
  FL_CamPosZ.setupFloat( persPt::camPos.z, -1000.0f, 1000.0f );
  FL_CamPosZ.inScale = 0.5f;
  FL_CamPosZ.inBoost = 16.0f;
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
  circleArr[0].init( vec3f( -40.0f, 4.0f, 140.0f ), 4.0f, Light( 120, 0, 20 ), vec3f() );
  circleArr[1].init( vec3f( -20.0f, 4.0f, 100.0f ), 4.0f, Light( 100, 60, 40 ), vec3f() );
  circleArr[2].init( vec3f( 0.0f, 4.0f, 60.0f ), 4.0f, Light( 80, 0, 60 ), vec3f() );
  circleArr[3].init( vec3f( 20.0f, 4.0f, 100.0f ), 4.0f, Light( 60, 40, 80 ), vec3f() );
  circleArr[4].init( vec3f( 40.0f, 4.0f, 140.0f ), 4.0f, Light( 40, 0, 100 ), vec3f() );
  // rectangles
  rectArr[0].init( vec3f( 20.0f, 16.0f, -200.0f ), 12.0f, 8.0f, vec3f(0.0f,0.0f,1.0f), Light(100,0,100) );
  rectArr[1].init( vec3f( -30.0f, 16.0f, 200.0f ), 12.0f, 8.0f, vec3f(0.0f,0.0f,1.0f), Light(100,100,0) );
  // assign pointers
  int N = 0;
  for( int k = 0; k < numCircles; ++k ) pPersShape[N++] = circleArr + k;
  // rects
  pPersShape[N++] = rectArr;
  pPersShape[N++] = rectArr + 1;

  // lines NOT a persPt type. draw after background
  lineArr[0].init( vec3f( -6.0f, 0.0f, -300.0f ), vec3f( -6.0f, 0.0f, 300.0f ), Light(0,0,0) );
  lineArr[1].init( vec3f( 6.0f, 0.0f, -300.0f ), vec3f( 6.0f, 0.0f, 300.0f ), Light(0,0,0) );
  // east and west markers
  lineArr[2].init( vec3f( -500.0f, 10.0f, -10.0f ), vec3f( -500.0f, 10.0f, 10.0f ), Light(200,0,60) );
  lineArr[3].init( vec3f( 500.0f, 10.0f, -10.0f ), vec3f( 500.0f, 10.0f, 10.0f ), Light(60,200,0) );
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
              rollAngle = 57.296f*asinf( persPt::xuCam.dot( persPt::yHat ) );// in degrees
              thePage.updateDisplay();
            }
        }        
    }

    // force it
    // reset camera directions
    if( MenuLine::pRotEncButt && MenuLine::pRotEncButt->pollEvent() == 1 )
    {
      if( thePage.pCurrLine == &FL_Yaw )
      {
        persPt::xuCam = persPt::xHat;
        persPt::yuCam = persPt::yHat;
        persPt::zuCam = persPt::zHat;
        compassHeading = 0.0f;
        pitchAngle = 0.0f;
        rollAngle = 0.0f;
        thePage.updateDisplay();
      }
      else if( thePage.pCurrLine == &FL_CamPosX ) persPt::camPos.x = 0.0f;
      else if( thePage.pCurrLine == &FL_CamPosY ) persPt::camPos.y = 10.0f;
      else if( thePage.pCurrLine == &FL_CamPosZ ) persPt::camPos.z = 0.0f;
    }
}

void LevelPerspective::update( float dt )
{
   if( !Target_LG.pLt0 ) return;
   processInput();

   // lines
   for( int n = 0; n < 4; ++n )
    lineArr[n].update(dt);
   
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
  // sort? OK do it
  persPt::sortByDistance( pPersSorted, numToDraw );
    
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
  // so this works upside down
  Light clrUp = ( pt1.x > pt0.x ) ? skyColor : groundColor;
  Light clrDown = ( pt1.x > pt0.x ) ? groundColor : skyColor;
  float m = ( pt1.y - pt0.y )/( pt1.x - pt0.x );// slope
  for( int c = 0; c < Target_LG.cols; ++c )
  {
    for( int r = 0; r < Target_LG.rows; ++r )
    {
      int rLn = pt0.y + m*( c - pt0.x );// r0 + m*(c-c0)
      if( r < rLn ) Target_LG.pLt0[ r*Target_LG.cols + c ] = clrUp;
      else Target_LG.pLt0[ r*Target_LG.cols + c ] = clrDown;
    }
  }

  // lines
  for( int n = 0; n < 4; ++n )
    lineArr[n].draw();

  // persPt types
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