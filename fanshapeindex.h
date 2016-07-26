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

#ifndef FANSHAPEINDEX_H
#define FANSHAPEINDEX_H
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>

#include <itkImageRegionIterator.h>
#include <itkConstNeighborhoodIterator.h>
#include "itkNeighborhoodAlgorithm.h"
#include <itkNeighborhoodIterator.h>
#include <otbImage.h>
#include <otbImageFileReader.h>
#include <otbImageFileWriter.h>
#include <otbImportImageFilter.h>
#include <fstream>

//for double images
typedef double PixelType;
typedef otb :: Image<PixelType, 2> ImageType;
typedef ImageType :: IndexType IndexType;
typedef otb :: ImageFileReader<ImageType> ReaderType;
typedef otb :: ImageFileWriter<ImageType> WriterType;
typedef itk :: ImageRegionIterator <ImageType> IteratorType;
typedef itk :: ImageRegionConstIterator < ImageType > ConstIteratorType;
typedef itk::NeighborhoodIterator<ImageType> NeighborhoodIteratorType;
typedef itk :: ImageRegionIterator < ImageType >  IteratorType;

//for int images
typedef int IntPixelType;
typedef otb :: Image<PixelType, 2> IntImageType;
typedef IntImageType :: IndexType IntIndexType;
typedef otb :: ImageFileReader<IntImageType> IntReaderType;
typedef otb :: ImageFileWriter<IntImageType> IntWriterType;
typedef itk :: ImageRegionIterator <IntImageType> IntIteratorType;
typedef itk :: ImageRegionConstIterator < IntImageType > IntConstIteratorType;
typedef itk::NeighborhoodIterator<IntImageType> IntNeighborhoodIteratorType;
typedef itk :: ImageRegionIterator < IntImageType >  IntIteratorType;


typedef itk::NeighborhoodAlgorithm :: ImageBoundaryFacesCalculator<ImageType> FaceCalculatorType;

class FanShapeIndex
{
    long nullValue;
    std :: map<int, double> fanShape, fanRadius, fanRadiusDirection,  apexDistanceFromCentroid;
    std :: map<int, NeighborhoodIteratorType :: IndexType> apexMap;
    std :: string classificationResult, inputBinary, inputDem, inputStrahler, inputStream;
    ImageType :: Pointer tmpImage, tmpApex;
    IntImageType :: Pointer fanId;
    ReaderType :: Pointer cRes, binary, dem, strahler;
    NeighborhoodIteratorType  *apexIt,  *binaryIt, *cResIt, *demIt, *strahlerIt;
    std :: vector <NeighborhoodIteratorType :: IndexType> fanCentroid;
    std :: vector <long int> fanArea;
    IntNeighborhoodIteratorType *fanIdIt;

    NeighborhoodIteratorType :: RadiusType radius;

    FaceCalculatorType::FaceListType::iterator innerAreaIt;


    void computeArea( long int*, int*, long int*, long int* );
    NeighborhoodIteratorType :: IndexType computeCentroid( NeighborhoodIteratorType :: IndexType*, NeighborhoodIteratorType*, int );
    void extendStreamDownwards();


public:

    void computeFanArea();
    void detectApex();
    void extendStream();
    FanShapeIndex();
    ~FanShapeIndex();
    FanShapeIndex( std:: string, std :: string, std :: string, std::string, long );
    void writeResult();

};

#endif // FANSHAPEINDEX_H
