/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fanshapeindex.h"

struct Pnt {
    NeighborhoodIteratorType :: IndexType index;
    double height;
    Pnt(double h, NeighborhoodIteratorType :: IndexType idx): height(h), index(idx) {}

    bool operator > (const Pnt& pp)  const{
        return (this->height >= pp.height);
    }
};

void FanShapeIndex :: computeArea(long int *area, int *id, long int *xc, long int *yc) {

    (*area)++;
    *xc += fanIdIt->GetIndex()[0];
    *yc += fanIdIt->GetIndex()[1];
    fanIdIt->SetCenterPixel(*id);
    //std :: cout <<flagFanIt->GetIndex() <<"\t" << binaryIt->GetIndex() << "\t" << apexIt->GetIndex() << "\n";


    NeighborhoodIteratorType :: IndexType * tmpIdx;
    tmpIdx = new NeighborhoodIteratorType :: IndexType( fanIdIt->GetIndex() );
    int *i  = 0;
    i = new int;

    for ( *i = 0;  *i < 9;  (*i)++ )  {

        if ( (  binaryIt->IndexInBounds(*i) ) && ( binaryIt->GetPixel(*i) == 1  ) && ( fanIdIt->GetPixel(*i) == 0 ) ) {
            binaryIt->SetLocation( fanIdIt->GetIndex(*i) );
            fanIdIt->SetLocation( fanIdIt->GetIndex(*i) );

            computeArea( area, id, xc, yc );

            binaryIt->SetLocation( *tmpIdx );
            fanIdIt->SetLocation( *tmpIdx );
        }
    }

    delete i;
    i = NULL;
    delete tmpIdx;
    tmpIdx = NULL;

}

void FanShapeIndex :: computeFanArea() {


    long int area = 0;
    int id = 1;

    for ( binaryIt->GoToBegin(),  fanIdIt->GoToBegin(), apexIt->GoToBegin(); !fanIdIt->IsAtEnd(); binaryIt->operator ++(), apexIt->operator ++(), fanIdIt->operator ++() ) {

        if ( ( binaryIt->GetCenterPixel() == 1  ) && ( fanIdIt->GetCenterPixel() == 0 ) ) {//an alluvial fan was found
            area = 0;
            long int xc =0, yc = 0; //alluvial fan centroid coordinates
            computeArea( &area, &id, &xc, &yc);

            xc /= area;
            yc /= area;
            id++;
            IntNeighborhoodIteratorType :: IndexType kk;
            kk[0] = xc;
            kk[1] = yc;
            fanCentroid.push_back(kk);
            fanArea.push_back(area);
        }
    }
}


void FanShapeIndex :: detectApex() {
    std :: vector<Pnt> streams;
    for (demIt->GoToBegin(), cResIt->GoToBegin(); !cResIt->IsAtEnd(); demIt->operator ++(), cResIt->operator ++() ) {
        if (cResIt->GetCenterPixel() != nullValue) {
            streams.push_back( Pnt (demIt->GetCenterPixel(), demIt->GetIndex() ) );
        }
    }
    //sorting based on height
    __gnu_parallel::sort( streams.begin(), streams.end(), std::greater<Pnt>() );

    for (register int i = 0; i < streams.size(); i++) {
        //checking if this is the edge of the stream
        cResIt->SetLocation( streams[i].index );
        //std :: cout << streams[i].index <<"\n";

        bool edge = true;
        int cntEdge = 0;

        for (register int p = 0; p < 9; p++ )
            if ( ( cResIt->GetPixel(p) == cResIt->GetCenterPixel() ) )
                cntEdge++;


        if ( cntEdge > 2 )
            edge = false;


        if ( edge ) {
            binaryIt->SetLocation( streams[i].index );
            apexIt->SetLocation( streams[i].index );
            NeighborhoodIteratorType :: IndexType apexIdx;
            bool apexMark = false;

            for (register int l = 1; l < 9 && apexMark == false; l+=2) {
                if ( binaryIt->GetPixel(l) > 0 ) {
                    apexMark = true;
                    apexIdx = apexIt->GetIndex(l);
                }
            }

            if ( !apexMark ) {
                for (register int l = 0; l < 9 && apexMark == false; l+=2) {
                    if ( binaryIt->GetPixel(l) > 0 ) {
                        apexMark = true;
                        apexIdx = apexIt->GetIndex(l);
                    }
                }
            }

            //checking if this is the minimum distance of the stream with the centroid

            if (apexMark) {
                fanIdIt->SetLocation( apexIdx );
                int curFanId =   fanIdIt->GetCenterPixel();
                NeighborhoodIteratorType :: OffsetType diff =  fanCentroid[ curFanId-1 ] - apexIdx;

                double dist = std :: sqrt( std :: pow( diff[0],2 ) + std::pow( diff[1], 2 ) );
                if ( apexDistanceFromCentroid.find(curFanId) == apexDistanceFromCentroid.end()  )
                    apexDistanceFromCentroid[ curFanId ] = dist;
                else if ( apexDistanceFromCentroid[ curFanId ] > dist )
                    apexDistanceFromCentroid[ curFanId ] = dist;
                else if (apexDistanceFromCentroid [curFanId ] < dist )
                    apexMark = false;
            }

            //this point is recognized as an apex
            if ( apexMark ) {
                fanIdIt->SetLocation( apexIdx );
                //getting centroid for the current fan
                apexMap [ fanIdIt->GetCenterPixel() ] = apexIdx;
            }
        }
    }

    for (register int curFanId = 1; curFanId <= apexMap.size(); curFanId++) {

        NeighborhoodIteratorType :: IndexType apexIdx = apexMap [ curFanId ];
        std :: cout <<"apex index = " << apexIdx <<"\n";

        //computing semi-circle radius
        double angle = std :: atan2( fanCentroid[ curFanId-1 ][0] - apexIdx[0], fanCentroid[ curFanId-1 ][1] - apexIdx[1] );

        double dist;
        int step;

        NeighborhoodIteratorType :: IndexType end, oldend;
        bool stop = false;
        for (int step = 0; stop == false; step++) {
            oldend = end;
            end[0] = fanCentroid[ curFanId-1 ][0] + 0.01*step*sin(angle);
            end[1] = fanCentroid[ curFanId-1 ][1] + 0.01*step*cos(angle);
            binaryIt->SetLocation(end);

            if (binaryIt->GetCenterPixel() == 0 ) {
                stop = true;
                end = oldend;
            }
        }
        double radius = sqrt ( pow( end[0] - apexIdx[0], 2 ) +pow ( end[1] - apexIdx[1], 2) );
        std :: vector < NeighborhoodIteratorType :: IndexType > delIdx;

        //computing the respective center angle
        double fi = 2.0*fanArea[ curFanId - 1 ] / pow( radius, 2 );

        NeighborhoodIteratorType :: IndexType tmp;
        for ( double ang = angle - fi/2.0; ang < angle + fi /2.0; ang +=0.00003  )
            for ( dist = 0, step = 1; dist < radius; dist += 0.1*step ) {
                tmp[0] = apexIdx[0] + dist*sin(ang);
                tmp[1] = apexIdx[1] + dist*cos(ang);
                apexIt->SetLocation( tmp );
                if ( (apexIt->InBounds()) && ( apexIt->GetCenterPixel() != curFanId ) )  {
                    apexIt->SetCenterPixel( curFanId );
                    delIdx.push_back(tmp);
                }
            }

        float commonArea = 0;
        for (register unsigned int pp = 0; pp < delIdx.size(); pp++) {
            apexIt->SetLocation( delIdx[pp] );
            fanIdIt->SetLocation( delIdx[pp] );
            if ( apexIt->GetCenterPixel()  == fanIdIt->GetCenterPixel()  )
                commonArea++;
        }
        double val = commonArea /( commonArea + (fanArea[ curFanId -1  ] - commonArea)*2  );

        fanShape[ curFanId ] = val;

        WriterType :: Pointer writer;
        writer = WriterType :: New();
        std :: stringstream ss;
        ss << curFanId <<".tif";
        writer->SetFileName( ss.str() );
        writer->SetInput(tmpApex);
        writer->Update();
        tmpApex->FillBuffer(-1);
        delIdx.clear();


    }
}

NeighborhoodIteratorType :: IndexType FanShapeIndex :: computeCentroid ( NeighborhoodIteratorType :: IndexType *idx, NeighborhoodIteratorType *flagFanIt, int id) {
    binaryIt->SetLocation(*idx);
    flagFanIt->SetLocation( *idx );
    flagFanIt->SetCenterPixel(1);
    apexIt->SetLocation( *idx );
    apexIt->SetCenterPixel( id );

    int *i;
    i = new int (0);
    for ( *i = 0;  *i  < 9; (*i)++ ) {
        if (*i != 4 && flagFanIt->GetCenterPixel() != 1 && binaryIt->GetPixel(*i) == 1 ) {
            NeighborhoodIteratorType :: IndexType *tmpIdx = new NeighborhoodIteratorType :: IndexType (  apexIt->GetIndex(*i) );
            computeCentroid(tmpIdx, flagFanIt, id);
            flagFanIt->SetLocation( *idx );
            binaryIt->SetLocation(*idx);
        }
        //std :: cout <<*i <<"\n";

    }
    delete i;
}





int cnt = 0;
void FanShapeIndex :: extendStreamDownwards() {
    if ( cResIt->InBounds() )  {
        strahlerIt->SetLocation( cResIt->GetIndex() );
        binaryIt->SetLocation( cResIt->GetIndex() );
        int *i;
        i = new int(0);
        for (*i = 0; *i < 9; (*i)++ ) {
            if ( (strahlerIt->GetPixel( *i ) >= strahlerIt->GetCenterPixel() )  && (  strahlerIt->GetPixel(*i) < 20  )  && ( cResIt->GetPixel( *i ) == nullValue ) && (binaryIt->GetPixel(*i) == 0 )  && ( cnt < 40 )  ) {
                cResIt->SetPixel(*i, cResIt->GetCenterPixel()  );
                NeighborhoodIteratorType :: IndexType *tmpIdx = new NeighborhoodIteratorType :: IndexType ( cResIt->GetIndex() );
                cResIt->SetLocation( cResIt->GetIndex( *i ) );
                cnt++;
                extendStreamDownwards();
                cResIt->SetLocation(*tmpIdx);
                delete tmpIdx;
                tmpIdx = NULL;
            }
        }
        i = NULL;
        delete i;
    }
    cnt = 0;
}



void FanShapeIndex :: extendStream() {
    for ( cResIt->GoToBegin(); !cResIt->IsAtEnd(); cResIt->operator ++() ) {
        if( cResIt->GetCenterPixel() != nullValue ) {
            extendStreamDownwards();
        }
    }
}


FanShapeIndex :: FanShapeIndex() {}

FanShapeIndex :: FanShapeIndex(std :: string cres, std :: string bn, std:: string dm, std :: string sthl, long nl ):classificationResult(cres), inputBinary(bn), inputDem(dm), inputStrahler(sthl), nullValue( nl )  {
    std :: cout <<"nullvalue " <<nullValue <<"\n";
    binary = ReaderType :: New();
    cRes = ReaderType :: New();
    dem = ReaderType :: New();
    strahler = ReaderType :: New();

    //reading input data
    binary->SetFileName(inputBinary);
    cRes->SetFileName(classificationResult);
    dem->SetFileName(inputDem);
    strahler->SetFileName(inputStrahler);

    cRes->Update();
    binary->Update();
    dem->Update();
    strahler->Update();

    //creating area for examination

    radius.Fill(1);

    FaceCalculatorType faceCalculator;
    FaceCalculatorType::FaceListType faceList;

    faceList = faceCalculator(binary->GetOutput(), binary->GetOutput()->GetLargestPossibleRegion(), radius);
    innerAreaIt = faceList.begin();

    //creating iterators
    tmpImage = ImageType :: New();
    tmpImage = cRes->GetOutput();
    //mpImage->SetRegions( reader->GetOutput()->GetLargestPossibleRegion() );
    tmpImage->CopyInformation(cRes->GetOutput());

    tmpApex = ImageType :: New();
    tmpApex->CopyInformation(binary->GetOutput());
    tmpApex->SetRegions( binary->GetOutput()->GetRequestedRegion() );
    tmpApex->Allocate();

    fanId = IntImageType :: New();
    fanId->CopyInformation( binary->GetOutput() );
    fanId->SetRegions( binary->GetOutput()->GetRequestedRegion() );
    fanId->Allocate();


    apexIt = new NeighborhoodIteratorType(radius, tmpApex, *innerAreaIt);
    binaryIt = new NeighborhoodIteratorType(radius, binary->GetOutput(), *innerAreaIt);
    cResIt = new NeighborhoodIteratorType(radius, tmpImage, *innerAreaIt);
    demIt = new NeighborhoodIteratorType(radius, dem->GetOutput(), *innerAreaIt);
    strahlerIt = new NeighborhoodIteratorType(radius, strahler->GetOutput(), *innerAreaIt);
    fanIdIt = new IntNeighborhoodIteratorType(radius, fanId, *innerAreaIt);

    tmpApex->FillBuffer(-1);

}
FanShapeIndex :: ~FanShapeIndex() {
    delete  apexIt;
    apexIt = NULL;

    delete binaryIt;
    binaryIt = NULL;

    delete cResIt;
    cResIt = NULL;

    delete demIt;
    demIt = NULL;

    delete strahlerIt;
    strahlerIt = NULL;

    delete fanIdIt;
    fanIdIt = NULL;
}


void FanShapeIndex :: writeResult() {

    WriterType :: Pointer writer;
    writer = WriterType :: New();
    /*
    writer->SetFileName( output );
    writer->SetInput( tmpImage );
    writer->Update();
    */

    //writing fan ids
    writer->SetFileName("fanid.tif");
    writer->SetInput(fanId);
    writer->Update();

    //writing fan shape index
    std :: ofstream file;
    file.open("fanShape.txt");

    std :: map<int,double>::iterator mapIt;

    for ( mapIt=fanShape.begin();  mapIt != fanShape.end(); mapIt++  ){
        file << (*mapIt).first <<"\t"<<(*mapIt).second <<"\n";
    }
    file.close();

}
