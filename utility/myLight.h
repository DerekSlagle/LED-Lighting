#ifndef MYLIGHT_H
#define MYLIGHT_H

#include<iostream>
#include<vector>
#include<cmath>
#include<functional>

class myLight
{
    public:

    unsigned int rd = 1, gn = 2, bu = 3;// so can test for default construction
    float brightness = 0.5f;// 0 to 1
    void init( unsigned int Rd, unsigned int Gn, unsigned int Bu, float Brightness )
    { rd = Rd; gn = Gn; bu = Bu; brightness = Brightness;  }
    myLight( unsigned int Rd, unsigned int Gn, unsigned int Bu, float Brightness ){ init( Rd, Gn, Bu, Brightness ); }
    myLight(){}

    bool init( std::istream& is )
        { is >> rd >> gn >> bu; return is.good(); }
    void to_ostream( std::ostream& os )const
        { os << rd << ' ' << gn << ' ' << bu << ' '; }

    bool operator == ( const myLight& Lt )const
    {
        if( rd != Lt.rd || gn != Lt.gn || bu != Lt.bu ) return false;
        if( brightness != Lt.brightness ) return false;
        return true;
    }

    bool operator != ( const myLight& Lt )const
    {
        if( rd != Lt.rd || gn != Lt.gn || bu != Lt.bu ) return true;// not equal
        if( brightness != Lt.brightness ) return true;// not equal
        return false;// they are equal
    }
};

class lightGroup
{
public:
    myLight* pLt0 = nullptr;
    unsigned int rows = 1, cols = 1;

    float tLimit = 1.0f, tElap = 0;// to aid animation use

    void init( myLight* pLt, unsigned int numLts )
    { pLt0 = pLt; cols = numLts; rows = 1; }
    lightGroup( myLight* pLt, unsigned int numLts ) { init( pLt, numLts ); }

    void init( myLight* pLt, unsigned int Rows, unsigned int Cols )
    { pLt0 = pLt; cols = Cols; rows = Rows; }
    lightGroup( myLight* pLt, unsigned int Rows, unsigned int Cols ) { init( pLt, Rows, Cols ); }
    lightGroup(){}


    virtual void setPosition( unsigned int row0, unsigned int col0 ){}// no sensible meaning in base?
    virtual myLight getLight( unsigned int Row, unsigned int Col )const;// override in lightGroupImage where source is pImg

    // actions on the lights. writes to lights pointed to = OK
    virtual myLight* get_pLt( unsigned int Row, unsigned int Col )const { return pLt0 + cols*Row + Col; }
    void Clear( myLight clearLight )const;
    void copyFrom( const myLight& Lt_toCopy,  unsigned int toRow, unsigned int toCol  )const;// target row, col
    void copyFrom( const lightGroup& LG,  unsigned int toRow0, unsigned int toCol0  )const;// target row, col

    // if cols > 1
    void shiftLeft()const;// copy to left
    void shiftRight()const;// copy to right
    // if rows > 1
    void shiftUp()const;// copy upward
    void shiftDown()const;// copy

    void shiftLeft( lightGroup& buffGrp, unsigned int buffCol, unsigned int bcShft = 0 )const;

    void scroll( char Dir, float dt );// 'L', 'R', 'U' and 'D'. Writes to tElap

    // alter states
    void setBrightness( float Brightness )const;
    // streaming states
    void drawFromFunction( std::function<float(float,float)> Yxt, myLight LtF )const;
    void drawFrom_istream( std::istream& is )const;
    void drawFrom_sceGroup( const lightGroup& sceGroup )const;// already bound to targetGroup
};

class lightGroupGrid : public lightGroup
{
public:
    myLight* pGrid0 = nullptr;// = pLt0 in target group
    unsigned int gridRows = 1, gridCols = 1;

    unsigned int getRow0()const { return ( pLt0 - pGrid0 )/gridCols; }
    unsigned int getCol0()const { return ( pLt0 - pGrid0 )%gridCols; }
    virtual void setPosition( unsigned int row0, unsigned int col0 );// assign new pLt0
    virtual myLight* get_pLt( unsigned int Row, unsigned int Col )const { return pLt0 + gridCols*Row + Col; }

    virtual void init( myLight* pLt, unsigned int Rows, unsigned int Cols, myLight* pGrid, unsigned int GridRows, unsigned int GridCols )
    { pLt0 = pLt; cols = Cols; rows = Rows; pGrid0 = pGrid; gridCols = GridCols; gridRows = GridRows; }

    lightGroupGrid ( myLight* pLt, unsigned int Rows, unsigned int Cols, myLight* pGrid, unsigned int GridRows, unsigned int GridCols )
    { init( pLt, Rows, Cols, pGrid, GridRows, GridCols ); }

    lightGroupGrid(){}

    lightGroupGrid& operator=( const lightGroupGrid& GG );
    lightGroupGrid& operator=( const lightGroup& LtGrp );



    // space frame r0 rows and c0 cols from grid group. false if off bounding grid
    bool makeFrame( std::vector<myLight*>& pLtVec, unsigned int r0, unsigned int c0 )const;// writes to LtGp
};

class groupGridMove : public lightGroupGrid
{
    public:
    std::vector<myLight> swatchVec;// saved background to restore before moving
    void updateSwatch();

    // restore swatch, assign new pLt0, save new swatch
    // setting light states from source is not the groups job. global functions use a group
    virtual void setPosition( unsigned int row0, unsigned int col0 );
    // fill swatchVec with initial swatch
    virtual void init( myLight* pLt, unsigned int Rows, unsigned int Cols, myLight* pGrid, unsigned int GridRows, unsigned int GridCols );
};

class groupGridRotate : public lightGroupGrid
{
    public:
    std::vector<float> colVec, rowVec;// row, col pair for each to be rotated
    float rotAng = 0.0f;// assign then map image
 //   virtual void setPosition( unsigned int row0, unsigned int col0 );// base version should be good
    // fill rowVec and colVec. rows = cols only
    void init( myLight* pLt_0, unsigned int Cols, myLight* pGrid_0, unsigned int GridRows, unsigned int GridCols );
    // give angle and index into rowVec and colVec. Writes rowVec[idx] and Rrot. same for D, Crot. false if idx too high
    bool Rotate( unsigned int idx, unsigned int& R, unsigned int& C, unsigned int& Rrot, unsigned int& Crot );
};

#endif // MYLIGHT_H
