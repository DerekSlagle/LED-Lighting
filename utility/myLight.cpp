#include "myLight.h"

myLight lightGroup::getLight( unsigned int Row, unsigned int Col )const
{
    if( !pLt0 || Row + 1 > rows || Col + 1 > cols ) return myLight();
  //  return *( pLt0 + Row*cols + Col );
    return *get_pLt( Row, Col );
}

// write clearLight to all
void lightGroup::Clear( myLight clearLight )const
{
    if( !pLt0 ) return;

    for( unsigned int r = 0; r < rows; ++r )
    {
        myLight* iter = get_pLt( r, 0 );// start of row
        for( unsigned int c = 0; c < cols; ++c )
            *( iter + c ) = clearLight;
    }
}

void lightGroup::copyFrom( const myLight& Lt_toCopy,  unsigned int toRow, unsigned int toCol  )const
{
    if( toRow > rows - 1 ) return;// out of bounds
    if( toCol > cols - 1 ) return;// out of bounds
    *( get_pLt( toRow, toCol ) ) = Lt_toCopy;
}

void lightGroup::copyFrom( const lightGroup& LG,  unsigned int toRow0, unsigned int toCol0  )const
{
    if( rows < toRow0 + LG.rows ) return;// will not fit
    if( cols < toCol0 + LG.cols ) return;// will not fit

    for( unsigned int r = 0; r < LG.rows; ++r )
    {
        myLight* iterThis = get_pLt( toRow0 + r, toCol0 );// start of target row
        const myLight* iterLG = LG.get_pLt( r, 0 );// start of target row
        for( unsigned int c = 0; c < LG.cols; ++c )
            *( iterThis + c ) = *( iterLG + c );
    }
}

void lightGroup::shiftLeft()const
{
    if( cols < 2 ) return;

    for( unsigned int r = 0; r < rows; ++r )
    {
        myLight Lt0 = *get_pLt( r, 0 );
        for( unsigned int c = 0; c + 1 < cols; ++c )
            *get_pLt( r, c ) = *get_pLt( r, c + 1 );// copy back

        *get_pLt( r, cols - 1 ) = Lt0;
    }
}

void lightGroup::shiftLeft( lightGroup& buffGrp, unsigned int buffCol, unsigned int bcShft )const
{
    if( cols < 2 ) return;
    if( buffCol > buffGrp.cols - 1 ) return;// no data left

    for( unsigned int r = 0; r < rows && r < buffGrp.rows + bcShft; ++r )
    {
        for( unsigned int c = 0; c + 1 < cols; ++c )
            *get_pLt( r, c ) = *get_pLt( r, c + 1 );// copy back

        *get_pLt( r + bcShft, cols - 1 ) = *buffGrp.get_pLt( r, buffCol );
    }
}

void lightGroup::shiftRight()const
{
    if( cols < 2 ) return;

    for( unsigned int r = 0; r < rows; ++r )
    {
        myLight Lt0 = *get_pLt( r,cols - 1 );// save last for 1st

        for( unsigned int c = cols - 1; c > 0; --c )
        {
            *get_pLt( r, c ) = *get_pLt( r, c - 1 );// copy forward
        }
        *get_pLt( r, 0 ) = Lt0;//  last to 1st
    }
}

void lightGroup::shiftUp()const
{
    if( rows < 2 ) return;

    // copy columns upward from rows - 1 toward row = 1 -> 0. save 1st for last
    for( unsigned int c = 0; c < cols; ++c )
    {
        myLight Lt0 = *get_pLt( 0, c );// save 1st for last. This wraps column c

        for( unsigned int r = 0; r + 1 < rows; ++r )
            *get_pLt( r, c ) = *get_pLt( r + 1, c );// copy back

        *get_pLt( rows - 1, c ) = Lt0;//  1st to 1ast
    }
}

void lightGroup::shiftDown()const
{
    if( rows < 2 ) return;

    // for each column, shift it downward
    for( unsigned int c = 0; c < cols; ++c )
    {
        // save last for 1st. This wraps column c
        myLight LtF = *get_pLt( rows - 1, c );

        // copy down 1 row
        for( unsigned int r = rows - 1; r > 0; --r )
            *get_pLt( r, c ) = *get_pLt( r - 1, c );// copy down to higher row

        *get_pLt( 0, c ) = LtF;// last to 1st
    }
}

void lightGroup::scroll( char Dir, float dt )// writes to tElap
{
    tElap += dt;
    if( tElap > tLimit )// shift
    {
        tElap = 0.0f;
        if( Dir == 'L' ) shiftLeft();
        else if( Dir == 'R' ) shiftRight();
        else if( Dir == 'U' ) shiftUp();
        else if( Dir == 'D' ) shiftDown();
    }
}

void lightGroup::setBrightness( float Brightness )const
{
    for( unsigned int r = 0; r < rows; ++r )
    {
        myLight* pRow = get_pLt( r, 0 );
        for( unsigned int c = 0; c < cols; ++c )
            ( pRow + c )->brightness = Brightness;
    }
}

void lightGroup::drawFromFunction( std::function<float(float,float)> Yxt, myLight LtF )const
{
    if( !(Yxt && pLt0) ) return;
    for( unsigned int c = 0; c < cols; ++c )
    {
        int r = Yxt( c, tElap );// in case < 0
        if( r >= 0 && r < (int)rows ) *get_pLt( r, c ) = LtF;
    }
}

void lightGroup::drawFrom_istream( std::istream& is )const
{
    if( !( pLt0 && is.good() ) ) return;
    myLight temp;
    for( unsigned int r = 0; r < rows; ++r )
    {
        myLight* pRow = get_pLt( r, 0 );
        for( unsigned int c = 0; c < cols; ++c )
        {
            if( temp.init(is) )
                *( pRow + c ) = temp;
            else
                return;// no more data
        }

    }
}

void lightGroup::drawFrom_sceGroup( const lightGroup& sceGroup )const// already bound to targetGroup
{
    if( !pLt0 ) return;

    for( unsigned int r = 0; r < rows && r < sceGroup.rows; ++r )
    {
        myLight* pRow = get_pLt( r, 0 );
     //   const myLight* pRowSce = sceGroup.get_pLt( r, 0 );
        for( unsigned int c = 0; c < cols && c < sceGroup.cols; ++c )
        {
             //   *( pRow + c ) = *( pRowSce + c );
                *( pRow + c ) = sceGroup.getLight(r,c);
        }

    }
}


//*** lightGroupGrid methods ***
void lightGroupGrid::setPosition( unsigned int row0, unsigned int col0 )
{
    pLt0 = pGrid0 + gridCols*row0 + col0;
}

lightGroupGrid& lightGroupGrid::operator=( const lightGroupGrid& GG )
{
    pGrid0 = GG.pGrid0;
    pLt0 = GG.pLt0;
    rows = GG.rows;
    cols = GG.cols;
    gridCols = GG.gridCols;
    gridRows = GG.gridRows;
    tElap = GG.tElap;
    tLimit = GG.tLimit;
    return *this;
}

lightGroupGrid& lightGroupGrid::operator=( const lightGroup& LtGrp )
{
    pGrid0 = pLt0 = LtGrp.pLt0;
    gridRows = rows = LtGrp.rows;
    gridCols = cols = LtGrp.cols;
    tElap = 0.0f;
    return *this;
}

// space frame r0 rows and c0 cols from grid group. false if off bounding grid
bool lightGroupGrid::makeFrame( std::vector<myLight*>& pLtVec, unsigned int r0, unsigned int c0 )const// writes to LtGp
{
    if( r0 < 1 || c0 < 1 ) return false;

    unsigned int Row0 = getRow0();
    unsigned int Col0 = getCol0();
    if( Row0 < r0 || Col0 < c0 ) return false;// out of bounds

    unsigned int endRow = rows + 2*r0, endCol = cols + 2*c0;
    pLtVec.clear();
    pLtVec.reserve( 2*( endRow + endCol ) - 4 );
    std::cout << "\n pLtVec.cap = " << pLtVec.capacity();

    myLight* pLt = pLt0 - r0*( gridCols ) - c0;// up 2 rows and left 2
    for( unsigned int k = 0; k < endCol; ++k )
        pLtVec.push_back( pLt + k );// top edge

    pLt = pLtVec.back();
    for( unsigned int k = 1; k < endRow; ++k )
        pLtVec.push_back( pLt + k*gridCols );// right side

    pLt = pLtVec.back();
    for( unsigned int k = 1; k < endCol; ++k )
        pLtVec.push_back( pLt - k );// bottom edge

    pLt = pLtVec.back();
    for( unsigned int k = 1; k + 1 < endRow; ++k )// exclude last. it was pushed back first
        pLtVec.push_back( pLt - k*gridCols );// left side

    std::cout << "\n pLtVec.sz = " << pLtVec.size();

    return true;
}

// groupGridMove
void groupGridMove::init( myLight* pLt, unsigned int Rows, unsigned int Cols, myLight* pGrid, unsigned int GridRows, unsigned int GridCols )
{
    lightGroupGrid::init( pLt, Rows, Cols, pGrid, GridRows, GridCols );

    unsigned int numLts = rows*cols;
    swatchVec.reserve( rows*cols );
    for( unsigned int k = 0; k < numLts; ++k )
        swatchVec.push_back( *( pLt0 + (k/cols)*gridCols + k%cols ) );
}

void groupGridMove::setPosition( unsigned int row0, unsigned int col0 )
{
    unsigned int numLts = rows*cols;

    // restore background
    for( unsigned int k = 0; k < numLts; ++k )
        *( pLt0 + (k/cols)*gridCols + k%cols ) = swatchVec[k];

    // move
    lightGroupGrid::setPosition( row0, col0 );

    // save new swatch
    for( unsigned int k = 0; k < numLts; ++k )
        swatchVec[k] = *( pLt0 + (k/cols)*gridCols + k%cols );
}

void groupGridMove::updateSwatch()
{
    unsigned int numLts = rows*cols;
    for( unsigned int k = 0; k < numLts; ++k )
        swatchVec[k] = *( pLt0 + (k/cols)*gridCols + k%cols );
}

// type groupGridRotate
// fill rowVec and colVec. rows = cols only
void groupGridRotate::init( myLight* pLt_0, unsigned int Cols, myLight* pGrid_0, unsigned int GridRows, unsigned int GridCols )
{
    lightGroupGrid::init( pLt_0, Cols, Cols, pGrid_0, GridRows, GridCols );
    float Cc = cols/2;
    float R = Cc - 0.4f;
  //  std::cout << "\n r,c pairs: ";
    for( unsigned int r = 0; r < rows; ++r )
    {
        for( unsigned int c = 0; c < cols; ++c )
        {
            float dSq = ( (float)c - Cc )*( (float)c - Cc ) + ( (float)r - Cc )*( (float)r - Cc );
            if( dSq <= R*R )
            {
                rowVec.push_back(r);
                colVec.push_back(c);
            //    std::cout << r << ", " << c << ", ";
            }
        }
    }

    std::cout << "\n numPairs = " << rowVec.size();
}

    // give angle and index into rowVec and colVec. Writes rowVec[idx] and Rrot. same for D, Crot
bool groupGridRotate::Rotate( unsigned int idx, unsigned int& R, unsigned int& C, unsigned int& Rrot, unsigned int& Crot )
{
    if( idx >= rowVec.size() ) return false;

    float sNa = sinf(rotAng), cSa = cosf(rotAng);
    float Rf = rowVec[idx], Cf = colVec[idx];
    C = Cf;
    R = Rf;
    float Cc = cols/2;
    Rf -= Cc;
    Cf -= Cc;
    Rrot = static_cast<unsigned int>( Rf*cSa - Cf*sNa + Cc );
    Crot = static_cast<unsigned int>( Rf*sNa + Cf*cSa + Cc );
    // will it work?
    return true;
}

