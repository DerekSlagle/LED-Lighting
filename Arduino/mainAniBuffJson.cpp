// display a full color image

#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>// a micro SD drive

#include "FileParser.h"
//#include "SSD_1306Component.h"// a tiny oled panel

#include "LightPanel.h"

// the full target rectangle
const int gridRows = 32, gridCols = 32;
const unsigned int numLEDs = static_cast<unsigned int>( gridRows*gridCols );

//const unsigned int numPanels = 4;
//LightPanel panel[ numPanels ];

// now dynamic allocation
unsigned int numPanels = 4;
LightPanel* panel = nullptr;

CRGB leds[numLEDs];// panels map to this array
Light clearLt;

#include "BufferSource.h"// includes LightGrid and LightSource
Light LightArr[numLEDs];// players write to this static array
//Light* LightArr = nullptr;// players write to this dynamic array
LightGrid LightArr_LG;// owns the Lights

// A BufferSource for the animation
LightGrid* pAni_LG = nullptr;// delete in dtor
unsigned int numAni_LG = 0;// until allocated in function below
BufferSource AniBuff_Src;

// memory for a LightPool
// size = 20*(32*32*3) = 20*3Kb = 60 Kb
Light MemData[ 20*numLEDs ];// up to 20 full size animation frames

// time delta
float dtLoop = 0.02f;// entire loop

void init_panels();// per coded configuration
bool init_SD();
bool SDgood = false;
const String rootFileName( "/SourcePlayers" );
int numFiles = 0;// dynamically allocated to below
String* pFileNames = nullptr;
int* pFormatID = nullptr;// one per file
int fileIndex = 0;// cycle through

//int imageHeight = gridRowsFull, imageWidth = gridColsFull;
void LoadAnimation();

#include "switchSPST.h"
switchSPST pushButtA( 2, 0.04f, false );// regular button. Press for next animation
switchSPST pushButtB( 3, 0.04f, true );// cap touch button. Press to rotate image

bool updateLEDS = false;
void LoadNextAnimation()// callback for pushButtA
{
  fileIndex = ( 1 + fileIndex )%numFiles;
  LightArr_LG.Clear( clearLt );
  LoadAnimation();
  updateLEDS = true;
}

void rotateCanvasCW();
void rotateCanvasCCW();
void RotateCanvas()// callback for pushButtB
{
  AniBuff_Src.doBlend = !AniBuff_Src.doBlend;
  if( AniBuff_Src.doBlend )
  {
    LightArr[24].setRGB( 160, 0, 0 );
    LightArr[25].setRGB( 160, 0, 0 );
    LightArr[26].setRGB( 160, 0, 0 );
  }
  else
  {
    LightArr[24] = clearLt;
    LightArr[25] = clearLt;
    LightArr[26] = clearLt;
  }

  updateLEDS = true;
}

// a rotary encoder to adjust brightness
int pinEncoderCLK = 6, pinEncoderDT = 7, pinEncoderSW = 8;
int lastClkState = HIGH;
switchSPST pushButtEncSW( pinEncoderSW, 0.06f );// for encoder SW input
void updateEncoder();// callback for interrupt
volatile int LEDbrightness = 64;// assigned in updateEncoder() to range 0 to 510. Use half value as brightness
int currBrightness = LEDbrightness;// change event detection
// a numeric display for brightness
#include "IntegerSource.h"
IntegerSource brightDisplay;
void newBright()// callback for encoder push button aka: pushButtEncSW
{
  LEDbrightness = 64;// reset: use double the brightness value  
  FastLED.setBrightness( LEDbrightness/2 );
  currBrightness = LEDbrightness;// reset
  
  // re draw base image if number of digits reduced
  if( brightDisplay.setValue( LEDbrightness/2 ) ) AniBuff_Src.draw();
  brightDisplay.draw();
  updateLEDS = true;
}

#include "SSD_1306Component.h"// a tiny oled panel
SSD1306_Display theOled;

#include "ArduinoJson.h"
//DynamicJsonDocument jsonDocument(10*1024);
//JsonDocument jsonDocument;

void setup()
{
  theOled.setupDisplay();
  theOled.setTextColor( COLOR_WHITE );

  SDgood = init_SD();
  if( !SDgood ) return;
  
  pushButtA.onClose = LoadNextAnimation;
  pushButtB.onOpen = RotateCanvas;
  pushButtEncSW.onClose = newBright;  

  // Allocate main Grid dynamically to replace previous LightArr
  //LightArr_LG.init( gridRows, gridCols );// main grid
  //LightArr = LightArr_LG.pLt0;// for existing usage of LightArr in code here
  clearLt.setRGB( 0, 10, 20 );  

  init_panels();

  IntegerSource::init_static();
  // a numeric display
  brightDisplay.setTarget( LightArr_LG );
  // set initial brightness  
  LEDbrightness = 64;
  brightDisplay.setValue( LEDbrightness/2 );
  brightDisplay.bgLt.setRGB( 20,20,20 );
  brightDisplay.fgLt.setRGB( 200,200,200 );
  brightDisplay.setPosition( 2, 2 );
  
 // for( unsigned int n = 0; n < numLEDs; ++n )  
//    LightArr[n] = clearLt;  

  // initialize LightGrids
  LightArr_LG.init( LightArr[0], gridRows, gridCols );// main grid
  // initialize BufferSources
  AniBuff_Src.setTarget( LightArr_LG );
  LightArr_LG.Clear( clearLt );
  //AniBuff_Src.setSource( BuffA_LG );
  AniBuff_Src.setPosition( 0, 0 );
  

  fileIndex = -1;// will be incremented before use
  LoadNextAnimation();// first one

  // panel orientation test = pass!
  LightArr[0].setRGB(255,255,255);
  LightArr[16].setRGB(255,255,255);
  LightArr[24].setRGB(255,255,255);
  LightArr[512].setRGB(255,255,255);
  
  // for encoder
  pinMode( pinEncoderCLK, INPUT_PULLUP );
  pinMode( pinEncoderDT, INPUT_PULLUP );
  pinMode( pinEncoderSW, INPUT_PULLUP );
  lastClkState = digitalRead(pinEncoderCLK);// initial value  
  // Attach interrupt
  attachInterrupt(digitalPinToInterrupt(pinEncoderCLK), updateEncoder, CHANGE);

  FastLED.addLeds<WS2812, 8, GRB>(leds, numLEDs).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness( LEDbrightness/2 );
  // write to leds
  FastLED.clear();
  // LightPanels map from LightArr to leds
  for( unsigned int p = 0; p < numPanels; ++p )
    panel[p].update(); 
	FastLED.show();
}


void loop() 
{
  if( !SDgood ) return;

  static unsigned long lastTime = micros();
  unsigned long currTime = micros();
  dtLoop = ( currTime - lastTime )*0.000001f;
  lastTime = currTime; 

  AniBuff_Src.update( dtLoop );  
  AniBuff_Src.draw();
  updateLEDS = true;  
  
  pushButtA.update( dtLoop );// start next animation
  pushButtB.update( dtLoop );// rotate canvas CCW
  pushButtEncSW.update( dtLoop );// reset brightness?

  // encoder changed LEDbrightness
  if( currBrightness != LEDbrightness )
  {
    currBrightness = LEDbrightness;// reset
    FastLED.setBrightness( LEDbrightness/2 );
    
    // re draw base image if number of digits reduced
    if( brightDisplay.setValue( LEDbrightness/2 ) ) AniBuff_Src.draw();
    brightDisplay.draw();

    updateLEDS = true;      
  }
  
  if( updateLEDS )
  {
    FastLED.clear();

    // map LightArr to leds
    for( unsigned int p = 0; p < numPanels; ++p )
      panel[p].update();
    
    FastLED.show();
    updateLEDS = false;
  }
  else
    delay(10); 

}// end of loop()

void updateEncoder()// callback for interrupt
{
  int currentClkState = digitalRead(pinEncoderCLK);
  int currentDtState = digitalRead(pinEncoderDT);

  if (currentClkState != lastClkState) { // Check for a change in CLK
    if (currentDtState != currentClkState) { // Determine direction
      if( LEDbrightness < 510 ) LEDbrightness++; // Clockwise
    } else {
      if( LEDbrightness > 0 ) LEDbrightness--; // Counter-clockwise
    }    
  }

  lastClkState = currentClkState;
}

void rotateCanvasCW()
{
  
  // new method for CW canvas rotation not working
  for( unsigned int n = 0; n < numPanels; ++n )
  {
    // increment each rotIdx
    
    // rotate each row0, col0 back
    int newRow0 = gridRows - panel[n].col0 - panel[n].cols;
    int newCol0 = panel[n].row0;
    // swap rows, cols
    panel[n].set_SrcArea( panel[n].cols, panel[n].rows, newRow0, newCol0 );
    // assign swapTgtRCs = true if 8x24 is horizontal
    if( panel[n].cols != panel[n].rows )// 8x32
    {
      panel[n].swapTgtRCs = panel[n].cols > panel[n].rows;
      if( panel[n].swapTgtRCs )// went horizontal
      {
        if( panel[n].rotIdx == 0 ) panel[n].rotIdx = 2;
        else panel[n].rotIdx = 0;
      }
    }
    else// square panel
    {
      ++panel[n].rotIdx;
      if( panel[n].rotIdx > 2 ) panel[n].rotIdx = -1;
    }
  }

  return;
}

void rotateCanvasCCW()
{
  for( unsigned int n = 0; n < numPanels; ++n )
  {
    // increment each rotIdx
    
    // rotate each row0, col0 forward
    int newRow0 = panel[n].col0;
    int newCol0 = gridRows - panel[n].row0 - panel[n].rows;
    // swap rows, cols
    panel[n].set_SrcArea( panel[n].cols, panel[n].rows, newRow0, newCol0 );
    // assign swapTgtRCs = true if 8x24 is horizontal
    if( panel[n].cols != panel[n].rows )// 8x32
    {
      panel[n].swapTgtRCs = panel[n].cols > panel[n].rows;
      if( !panel[n].swapTgtRCs )// went vertical
      {
        if( panel[n].rotIdx == 0 ) panel[n].rotIdx = 2;
        else panel[n].rotIdx = 0;
      }
    }
    else// square panel
    {
      --panel[n].rotIdx;
      if( panel[n].rotIdx < -1 ) panel[n].rotIdx = 2;
    }
  }
}

void init_panels()
{  
  Light* pLt0 = leds;

  String fName = rootFileName + "/panels_init.txt";
  FileParser FP( fName.c_str() );

  int NP = 0;
  FP >> NP;// no support for unsigned int
  numPanels = NP;
  panel = new LightPanel[ numPanels];

  for( unsigned int p = 0; p < numPanels; ++p )// all same
  {
    panel[p]. init_Src( LightArr, gridRows, gridCols );
    int Rows, Cols, Row0, Col0;
    FP >> Rows >> Cols >> Row0 >> Col0;
    panel[p].set_SrcArea( Rows, Cols, Row0, Col0 );
    FP >> panel[p].rotIdx;
    FP >> panel[p].type;
    panel[p].pTgt0 = pLt0;
    pLt0 += Rows*Cols;
    // for special 32x8 case
    if( Cols > Rows ) 
      panel[p].swapTgtRCs = true;
    else
      panel[p].swapTgtRCs = false;
  } 
}

//JsonDocument initJson;
bool init_SD()
{
  /*
  if (!SD.begin(10)) {
    return false;
  }
  String mainFileName = rootFileName + "/SD_init_json.json";
  DeserializationError parsingError = deserializeJson(initJson, mainFileName);
  if (parsingError != DeserializationError::Ok) {
    return false;
  }

  numFiles = initJson["files"].size();
  pFileNames = new String[numFiles];
  for (int n = 0; n < numFiles; ++n) {
    pFileNames[n] = initJson["files"][n].as<String>();
  }
  return true;
  */

  
  bool SD_good = SD.begin( 10 );
  String oledReport = SD_good ? "SD Good!" : "SD Bad!";
  
  theOled.clear();
  theOled.printAt( 0, 0, oledReport.c_str(), 1 );
  theOled.show();

  if( SD_good )
  {
    String mainFileName = rootFileName + "/SD_init.txt";
    FileParser FP( mainFileName.c_str() );

    FP >> numFiles;// images to cycle through
    pFileNames = new String[numFiles];
    pFormatID = new int[numFiles];
    for( int n = 0; n < numFiles; ++n )
    { 
      FP >> pFormatID[n];
      FP >> pFileNames[n]; 
    }
  }

  return SD_good;
}

// ** load animation functions **
void LoadAnimation_fromBytes(  const char* fName  )
{
  FileParser fin( fName );
  // temp: burn the type
  int type = 0;
  fin >> type;// cross check?

  float tFrame = 0.1f;
  fin >> tFrame;

//    fin >> BA.sizeBits;
    int Rows = 0, Cols = 0;
    fin >> Rows >> Cols;
    fin >> AniBuff_Src.row0 >> AniBuff_Src.col0;

    // color data
    int NumColors = 0;
    fin >> NumColors;
    Light* pColor = new Light[ NumColors ];
    int rd = 0, gn = 0, bu = 0;
    for( unsigned int n = 0; n < NumColors; ++n )
    {
        fin >> rd >> gn >> bu;
        pColor[n].setRGB(rd,gn,bu);
    }

    // index data
    bitArray BA;
    int Cap = 0, szBits = 0;
    fin >> Cap >> szBits;
    BA.capBytes = Cap;
    BA.sizeBits = szBits;
    BA.pByte = new uint8_t[ BA.capBytes ];
    int temp = 0;// transfer value to uint8_t
    for( unsigned int n = 0; n < BA.capBytes; ++n )
    {
        fin >> temp;
        BA.pByte[n] = temp;
    }

    // allocate the LightGrids
    unsigned int bitsPerLt = 0;
    if( NumColors > 8 && NumColors <= 16 )
        bitsPerLt = 4;
    else if( NumColors > 4 )// 5,6,7,8
        bitsPerLt = 3;
    else if( NumColors > 2 )// 3 or 4
        bitsPerLt = 2;
    else// 2 colors
        bitsPerLt = 1;

    // number of frames = number of LightGrids
    unsigned int newNum_LG = BA.sizeBits/( Rows*Cols*bitsPerLt ); 
    if( newNum_LG < 20 )// TEMP
    {
        // check for existing allocation
        // This function will be called to load a new animation
        if( pAni_LG && ( newNum_LG != numAni_LG ) )// else keep it
        {
            if( numAni_LG < 1 )// abnormal condition
                return;// dtor will cope?

            // out with the old
            // keep if numAni_LG is same?
            if( numAni_LG > 1 ) delete [] pAni_LG;
            else delete pAni_LG;
            pAni_LG = nullptr;// no danglers!!
            numAni_LG = 0;// until allocated below
        }
        // allocate here. CLEANUP in destructor
        if( !pAni_LG )// may have saved above
        {
            numAni_LG = newNum_LG;
            pAni_LG = new LightGrid[ numAni_LG ];
        }
        // final check
        if( !pAni_LG ) return;
        // proceed

        Light* pStore = MemData;
        unsigned int LtCount = 0;// iterator for bitArray
        for( unsigned int n = 0; n < numAni_LG; ++n )
        {
        //    pAni_LG[n].init( Rows, Cols );// Lights allocated dynamically
            pAni_LG[n].init( pStore[0], Rows, Cols );// Lights allocated from MemData
            pStore += Rows*Cols;// for next time

            for( int k = 0; k < Rows*Cols; ++k )
            {
                uint8_t val = 0;
                if( bitsPerLt == 4 )
                    val = BA.getQuadBit( LtCount );
                else if( bitsPerLt == 3 )
                    val = BA.getTriBit( LtCount );
                else if( bitsPerLt == 2 )
                    val = BA.getDblBit( LtCount );
                else val = BA.getBit( LtCount ) ? 1 : 0;

                pAni_LG[n].pLt0[k] = pColor[ val ];
                ++LtCount;
            }
        }

        AniBuff_Src.setSource( pAni_LG[0], numAni_LG, tFrame );
        AniBuff_Src.setTarget( LightArr_LG );
    }
    else
    {
        return;
    }

    // cleanup color and index data
    delete [] pColor;
    delete [] BA.pByte;

    // ready
    AniBuff_Src.draw();

    // oled report
    String str = "From Bytes";
    str += "\nRows: ";
    str += Rows;
    str += "  Cols: ";
    str += Cols;
    str += "\nnumFrames: ";
    str += numAni_LG;
    str += "\nnumLights: ";
    str += numAni_LG*Rows*Cols;

    theOled.clear();
    theOled.printAt( 0, 0, str.c_str(), 1 );
    theOled.show();

    return;
}

// type = 4
JsonDocument jsonDocument;
void LoadAnimation_fromBytes_Json(  const char* fName  )
{
  String jsonString;
  File file = SD.open(fName, FILE_READ);
  if (!file) return;
  jsonString = file.readString();
  file.close();  
  DeserializationError err = deserializeJson(jsonDocument, jsonString);
  // need this?
   if ( err != DeserializationError::Ok )  return;
 
  // parse
  int type = jsonDocument["type"].as<int>();
  float tFrame = jsonDocument["frametime"].as<float>();
  int Rows = 0, Cols = 0, Row0 = 0, Col0 = 0;
  Rows = jsonDocument["rows"].as<int>();
  Cols = jsonDocument["cols"].as<int>();
  Row0 = jsonDocument["row0"].as<int>();
  Col0 = jsonDocument["col0"].as<int>();
  AniBuff_Src.row0 = Row0;
  AniBuff_Src.col0 = Col0;
  bitArray BA;
  BA.sizeBits = jsonDocument["sizeBits"].as<int>();

// for later
int NumColors = 0;
Light *pColor = nullptr;
  if (jsonDocument["colors"].is<JsonArray>()) 
  {
    JsonArray arr = jsonDocument["colors"];
    NumColors = (int)arr.size();
    pColor = new Light[NumColors];
    for (int i = 0; i < NumColors; i++) {
      pColor[i].r = arr[i]["r"].as<uint8_t>();
      pColor[i].g = arr[i]["g"].as<uint8_t>();
      pColor[i].b = arr[i]["b"].as<uint8_t>();
    }
  }

  // index data
  size_t NumBytes = 0;
  if (jsonDocument["data"].is<JsonArray>()) 
  {
    JsonArray arr = jsonDocument["data"];
    NumBytes = arr.size();
    BA.capBytes = NumBytes;
    BA.pByte = new uint8_t[ BA.capBytes ];
    for (int i = 0; i < NumBytes; i++) {
      BA.pByte[i] = arr[i].as<uint8_t>();// ???
    }
  }
  else return; 

    // allocate the LightGrids
    unsigned int bitsPerLt = 0;
    if( NumColors > 8 && NumColors <= 16 )
        bitsPerLt = 4;
    else if( NumColors > 4 )// 5,6,7,8
        bitsPerLt = 3;
    else if( NumColors > 2 )// 3 or 4
        bitsPerLt = 2;
    else// 2 colors
        bitsPerLt = 1;

    // number of frames = number of LightGrids
    unsigned int newNum_LG = BA.sizeBits/( Rows*Cols*bitsPerLt ); 
    if( newNum_LG < 20 )// TEMP
    {
        // check for existing allocation
        // This function will be called to load a new animation
        if( pAni_LG && ( newNum_LG != numAni_LG ) )// else keep it
        {
            if( numAni_LG < 1 )// abnormal condition
                return;// dtor will cope?

            // out with the old
            // keep if numAni_LG is same?
            if( numAni_LG > 1 ) delete [] pAni_LG;
            else delete pAni_LG;
            pAni_LG = nullptr;// no danglers!!
            numAni_LG = 0;// until allocated below
        }
        // allocate here. CLEANUP in destructor
        if( !pAni_LG )// may have saved above
        {
            numAni_LG = newNum_LG;
            pAni_LG = new LightGrid[ numAni_LG ];
        }
        // final check
        if( !pAni_LG ) return;
        // proceed

        Light* pStore = MemData;
        unsigned int LtCount = 0;// iterator for bitArray
        for( unsigned int n = 0; n < numAni_LG; ++n )
        {
        //    pAni_LG[n].init( Rows, Cols );// Lights allocated dynamically
            pAni_LG[n].init( pStore[0], Rows, Cols );// Lights allocated from MemData
            pStore += Rows*Cols;// for next time

            for( int k = 0; k < Rows*Cols; ++k )
            {
                uint8_t val = 0;
                if( bitsPerLt == 4 )
                    val = BA.getQuadBit( LtCount );
                else if( bitsPerLt == 3 )
                    val = BA.getTriBit( LtCount );
                else if( bitsPerLt == 2 )
                    val = BA.getDblBit( LtCount );
                else val = BA.getBit( LtCount ) ? 1 : 0;

                pAni_LG[n].pLt0[k] = pColor[ val ];
                ++LtCount;
            }
        }

        AniBuff_Src.setSource( pAni_LG[0], numAni_LG, tFrame );
        AniBuff_Src.setTarget( LightArr_LG );
    }
    else
    {
        return;
    }

    // cleanup color and index data
    delete [] pColor;
    delete [] BA.pByte;

    // ready
    AniBuff_Src.draw();

    // oled report
    String str = "From Bytes Json";
    str += "\nRows: ";
    str += Rows;
    str += "  Cols: ";
    str += Cols;
    str += "\nnumFrames: ";
    str += numAni_LG;
    str += "\nnumLights: ";
    str += numAni_LG*Rows*Cols;

    theOled.clear();
    theOled.printAt( 0, 0, str.c_str(), 1 );
    theOled.show();

    return;
}

void LoadAnimation_fromColorsOneFile(  const char* fName  )
{
  FileParser FP( fName );
  // temp: burn the type
  int type = 0;
  FP >> type;// cross check?

  // allocate the frames
  float t_Frame = 0.1f;
  FP >> t_Frame;
  int PlayForward = 1, doBlend = 0;
  FP >> PlayForward >> doBlend;
  AniBuff_Src.playForward = ( PlayForward == 1 );
  AniBuff_Src.doBlend = false;//( doBlend == 1 );

  int Rows, Cols = 0;// frame LightGrids
  FP >> Rows >> Cols >> AniBuff_Src.row0 >> AniBuff_Src.col0;
  
  // number of frames = number of files
  int newNum_LG = 0;
 //   fin >> numAni_LG;
    FP >> newNum_LG;
    if( newNum_LG < 20 )// proceed
    {
        // check for existing allocation
        // This function will be called to load a new animation
        if( pAni_LG && ( newNum_LG != numAni_LG ) )// else keep
        {
            if( numAni_LG < 1 )// abnormal condition
                return;// dtor will cope?

            // out with the old
            // keep if numAni_LG is same?
            if( numAni_LG > 1 ) delete [] pAni_LG;
            else delete pAni_LG;
            pAni_LG = nullptr;// no danglers!!
            numAni_LG = 0;// until allocated below
        }
        // allocate here. CLEANUP in destructor
        if( !pAni_LG )// may have saved above
        {
            numAni_LG = newNum_LG;
            pAni_LG = new LightGrid[ numAni_LG ];
        }
        // final check
        if( !pAni_LG ) return;
        // proceed
        Light* pStore = MemData;// memory pool
        for( unsigned int n = 0; n < numAni_LG; ++n )
        {
            pAni_LG[n].init( pStore[0], Rows, Cols );// Lights allocated from MemData
            pStore += Rows*Cols;
            FP.readLightArray( pAni_LG[n].pLt0, pAni_LG[n].rows*pAni_LG[n].cols, clearLt );            
        }

        AniBuff_Src.setSource( pAni_LG[0], numAni_LG, t_Frame );
        AniBuff_Src.setTarget( LightArr_LG );
        AniBuff_Src.draw();
    }

}

void setImage( LightGrid& LG, const char* fName );// from colors
void LoadAnimation_fromColors(  const char* fName )
{
  FileParser FP( fName );
  // temp: burn the type
  int type = 0;
  FP >> type;// cross check?

  // allocate the frames
  float t_Frame = 0.1f;
  FP >> t_Frame;
  int PlayForward = 1, doBlend = 0;
  FP >> PlayForward >> doBlend;
  AniBuff_Src.playForward = ( PlayForward == 1 );
  AniBuff_Src.doBlend = ( doBlend == 1 );

  int Rows, Cols = 0;// frame LightGrids
  FP >> Rows >> Cols >> AniBuff_Src.row0 >> AniBuff_Src.col0;
  
  // number of frames = number of files
  int newNum_LG = 0;
 //   fin >> numAni_LG;
    FP >> newNum_LG;
    if( newNum_LG < 20 )// proceed
    {
        // check for existing allocation
        // This function will be called to load a new animation
        if( pAni_LG && ( newNum_LG != numAni_LG ) )// else keep
        {
            if( numAni_LG < 1 )// abnormal condition
                return;// dtor will cope?

            // out with the old
            // keep if numAni_LG is same?
            if( numAni_LG > 1 ) delete [] pAni_LG;
            else delete pAni_LG;
            pAni_LG = nullptr;// no danglers!!
            numAni_LG = 0;// until allocated below
        }
        // allocate here. CLEANUP in destructor
        if( !pAni_LG )// may have saved above
        {
            numAni_LG = newNum_LG;
            pAni_LG = new LightGrid[ numAni_LG ];
        }
        // final check
        if( !pAni_LG ) return;
        // proceed
        Light* pStore = MemData;// memory pool
        for( unsigned int n = 0; n < numAni_LG; ++n )
        {
        //    pAni_LG[n].init( Rows, Cols );// Lights allocated dynamically
            pAni_LG[n].init( pStore[0], Rows, Cols );// Lights allocated from MemData
            pStore += Rows*Cols;
            String frameFname;
            FP >> frameFname;
            setImage( pAni_LG[n], frameFname.c_str() );
        }


        AniBuff_Src.setSource( pAni_LG[0], numAni_LG, t_Frame );
        AniBuff_Src.setTarget( LightArr_LG );
        AniBuff_Src.draw();
    }

    // oled report
    String str = "From Colors";
    str += "\nRows: ";
    str += Rows;
    str += "  Cols: ";
    str += Cols;
    str += "\nnumFrames: ";
    str += numAni_LG;
    str += "\nnumLights: ";
    str += numAni_LG*Rows*Cols;

    theOled.clear();
    theOled.printAt( 0, 0, str.c_str(), 1 );
    theOled.show();

}

void LoadAnimation()
{
  AniBuff_Src.playForward = true;

  if( pFormatID[ fileIndex ] == 1 )
  {
    LoadAnimation_fromColors( pFileNames[ fileIndex ].c_str() );
    
  }
  else if( pFormatID[ fileIndex ] == 2 )
  {
    LoadAnimation_fromBytes( pFileNames[ fileIndex ].c_str() );
  }
  else if( pFormatID[ fileIndex ] == 3 )
  {
    LoadAnimation_fromColorsOneFile( pFileNames[ fileIndex ].c_str() );
  }
  else if( pFormatID[ fileIndex ] == 4 )
  {
    LoadAnimation_fromBytes_Json( pFileNames[ fileIndex ].c_str() );
  }
}

// working!
void setImage( LightGrid& LG, const char* fName )
{
  FileParser FP( fName );
  int Cols = 1, Rows = 6;
  FP >> Rows >> Cols;
  FP.readLightArray( LG.pLt0, LG.rows*LG.cols, clearLt );
}

/*
void setImage( LightGrid& LG, const char* fName )
{
  // open the file for the image data
  FileParser FP( fName );
  int Cols = 1, Rows = 6;
  FP >> Rows >> Cols;
  if( Cols > LG.cols ) Cols = LG.cols;// check
  if( Rows > LG.rows ) Rows =  LG.rows;// check  

//  int rd, gn, bu, al;// each Light
  uint8_t* pRGBA = new uint8_t[ 4*Rows*Cols ];
  FP.readByteArray( pRGBA, 4*Rows*Cols );
  for( int n = 0; n < Rows*Cols; ++n )
  {      
  //  FP >> rd >> gn >> bu >> al;
 //   if( al == 0 )// alpha value     
    int N4 = 4*n;   
    if( pRGBA[ N4 + 3 ] == 0 )// alpha value        
      LG.pLt0[n] = clearLt;        
    else 
      LG.pLt0[n].setRGB( pRGBA[ N4 ], pRGBA[ N4 + 1 ], pRGBA[ N4 + 2 ] );      
  }

  delete [] pRGBA;
}
  */

/*
void setImage( LightGrid& LG, const char* fName )
{
  // open the file for the image data
  FileParser FP( fName );
  int Cols = 1, Rows = 6;
  FP >> Rows >> Cols;
  if( Cols > LG.cols ) Cols = LG.cols;// check
  if( Rows > LG.rows ) Rows =  LG.rows;// check  

  int rd, gn, bu, al;// each Light
  for( int n = 0; n < Rows*Cols; ++n )
  {      
    FP >> rd >> gn >> bu >> al;
    if( al == 0 )// alpha value        
      LG.pLt0[n] = clearLt;        
    else 
      LG.pLt0[n].setRGB( rd, gn, bu );      
  }
}
*/