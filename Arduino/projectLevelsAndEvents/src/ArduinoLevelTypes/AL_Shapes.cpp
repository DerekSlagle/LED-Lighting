#include "AL_Shapes.h"

bool AL_Shapes::setup()
{
    clearLt.setRGB( 0, 10, 20 );
    Light LineColor( 100, 0, 60 );
    lineLength = 10.0f;
    int Row0 = 16,  Col0 = 16;
    int RowF = 16,  ColF = Col0 + lineLength;
    theLine.setup( LineColor, Row0, Col0, RowF, ColF );
    theLine.doBlend = false;
    setupMenu();
    thePage.Display();
    
    return true;
}

void AL_Shapes::setupMenu()
{
    FL_LinePosX.setupBase( "Col0: ", nullptr, &FL_LinePosY );
    FL_LinePosX.setupFloat( theLine.pos.x, -50.0f, 50.0f, 0.5f );
    FL_LinePosY.setupBase( "Row0: ", nullptr, &FL_LineLength );
    FL_LinePosY.setupFloat( theLine.pos.y, -50.0f, 50.0f, 0.5f );
    FL_LineLength.setupBase( "Length: ", nullptr, &FL_LineAngle );
    FL_LineLength.setupFloat( lineLength, 2.0f, 50.0f, 0.5f );
    FL_LineAngle.setupBase( "Angle: ", nullptr, &ML_DrawMethod );
    FL_LineAngle.setupFloat( lineAngle, -400.0f, 400.0f, 0.5f );
    ML_DrawMethod.setupBase( "draw 1 or 2", nullptr, &ML_Quit );
    ML_Quit.setupBase( "Quit" );
    thePage.setup( " -- Shapes --", FL_LinePosX );
}

bool AL_Shapes::update( float dt )
{         
    // no action
    return true;
}

void AL_Shapes::draw()const
{
    if( !pLt0 ) return;
    // clear
    for( int n = 0; n < gridRows*gridCols; ++n )
        pLt0[n] = clearLt;

    if( do_draw2 )
    {
        theLine.draw3();
        pLt0[0].setRGB(1,200,0);
    }
    else
         theLine.draw2();
}

bool AL_Shapes::handleEvent( ArduinoEvent& AE )
{    
    if( !thePage.handleEvent( AE ) ) return false;

    // assign theLine.pos2 if any float value changes
    if( (AE.type == 2 && AE.ID == 1) || (AE.type == 1 && AE.ID == 3) )// rotary encoder
    {
        const MenuLine* pCL = thePage.pCurrLine;
        if( pCL == &FL_LinePosX || pCL == &FL_LinePosY || pCL == &FL_LineLength || pCL == &FL_LineAngle )
        {
            theLine.pos2.x = theLine.pos.x + lineLength*cosf( lineAngle*0.017453f );
            theLine.pos2.y = theLine.pos.y + lineLength*sinf( lineAngle*0.017453f );
        }
    }

    if( thePage.actButtPressed && thePage.pCurrLine == &ML_DrawMethod )// actButt pressed
    {
        do_draw2 = !do_draw2;
    }

    return true;
}