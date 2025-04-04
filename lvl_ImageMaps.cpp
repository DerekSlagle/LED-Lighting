#include "lvl_ImageMaps.h"

bool lvl_ImageMaps::init()
{
    std::ifstream fin("include/levels/lvl_ImageMaps/init_data.txt");
    if( !fin ) return false;

    if( !spriteSheetImage::loadSpriteSheets( SSIvec, "include/levels/lvl_ImageMaps/SSI_fileList.txt") )
    {  std::cout << "\n no spritesSheets";
        return false; }

    float posX, posY;
    fin >> posX >> posY;
    Level::quitButt.setPosition( sf::Vector2f( posX, posY ) );
    button::RegisteredButtVec.push_back( &Level::quitButt );
    fin >> posX >> posY;
    Level::goto_MMButt.setPosition( sf::Vector2f( posX, posY ) );
    button::RegisteredButtVec.push_back( &Level::goto_MMButt );

    unsigned int rd, gn, bu;
    fin >> rd >> gn >> bu;
    Level::clearColor = sf::Color(rd,gn,bu);
    button::setHoverBoxColor( Level::clearColor );

    // images etc

    fin >> SSnum >> SetNum[0] >> SetNum[1] >> SetNum[2] >> SetNum[3];
    FrIdx[0] = 0;
    FrIdx[1] = 2;
    FrIdx[2] = 4;
    FrIdx[3] = 6;
    // validate
    if( SSnum >= SSIvec.size() ){ std::cout << "\nSSnum too high"; return false; }
 //   if( SetNum >= SSIvec[SSnum].numSets() ){ std::cout << "\nSetNum too high"; return false; }
 //   if( FrIdx >= SSIvec[SSnum].getFrCount( SetNum ) ){ std::cout << "\nFrIdx too high"; return false; }
    sf::IntRect SrcRect = SSIvec[SSnum].getFrRect( 0, SetNum[0] );
    sf::IntRect DestRect = SrcRect;// all frames same size
    DestRect.top = DestRect.left = 0;
    destImage.create( 4*DestRect.width, DestRect.height );
    ISB.init( SSIvec[SSnum].Img, SrcRect, destImage, DestRect );
    ISB.draw();// 1st animation
    ISB.srcRect = SSIvec[SSnum].getFrRect( 0, SetNum[1] );
    ISB.destRect.left = DestRect.width;
    ISB.draw();// 2nd animation
    ISB.srcRect = SSIvec[SSnum].getFrRect( 0, SetNum[2] );
    ISB.destRect.left += DestRect.width;
    ISB.draw();// 3rd animation
    ISB.srcRect = SSIvec[SSnum].getFrRect( 0, SetNum[3] );
    ISB.destRect.left += DestRect.width;
    ISB.draw();// 4th animation
    // reset to play 1st animation
    ISB.destRect.left = 0;

    // prepare texture
    destTexture.loadFromImage( destImage );
    // draw to window from texture
    sf::Vector2u txtSz = destTexture.getSize();
    destTxtRect[0].texCoords.x = 0;      // up left
    destTxtRect[0].texCoords.y = 0;
    destTxtRect[1].texCoords.x = txtSz.x;// up right
    destTxtRect[1].texCoords.y = 0;
    destTxtRect[2].texCoords.x = txtSz.x;// down right
    destTxtRect[2].texCoords.y = txtSz.y;
    destTxtRect[3].texCoords.x = 0;      // down left
    destTxtRect[3].texCoords.y = txtSz.y;

    // window position
    fin >> posX >> posY >> Scale;
    destTxtRect[0].position.x = posX;      // up left
    destTxtRect[0].position.y = posY;
    destTxtRect[1].position.x = posX + Scale*txtSz.x;// up right
    destTxtRect[1].position.y = posY;
    destTxtRect[2].position.x = posX + Scale*txtSz.x;// down right
    destTxtRect[2].position.y = posY + Scale*txtSz.y;
    destTxtRect[3].position.x = posX;      // down left
    destTxtRect[3].position.y = posY + Scale*txtSz.y;

    destTxtRect[0].color = destTxtRect[1].color = destTxtRect[2].color = destTxtRect[3].color = sf::Color::White;

    std::string fileName;
    fin >> fileName;
    if( !initTxtA( fileName.c_str() ) ) return false;

    fin >> fileName;
    if( !makeImageAndSSdata( fileName.c_str() ) ) return false;

    return true;
}

bool lvl_ImageMaps::initTxtA( const char* fileName )
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\n init_TxtA() no file"; return false; }

    unsigned int SSnum, SetNum, FrIdx0, FrIdxF;
    fin >> SSnum >> SetNum >> FrIdx0 >> FrIdxF;
    if( SSnum >= SSIvec.size() ){ std::cout << "\n init_TxtA() SSnum too high"; return false; }


    // prepare texture
    TxtA.loadFromImage( SSIvec[SSnum].Img );
    // draw to window from texture
 //   sf::Vector2u txtSz = TxtA.getSize();
    sf::IntRect txtRect = SSIvec[SSnum].getFrRect( FrIdx0, SetNum );
    TxtRectA[0].texCoords.x = txtRect.left;      // up left
    TxtRectA[0].texCoords.y = txtRect.top;
    TxtRectA[1].texCoords.x = txtRect.left + txtRect.width;// up right
    TxtRectA[1].texCoords.y = txtRect.top;
    TxtRectA[2].texCoords.x = txtRect.left + txtRect.width;// down right
    TxtRectA[2].texCoords.y = txtRect.top + txtRect.height;
    TxtRectA[3].texCoords.x = txtRect.left;      // down left
    TxtRectA[3].texCoords.y = txtRect.top + txtRect.height;

    // window position
    float posX, posY;
    fin >> posX >> posY >> ScaleA;
    TxtRectA[0].position.x = posX;      // up left
    TxtRectA[0].position.y = posY;
    TxtRectA[1].position.x = posX + ScaleA*txtRect.width;// up right
    TxtRectA[1].position.y = posY;
    TxtRectA[2].position.x = posX + ScaleA*txtRect.width;// down right
    TxtRectA[2].position.y = posY + ScaleA*txtRect.height;
    TxtRectA[3].position.x = posX;      // down left
    TxtRectA[3].position.y = posY + ScaleA*txtRect.height;

    TxtRectA[0].color = TxtRectA[1].color = TxtRectA[2].color = TxtRectA[3].color = sf::Color::White;

    findImageColors( SSIvec[SSnum], SetNum, FrIdx0, FrIdxF );
    SSItoFile( SSnum, SetNum, FrIdx0, FrIdxF );

    return true;
}

void lvl_ImageMaps::findImageColors( const spriteSheetImage& SSI, size_t SetNum, size_t FrIdx0, size_t FrIdxF )
{
    sf::IntRect Rect = SSI.getFrRect( 0, 0 );

    if( FrIdxF >= SSI.getFrCount(SetNum) ) FrIdxF = SSI.getFrCount(SetNum) - 1;

    for( size_t n = FrIdx0; n <= FrIdxF;  ++n )
    {
        Rect = SSI.getFrRect( n, SetNum );
        for( int y = 0; y < Rect.height; ++y )
        {
            for( int x = 0; x < Rect.width; ++x )
            {
                sf::Color clr = SSI.Img.getPixel( Rect.left + x, Rect.top + y );

                bool found = false;

                if( !colorVec.empty() )
                {
                    for( const sf::Color& C : colorVec )
                    {
                        if( C.r == clr.r && C.g == clr.g && C.b == clr.b && C.a == clr.a )
                        {
                            found = true;
                            break;
                        }
                    }
                }

                if( !found )
                {
                    colorVec.push_back( clr );
                    unsigned int rd = clr.r, gn = clr.g, bu = clr.b, ap = clr.a;
                    std::cout << '\n' << rd << ' ' << gn << ' ' << bu << ' ' << ap;
                }

            }// end for
        }// end for
        std::cout << "\n *** \n";

    }// end for


    // report
    std::cout << "\nfindImageColors() numColors = " << colorVec.size();
}

void lvl_ImageMaps::SSItoFile( size_t SSnum, size_t SetNum, size_t FrIdx0, size_t FrIdxF )const
{
    if( SSIvec.size() < SSnum ) return;
    const spriteSheetImage& SI = SSIvec[SSnum];
    std::ofstream fout( "include/levels/lvl_ImageMaps/SSI_asBits.txt" );


 //   fout << 64*SI.getFrCount(0) << '\n';

    fout << colorVec.size() << '\n';
    for( const sf::Color& C : colorVec )// list colors in these images
    {
        unsigned int rd = C.r, gn = C.g, bu = C.b;
        fout << rd << ' ' << gn << ' ' << bu << '\n';
    }

    // output number of assignments total for all frames
    sf::IntRect Rect = SI.getFrRect( 0, SetNum );
    fout << Rect.width*Rect.height*( 1 + FrIdxF  - FrIdx0 ) << '\n';

    for( size_t n = FrIdx0; n <= FrIdxF; ++n )
    {
        Rect = SI.getFrRect( n, SetNum );
        for( int y = 0; y < Rect.height; ++y )
        {
            for( int x = 0; x < Rect.width; ++x )
            {
                sf::Color clr = SI.Img.getPixel( Rect.left + x, Rect.top + y );
                for( size_t k = 0; k < colorVec.size(); ++k )
                {
          //          if( clr.r == colorVec[k].r && clr.g == colorVec[k].g && clr.b == colorVec[k].b && clr.a == colorVec[k].a )
                    if( clr.r == colorVec[k].r && clr.g == colorVec[k].g && clr.b == colorVec[k].b )
                    {
                        fout << k << ' ';
                        break;
                    }
                }

             //   if( clr.r == 0 && clr.g == 0 && clr.b == 0 )
            //        fout << "0 ";
            //    else
            //        fout << "1 ";
            }
            fout << '\n';
        }
        fout << '\n';
    }
}

bool lvl_ImageMaps::makeImageAndSSdata( const char* fileName )const
{
    std::ifstream fin( fileName );
    if( !fin ){ std::cout << "\n makeImageAndSSdata() no file"; return false; }

    bool Bounce;
    fin >> Bounce;
    if( Bounce ){ std::cout << "\n makeImageAndSSdata() Bouncing!"; return true; }

    unsigned int SSnum;
    fin >> SSnum;
    if( SSnum >= SSIvec.size() ) return false;
    // all frames to have same width and height
    const spriteSheetImage& SI = SSIvec[SSnum];

    sf::IntRect srcRect = SI.getFrRect( 0, 0 );
    int frW = srcRect.width, frH = srcRect.height;

    std::string fNameImg, fNameData;
    fin >> fNameImg >> fNameData;
    size_t Cols;
    fin >> Cols;


    std::ofstream fout( fNameData.c_str() );
    fout << fNameImg.c_str() << '\n';
    fout << SI.numSets() << '\n';
    // calculate rows
    size_t numFrames = 0;
    for( size_t n = 0; n < SI.numSets(); ++n )
        numFrames += SI.getFrCount(n);
    size_t Rows = numFrames/Cols;
    if( numFrames%Cols > 0 ) ++Rows;// 1 more partial

    sf::Image saveImage;
    saveImage.create( frW*Cols, frH*Rows );
    sf::IntRect destRect( 0, 0, frW, frH );
    for( size_t s = 0; s < SI.numSets(); ++s )
    {
        fout << SI.getFrCount(s) << '\n';
        for( size_t f = 0; f < SI.getFrCount(s); ++f )
        {
            fout << destRect.left << ' ' << destRect.top << ' ' << frW << ' ' << frH << '\n';
            srcRect = SI.getFrRect( f, s );
            if( srcRect.width != frW || srcRect.height != frH ) return false;



            for( int y = 0; y < frH; ++y )
            {
                for( int x = 0; x < frW; ++x )
                {
                    const sf::Color pxlClr = SI.Img.getPixel( srcRect.left + x, srcRect.top + y );
                    saveImage.setPixel( destRect.left + x, destRect.top + y, pxlClr );
                }
            }

            // destRect for next time
            if( destRect.left < (int)(frW*( Cols - 1 )) )
               destRect.left += frW;
            else
            {
                destRect.left = 0;
                destRect.top += frH;
            }
        }
    }

    saveImage.saveToFile( fNameImg.c_str() );

    return true;
}

bool lvl_ImageMaps::handleEvent( sf::Event& rEvent )
{
    return true;
}

void lvl_ImageMaps::update( float dt )
{
    if( ++delayCnt >= FrDelay )
    {
        delayCnt = 0;
        for( unsigned int k = 0; k < 4; ++k )
        {
            ISB.srcRect = SSIvec[SSnum].getFrRect( FrIdx[k], SetNum[k], true );
            ISB.destRect.left = k*ISB.destRect.width;
            ISB.draw();
        }

        destTexture.update( destImage );
    }
}

void lvl_ImageMaps::draw( sf::RenderTarget& RT ) const
{
    RT.draw( destTxtRect, 4, sf::Quads, &destTexture );
    RT.draw( TxtRectA, 4, sf::Quads, &TxtA );
 //   RT.draw( destTxtRect, 4, sf::Quads );
}
