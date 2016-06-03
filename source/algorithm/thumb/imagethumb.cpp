#include "imagethumb.h"

#define ThumbWidth 200
#define Stretch 1

bool CreateSquareThumb( const string input , const string output , int bandID)  {

    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
    GDALDataset *pDataset = (GDALDataset *) GDALOpen( input.c_str(), GA_ReadOnly );
    if( NULL == pDataset )
        return false;

    int nSamples = ((GDALDataset *)pDataset)->GetRasterXSize();
    int nLines = ((GDALDataset *)pDataset)->GetRasterYSize();
    int nBands = ((GDALDataset *)pDataset)->GetRasterCount();

    if(bandID > nBands) {
        cerr<<"bandID error."<<endl;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }

    int i , j ;
    float *pBuffer = new(std::nothrow) float[ nSamples ];
    if ( NULL == pBuffer  ) {
        cerr<<"memory error."<<endl;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }

    GDALRasterBand *pBandRead = ((GDALDataset *)pDataset)->GetRasterBand( bandID );
    double dMinMax[2]={0.0,0.0};
    if (pBandRead->ComputeRasterMinMax(false,dMinMax) != CE_None) {
        cerr<<"compute minmax error."<<endl;
        delete[] pBuffer;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }

    float minV,maxV;
    minV = dMinMax[0];
    maxV = dMinMax[1];

    GDALDriver * pDriver = (GDALDriver*)GDALGetDriverByName( "MEM" );
    if( NULL == pDriver ) {
        cerr<<"create MEM driver error."<<endl;
        delete[] pBuffer;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }

    int stepSize = max(nSamples,nLines)>=ThumbWidth? max(nSamples,nLines)/ThumbWidth:max(nSamples,nLines);
    int outSample = int(nSamples / stepSize) + ((nSamples % stepSize) == 0 ? 0:1);
    int outLine = int(nLines / stepSize) + ((nLines % stepSize) == 0 ? 0:1 );
    int out = max(outSample,outLine);

    GDALDataset* pDSOut = pDriver->Create( "tmpFile",out,out,1,GDT_Byte,NULL);
    if( NULL == pDSOut ) {
        cerr<<"create tmpfile error."<<endl;
        delete[] pBuffer;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }

    unsigned char* pBufferOut = new(std::nothrow) unsigned char[ out ];
    if ( NULL == pBufferOut ) {
        cerr<<"memory error."<<endl;
        delete[] pBuffer;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }
    for (i=0;i<out;i++)
        pBufferOut[i] = 255;

    GDALRasterBand *pBandWrite = ((GDALDataset *)pDSOut)->GetRasterBand( 1 );
    if( NULL == pBandWrite ) {
        cerr<<"get raster band error."<<endl;
        delete[] pBuffer;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }
    int offset;
    int offsetY = 0;
    for (i=0;i<(out-outLine)/2;i++) {
        if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut ) != CE_None ) {
            cerr<<"write image error."<<endl;
            delete[] pBuffer;
            delete[] pBufferOut;
            GDALClose( (GDALDatasetH) pDataset );
            GDALClose( (GDALDatasetH) pDSOut );
            return false;
        }
        offsetY++;
    }
    for ( i = 0 ; i < nLines ; i+= stepSize ) {
        if( pBandRead->RasterIO(GF_Read,0,i,nSamples,1,pBuffer,nSamples,1,GDT_Float32,0,0 ) != CE_None ) {
            cerr<<"read image error."<<endl;
            delete[] pBuffer;
            delete[] pBufferOut;
            GDALClose( (GDALDatasetH) pDataset );
            GDALClose( (GDALDatasetH) pDSOut );
            return false;
        }

        offset = (out-outSample)/2;
        for( j = 0 ; j < nSamples ; j+= stepSize ) {
            pBufferOut[ offset ] = (unsigned char)(( pBuffer[ j ] - minV ) * 256 *Stretch/ ( maxV - minV + 1 ));
            offset++;
        }
        if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut ) != CE_None ) {
            cerr<<"write image error."<<endl;
            delete[] pBuffer;
            delete[] pBufferOut;
            GDALClose( (GDALDatasetH) pDataset );
            GDALClose( (GDALDatasetH) pDSOut );
            return false;
        }
        offsetY++;
    }
    for (i=0;i<out;i++)
        pBufferOut[i] = 255;

    for (i=offsetY;i<out;i++) {
        if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut ) != CE_None ) {
            cerr<<"write image error."<<endl;
            delete[] pBuffer;
            delete[] pBufferOut;
            GDALClose( (GDALDatasetH) pDataset );
            GDALClose( (GDALDatasetH) pDSOut );
            return false;
        }
        offsetY++;
    }
    delete[] pBuffer;
    delete[] pBufferOut;

    GDALDriver * poDriver = GetGDALDriverManager()->GetDriverByName("JPEG");
    if( NULL == poDriver ) {
        cerr<<"get driver error."<<endl;
        GDALClose( (GDALDatasetH) pDataset );
        GDALClose( (GDALDatasetH) pDSOut );
        return false;
    }
    GDALDataset * poDataset=poDriver->CreateCopy(output.c_str(),pDSOut,false,NULL,NULL,NULL);
    if( NULL == poDataset ) {
        cerr<<"driver copy error."<<endl;
        GDALClose( (GDALDatasetH) pDataset );
        GDALClose( (GDALDatasetH) pDSOut );
        return false;
    }
    GDALPamDataset *pamDs=dynamic_cast<GDALPamDataset*>(poDataset);
    if(NULL == pamDs ) {
        int pamFlags=pamDs->GetPamFlags();
        pamFlags|=GPF_NOSAVE;
        pamDs->SetPamFlags(pamFlags);
    }

    if( NULL == pDSOut )
        GDALClose( (GDALDatasetH) pDSOut );
    pDriver->Delete( "tmpFile" );
    if( NULL == pDataset )
        GDALClose( (GDALDatasetH) pDataset );
    if( NULL == poDataset )
        GDALClose( (GDALDatasetH) poDataset );
    return true;
}

bool CreateSquareThumb( const string input ,const string output, int redBand,int greenBand, int blueBand)  {

    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");

    GDALDataset *pDataset = (GDALDataset *) GDALOpen( input.c_str(), GA_ReadOnly );

    if( NULL == pDataset )
        return false;

    int nSamples = ((GDALDataset *)pDataset)->GetRasterXSize();
    int nLines = ((GDALDataset *)pDataset)->GetRasterYSize();
    int nBands = ((GDALDataset *)pDataset)->GetRasterCount();
    int rgbBand[] = {redBand,greenBand,blueBand};
    int i,j,k;
    if(redBand > nBands || greenBand > nBands || blueBand > nBands) {
        cerr<<"bandID error."<<endl;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }
    float minV[3]={0},maxV[3]={0};
    float *pBuffer = new(std::nothrow) float[ nSamples ];
    if ( NULL == pBuffer  ) {
        cerr<<"memory error."<<endl;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }
    double dMinMax[2]={0.0,0.0};
    for( k=0 ; k<3 ; k++ ) {
        GDALRasterBand *pBandRead = ((GDALDataset *)pDataset)->GetRasterBand( rgbBand[k] );//ªÒ»°÷∏∂®≤®∂Œµƒ ˝æ›
        if (pBandRead->ComputeRasterMinMax(false,dMinMax) != CE_None) {
            cerr<<"compute minmax error."<<endl;
            delete[] pBuffer;
            GDALClose( (GDALDatasetH) pDataset );
            return false;
        }

        minV[k] = dMinMax[0];
        maxV[k] = dMinMax[1];
    }

    GDALDriver * pDriver = (GDALDriver*)GDALGetDriverByName( "MEM" );
    if( NULL == pDriver ) {
        cerr<<"create MEM driver error."<<endl;
        delete[] pBuffer;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }
    int stepSize = max(nSamples,nLines)>=ThumbWidth? max(nSamples,nLines)/ThumbWidth:max(nSamples,nLines);
    int outSample = int(nSamples / stepSize) + ((nSamples % stepSize) == 0 ? 0:1);
    int outLine = int(nLines / stepSize) + ((nLines % stepSize) == 0 ? 0:1 );
    int out = max(outSample,outLine);

    GDALDataset* pDSOut = pDriver->Create( "tmpFile",out,out,3,GDT_Byte,NULL);
    if( NULL == pDSOut ) {
        cerr<<"create tmpfile error."<<endl;
        delete[] pBuffer;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }

    unsigned char* pBufferOut = new(std::nothrow) unsigned char[ out ];
    if (NULL == pBufferOut ) {
        cerr<<"memory error."<<endl;
        delete[] pBuffer;
        GDALClose( (GDALDatasetH) pDataset );
        return false;
    }
    for (i=0;i<out;i++)
        pBufferOut[i] = 255;

    GDALRasterBand *pBandWrite=NULL;
    GDALRasterBand *pBandRead=NULL;

    for( k=0 ; k<3 ; k++ ) {
        pBandWrite = ((GDALDataset *)pDSOut)->GetRasterBand( k +1 );
        pBandRead = ((GDALDataset *)pDataset)->GetRasterBand( rgbBand[k] );
        if( NULL == pBandWrite || pBandRead == NULL ) {
            cerr<<"get raster band error."<<endl;
            delete[] pBuffer;
            GDALClose( (GDALDatasetH) pDataset );
            return false;
        }

        int offset;
        int offsetY = 0;
        for (i=0;i<(out-outLine)/2;i++) {
            if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut ) != CE_None ) {
                cerr<<"write image error."<<endl;
                delete[] pBuffer;
                delete[] pBufferOut;
                GDALClose( (GDALDatasetH) pDataset );
                GDALClose( (GDALDatasetH) pDSOut );
                return false;
            }
            offsetY++;
        }
        for ( i = 0 ; i < nLines ; i+= stepSize ) {
            if( pBandRead->RasterIO(GF_Read,0,i,nSamples,1,pBuffer,nSamples,1,GDT_Float32,0,0 ) != CE_None ) {
                cerr<<"read image error."<<endl;
                delete[] pBuffer;
                delete[] pBufferOut;
                GDALClose( (GDALDatasetH) pDataset );
                GDALClose( (GDALDatasetH) pDSOut );
                return false;
            }

            offset = (out-outSample)/2;
            for( j = 0 ; j < nSamples ; j+= stepSize ) {
                pBufferOut[ offset ] = (unsigned char)(( pBuffer[ j ] - minV[k] ) * 256 *Stretch/ ( maxV[k] - minV[k] + 1 ));
                offset++;
            }
            if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut )!= CE_None ) {
                cerr<<"write image error."<<endl;
                delete[] pBuffer;
                delete[] pBufferOut;
                GDALClose( (GDALDatasetH) pDataset );
                GDALClose( (GDALDatasetH) pDSOut );
                return false;
            }
            offsetY++;
        }
        for (i=0;i<out;i++)
            pBufferOut[i] = 255;
        for (i=offsetY;i<out;i++) {
            if( pBandWrite->WriteBlock( 0 , offsetY , pBufferOut ) != CE_None ) {
                cerr<<"write image error."<<endl;
                delete[] pBuffer;
                delete[] pBufferOut;
                GDALClose( (GDALDatasetH) pDataset );
                GDALClose( (GDALDatasetH) pDSOut );
                return false;
            }
            offsetY++;
        }
    }

    delete[] pBuffer;
    delete[] pBufferOut;

    GDALDriver * poDriver = GetGDALDriverManager()->GetDriverByName("JPEG");
    if( NULL == poDriver ) {
        cerr<<"get driver error."<<endl;
        GDALClose( (GDALDatasetH) pDataset );
        GDALClose( (GDALDatasetH) pDSOut );
        return false;
    }
    GDALDataset * poDataset=poDriver->CreateCopy(output.c_str(),pDSOut,false,NULL,NULL,NULL);
    if( NULL == poDataset ) {
        cerr<<"driver copy error."<<endl;
        GDALClose( (GDALDatasetH) pDataset );
        GDALClose( (GDALDatasetH) pDSOut );
        return false;
    }
    GDALPamDataset *pamDs=dynamic_cast<GDALPamDataset*>(poDataset);
    if(NULL == pamDs ) {
        int pamFlags=pamDs->GetPamFlags();
        pamFlags|=GPF_NOSAVE;
        pamDs->SetPamFlags(pamFlags);
    }

    if( NULL == pDSOut )
        GDALClose( (GDALDatasetH) pDSOut );
    pDriver->Delete( "tmpFile" );
    if( NULL == pDataset )
        GDALClose( (GDALDatasetH) pDataset );
    if( NULL == poDataset )
        GDALClose( (GDALDatasetH) poDataset );
    return true;
}
