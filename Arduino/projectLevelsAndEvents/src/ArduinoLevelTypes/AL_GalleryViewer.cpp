#include "AL_GalleryViewer.h"

bool AL_GalleryViewer::setup( const char* fileName, SSD1306_Display* p_Display )
{
    row0 = 0;
    col0 = 0;
    tDisplay = 5.0f;
    tElapDisplay = 0.0f;
    idxDisplay = 0;
    shiftDirection = 'L';// cycle: L. U, R, D

    pDisplay = p_Display;
    menuIter = 0;
    updateDisplay();

    return initBuffSources( fileName );
}

bool AL_GalleryViewer::handleEvent( ArduinoEvent& rEvent )
{
    // handle Quit up front
    if( rEvent.type == -1 && (rEvent.ID == actButtID) && (menuIter == numOptions - 1) )
        return false;// Quit

    // return will be true from here
    if( rEvent.type == 1 )// button press
    {
        if( rEvent.ID == actButtID )
        {
            if( menuIter == 0 )
            {   // next frame now
                tElapDisplay = tDisplay;
            }
        }
        else if( rEvent.ID == menuButtID )// menu scroll button
        {
            menuIter = ( 1 + menuIter )%numOptions;
            updateDisplay();
        }
    }
    else if( rEvent.type == -1 )// button release
    {
        if( rEvent.ID == actButtID )// act
        {
            if( menuIter == 0 )
            {
         //       tFrame = 1.0f;
            }
        }
    }
    else if( rEvent.type == 2 && rEvent.ID == rotEncID )// rotary encoder
    {   // 2nd on menu
        if( menuIter == 1 )
        {
            tDisplay += tScale*rEvent.value;
            if( tDisplay < 1.0f ) tDisplay = 1.0f;
            tElapDisplay = 0.0f;
            updateDisplay();
        }
    }

    return true;
}

bool AL_GalleryViewer::update( float dt )
{
    bool retVal = false;
    tElapDisplay += dt;
    if( tElapDisplay > tDisplay )
    {
        tElapDisplay = 0.0f;
        BtoA_Src.draw();// copy image to BuffA_Arr
        idxDisplay = ( 1 + idxDisplay )%numFiles;
        loadColors( BuffB_LG, imageName[idxDisplay].c_str(), clearLt );
        BuffB_LG.RotateCW();
        cycleShift();
        BuffB_Src.draw();
        retVal = true;
    }
    else if( BuffB_Src.cShift != 0 || BuffB_Src.rShift != 0 )
    {
        if( BuffB_Src.cShift > 0 )
        {
            --BuffB_Src.cShift;
            BuffA_Src.cShift = BuffB_Src.cShift - BuffB_Src.getCols();// A is left of B
        }
        else if( BuffB_Src.cShift < 0 )
        {
            ++BuffB_Src.cShift;
            BuffA_Src.cShift = BuffB_Src.getCols() + BuffB_Src.cShift;// A is right of B
        }

        if( BuffB_Src.rShift > 0 )
        {
            --BuffB_Src.rShift;
            BuffA_Src.rShift = BuffB_Src.rShift - BuffB_Src.getRows();// A is above B
        }
        else if( BuffB_Src.rShift < 0 )
        {
            ++BuffB_Src.rShift;
            BuffA_Src.rShift = BuffB_Src.getRows() + BuffB_Src.rShift;// A is below B
        }

        BuffA_Src.draw();
        BuffB_Src.draw();

        // last iteration
        if( BuffB_Src.cShift == 0 && BuffB_Src.rShift == 0 )
        {
            BuffA_Src.rShift = BuffA_Src.cShift = 0;// reset
        }

        retVal = true;
    }

    return retVal;
}

void AL_GalleryViewer::draw()const
{
    if( !Target_LG.pLt0 ) return;
    BuffA_Src.draw();
    BuffB_Src.draw();
}

void AL_GalleryViewer::updateDisplay()const
{
    if( !pDisplay ) return;// crash avoidance
    String msg( "Gallery Viewer" );
    msg += ( menuIter == 0 ) ? "\n* " : "\n  ";
    msg += "Next Image";
    msg += ( menuIter == 1 ) ? "\n* " : "\n  ";
    msg += "tDisplay: ";
    msg += tDisplay;
    // Quit
    msg += ( menuIter == numOptions - 1 ) ? "\n* " : "\n  ";
    msg += "QUIT to menu";
    // write
    pDisplay->clear();
    pDisplay->printAt( 0, 0, msg.c_str(), 1 );
    pDisplay->show();
}

void AL_GalleryViewer::cycleShift()// cycle: L. U, R, D
{
    if( shiftDirection == 'L' )
    {
        shiftDirection = 'U';
        BuffB_Src.rShift = BuffB_LG.rows;
    }
    else if( shiftDirection == 'U' )
    {
        shiftDirection = 'R';
        BuffB_Src.cShift = -BuffB_LG.cols;
    }
    else if( shiftDirection == 'R' )
    {
        shiftDirection = 'D';
        BuffB_Src.rShift = -BuffB_LG.rows;
    }
    else
    {
        shiftDirection = 'L';
        BuffB_Src.cShift = BuffB_LG.cols;
    }
}

bool AL_GalleryViewer::initBuffSources( const char* fileName )
{
    FileParser fin( fileName );
 //   if( !fin ) return false;

    // 2 buffer arrays are unseen
    int BuffRows = 0, BuffCols = 0;
    fin >> BuffRows >> BuffCols;
    BuffA_LG.init( BuffRows, BuffCols );
    BuffA_LG.Clear( clearLt );// 1st image to slide out is blank
    BuffB_LG.init( BuffRows, BuffCols );
    int targetRow0 = 0, targetCol0 = 0;// applied to BuffB_Src then drawn to BuffA and LightArr
    fin >> targetRow0 >> targetCol0;

    int rd, gn, bu;
    fin >> rd >> gn >> bu;
    clearLt.setRGB(rd,gn,bu);

 //   unsigned int numFiles = 0;
    fin >> numFiles;
    imageName = new String[ numFiles ];
    for( unsigned int n = 0; n < numFiles; ++n )
        fin >> imageName[n];

    if( !loadColors( BuffB_LG, imageName[0].c_str(), clearLt ) )
    {
    //    std::cout << "\nImage Load Fail";
        return false;
    }
    BuffB_LG.RotateCW();

    BuffA_Src.setTarget( Target_LG );
    BuffA_Src.setSource( BuffA_LG );
    BuffA_Src.setPosition( 0, 0 );
    BuffA_Src.row0 = targetRow0;
    BuffA_Src.col0 = targetCol0;

    BuffB_Src.setTarget( Target_LG );
    BuffB_Src.setSource( BuffB_LG );
    shiftDirection = 'L';
    BuffB_Src.row0 = 0;
    BuffB_Src.col0 = 0;
    BuffB_Src.cShift = BuffB_LG.cols - 1;
    BuffB_Src.row0 = targetRow0;
    BuffB_Src.col0 = targetCol0;
    BuffB_Src.outLineThickness = 0;

    BtoA_Src.setTarget( BuffA_LG );
    BtoA_Src.setSource( BuffB_LG );
    BtoA_Src.setPosition( 0, 0 );

    BuffB_Src.draw();// clearLt

    return true;
}
