#include "FileParser.h"

FileParser::FileParser(const String& filename, Mode mode)
    : filename(filename)
    , mode(mode) {
    
    // Ensure directory exists for write/append modes
    if (mode != Mode::READ) {
        String dir = filename.substring(0, filename.lastIndexOf('/'));
        if (dir.length() > 0 && !SD.exists(dir.c_str())) {
            SD.mkdir(dir.c_str());
        }
    }
    
    // Open file in appropriate mode
    if (mode == Mode::READ) {
        file = SD.open(filename.c_str(), FILE_READ);
    } else if (mode == Mode::WRITE) {
        file = SD.open(filename.c_str(), FILE_WRITE);
    } else { // APPEND
        file = SD.open(filename.c_str(), FILE_APPEND);
    }
    
    if (file) {
        Serial.print("[FileParser] Opened ");
        Serial.print(filename);
        Serial.print(" in ");
        Serial.println(mode == Mode::READ ? "READ" : mode == Mode::WRITE ? "WRITE" : "APPEND");
    } else {
        Serial.print("[FileParser] Failed to open ");
        Serial.println(filename);
    }
}

FileParser::~FileParser() {
    if (file) {
        close();
    }
}

void FileParser::close() {
    if (file) {
        flush();
        file.close();
        Serial.print("[FileParser] Closed ");
        Serial.println(filename);
    }
}

// Input operators (>>) - for reading
FileParser& FileParser::operator>>(int& value) {
    if (mode != Mode::READ || !file) return *this;
    
    String token = getNextToken();
    if (token.length() > 0) {
        value = token.toInt();
    }
    return *this;
}

FileParser& FileParser::operator>>(float& value) {
    if (mode != Mode::READ || !file) return *this;
    
    String token = getNextToken();
    if (token.length() > 0) {
        value = token.toFloat();
    }
    return *this;
}

FileParser& FileParser::operator>>(double& value) {
    if (mode != Mode::READ || !file) return *this;
    
    String token = getNextToken();
    if (token.length() > 0) {
        value = token.toFloat(); // Arduino doesn't have double, use float
    }
    return *this;
}

FileParser& FileParser::operator>>(String& value) {
    if (mode != Mode::READ || !file) return *this;
    
    value = getNextToken();
    return *this;
}

FileParser& FileParser::operator>>(char& value) {
    if (mode != Mode::READ || !file) return *this;
    
    if (file.available()) {
        value = file.read();
    }
    return *this;
}

// Output operators (<<) - for writing
FileParser& FileParser::operator<<(int value) {
    if (mode == Mode::READ || !file) return *this;
    
    file.print(value);
    return *this;
}

FileParser& FileParser::operator<<(float value) {
    if (mode == Mode::READ || !file) return *this;
    
    file.print(value);
    return *this;
}

FileParser& FileParser::operator<<(double value) {
    if (mode == Mode::READ || !file) return *this;
    
    file.print(value);
    return *this;
}

FileParser& FileParser::operator<<(const String& value) {
    if (mode == Mode::READ || !file) return *this;
    
    file.print(value);
    return *this;
}

FileParser& FileParser::operator<<(const char* value) {
    if (mode == Mode::READ || !file) return *this;
    
    file.print(value);
    return *this;
}

FileParser& FileParser::operator<<(char value) {
    if (mode == Mode::READ || !file) return *this;
    
    file.print(value);
    return *this;
}

// Helper methods
String FileParser::readLine() {
    if (mode != Mode::READ || !file) return "";
    
    return file.readStringUntil('\n');
}

void FileParser::writeLine(const String& line) {
    if (mode == Mode::READ || !file) return;
    
    file.println(line);
}

void FileParser::flush() {
    if (file) {
        file.flush();
    }
}

// Private helper methods
String FileParser::getNextToken() {
    if (mode != Mode::READ || !file) return "";
    
    skipWhitespace();
    
    String token = "";
    while (file.available()) {
        char c = file.peek();
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            break;
        }
        token += (char)file.read();
    }
    
    return token;
}

void FileParser::skipWhitespace() {
    if (mode != Mode::READ || !file) return;
    
    while (file.available()) {
        char c = file.peek();
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
            break;
        }
        file.read(); // Skip this character
    }
}

bool FileParser::readByteArray( uint8_t* pArr, int Size )
{
    if (mode != Mode::READ || !file) return false;
  //  if( Size == file.read( pArr, Size ) ) return true;// faster but result is garbage
  //  return false;

    int numRead = 0;
    char N = 0;
    bool newN = true;
    while( file.available() )
    {        
        char c = file.read();        
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
        {
            char digit = c - '0';
            if( newN )
            {
                N = digit;
                newN = false;                
            }
            else
            {
                N = 10*N + digit;
            }
        }
        else// white space
        { 
            if( !newN )
            {       
                pArr[ numRead ] = (uint8_t)N;
                if( ++numRead == Size ) return true;
                newN = true;       
            }
        }        
    }

    // no space at end? get the last value
    if( !newN )
    {       
        pArr[ numRead ] = (uint8_t)N;
        if( ++numRead == Size ) return true;
    }

    return false;
}

// working
bool FileParser::readIntArray( int* pArr, int Size )
{
    if (mode != Mode::READ || !file) return false;
    
    int numRead = 0;
    int N = 0;
    bool newN = true;
    while( file.available() )
    {        
        char c = file.read();        
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
        {
            if( newN )
            {
                N = (int)( c - '0' );
                newN = false;
            }
            else
            {
                N = 10*N + (int)( c - '0' );
            }
        }
        else// white space
        { 
            if( !newN )
            {       
                pArr[ numRead ] = N;
                if( ++numRead == Size ) return true;
                newN = true;       
            }
        }        
    }

    // no space at end? get the last value
    if( !newN )
    {       
        pArr[ numRead ] = N;
        if( ++numRead == Size ) return true;
    }

    return false;
}

// working!
bool FileParser::readLightArray( Light* pLt, int Size, Light bkgdLt )
{
    if (mode != Mode::READ || !file) return false;

    // for writing
    uint8_t* pByte = &( pLt[0].r );// 1st byte

    int numRead = 0;
    char N = 0;// build up to byte value
    bool newN = true;// index to byte in the Light = 0,1,2 or 3
    int idxByte = 0;// 0,1,2 = r,g,b
    while( file.available() )
    {        
        char c = file.read();        
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
        {
            char digit = c - '0';
            if( newN )
            {
                N = digit;
                newN = false;               
            }
            else
            {
                N = 10*N + digit;
            }
        }
        else// white space
        { 
            if( !newN )
            {      
                ++idxByte;
                if( idxByte < 4 )// 1,2,3 = r,g,b read
                {
                    *pByte = (uint8_t)N;
                    ++pByte;
                    newN = true;
                }
                else// alpha value and Light is read
                {
                    if( N == 0 )// assign bkgdLt to r,g,b just read
                    {
                        pByte[-3] = bkgdLt.r;
                        pByte[-2] = bkgdLt.g;
                        pByte[-1] = bkgdLt.b;
                    }
                    idxByte = 0;
                    newN = true;
                    // next Light
                    if( ++numRead == Size ) return true; 
                }       
                      
            }
        }        
    }

    // no space at end? get the last value
    if( !newN )
    {      
        ++idxByte;
        if( idxByte < 4 )// r,g,b read
        {
            *pByte = (uint8_t)N;
            ++pByte;
            newN = true;
        }
        else// alpha value and Light is read
        {
            if( N == 0 )// assign bkgdLt to r,g,b just read
            {
                pByte[-3] = bkgdLt.r;
                pByte[-2] = bkgdLt.g;
                pByte[-1] = bkgdLt.b;
            }
            idxByte = 0;
            newN = true;
            // next Light
            if( ++numRead == Size ) return true; 
        }       
                
    }

    return false;
}

/*
// working!
bool FileParser::readIntArray( int* pArr, int Size )
{
    if (mode != Mode::READ || !file) return false;
    
    int numRead = 0;
    String token = "";
     while( file.available() )
     {        
        char c = file.read();        
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
        {
            token += c;
        }
        else// white space
        {
            if (token.length() > 0) 
            {
                pArr[ numRead ] = token.toInt();
                if( ++numRead == Size ) return true;
                token = "";
            }
            continue;
        }        
     }

    return false;
}

// also working
bool FileParser::readIntArray( int* pArr, int Size )
{
    if (mode != Mode::READ || !file) return false;
    
    for( int n = 0; n < Size; ++n )
    {
        String token = getNextToken();
        if (token.length() > 0) {
            pArr[n] = token.toInt();
        }
        else return false;// did not catch them all
    }

    return true;
}
*/