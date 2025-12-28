#ifndef AL_GALLERYVIEWER_H
#define AL_GALLERYVIEWER_H

#include "Arduino.h"// for type String in updateDisplay()

#include "ArduinoLevel.h"
#include "../LightSourceTypes/BuffSrcLoadfromFile.h"
#include "SSD_1306Component.h"// use a display

class AL_GalleryViewer : public ArduinoLevel
{
    public:
    // MUST HAVE members
    LightGrid Target_LG;// owned by user
    void bindToGrid( Light* pGrid0, int GridRows, int GridCols )
    { Target_LG.init( *pGrid0, GridRows, GridCols ); }

    // rows, cols match the source image dimensions
    int row0 = 0, col0 = 0;
    float tDisplay = 5.0f, tElapDisplay = 0.0f;
    int idxDisplay = 0;
    char shiftDirection = 'L';// cycle: L. U, R, D
    void cycleShift();

    // each image is loaded to BuffB_LG. This is copied to BuffA_LG before loading the next image
    LightGrid BuffA_LG, BuffB_LG;// previous image on A, current image on B
    // draw from buffer to Target_LG
    BufferSource BuffA_Src, BuffB_Src;// draw from BuffA_LG (or BuffB_LG) to Target_LG
    BufferSource BtoA_Src;// draw from BuffB_LG to BuffA_LG
    bool initBuffSources( const char* fileName );
    int numFiles = 0;// until allocated
    String* imageName;// dynamic
    Light clearLt;// fill around small images

    // for use with menu
    int menuButtID = 2;
    int rotEncID = 1;
    float tScale = 0.5f;// for adjustment by 1 second
    int numOptions = 3;// next frame, adjust tDisplay and Quit
    int menuIter = 0;
    SSD1306_Display* pDisplay = nullptr;
    void updateDisplay()const;

    bool setup( const char* fileName, SSD1306_Display* p_Display );
    virtual bool update( float dt );
    virtual void draw()const;
    virtual bool handleEvent( ArduinoEvent& rEvent );

    AL_GalleryViewer(){}
    virtual ~AL_GalleryViewer()
    {
        if( imageName )
        {
            if( numFiles == 1 ) delete imageName;
            else delete [] imageName;
        }
    }

    protected:

    private:
};

#endif // AL_GALLERYVIEWER_H
