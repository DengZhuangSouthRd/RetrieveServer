#include "imgcap.h"

char* findImageTypeGDAL( char *pDstImgFileName)  
{  
    char *dstExtension = strlwr(strrchr(pDstImgFileName,'.') + 1);  
    char *Gtype = NULL;  
    if      (0 == strcmp(dstExtension,"bmp")) Gtype = "BMP";  
    else if (0 == strcmp(dstExtension,"jpg")) Gtype = "JPEG";  
    else if (0 == strcmp(dstExtension,"png")) Gtype = "PNG";  
    else if (0 == strcmp(dstExtension,"tif")) Gtype = "GTiff";   
    else Gtype = NULL;  
  
    return Gtype;  
} 
bool imgcap(const string imgurl, int upleftx, int uplefty, int height, int width, const string saveurl)
{
    /*imgcap	    图像裁剪
	*imgurl			原图像地址
	*upleftx		裁剪坐标x
	*uplefty		裁剪坐标y
	*height			裁剪高度
	*width			裁剪宽度
	*saveurl		裁剪后图像保存位置
	*return			true|false
	*author:ys
	*date:2016.06.22
	*/
    GDALAllRegister();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
    
    GDALDataset *ReadDataset = (GDALDataset *) GDALOpen( imgurl.c_str(), GA_ReadOnly );
    if( NULL == ReadDataset )
    {
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
        return false;
    }
    int imgwidth = ReadDataSet->GetRasterXSize();	//图像宽度
	int imgheight = ReadDataSet->GetRasterYSize();	//图像高度
	int imgbandcount = ReadDataSet->GetRasterCount();//图像波段
    double adfGeoTransform[6] = {0};
    bool flag = false;//标记是否有地理信息
    if (ReadDataSet->GetGeoTransform(adfGeoTransform) == CE_None){ 
        flag = true; //包含地理信息
	}
	if (x + width > imgwidth || y + height > imgheight)
	{
		cerr<<"Param Error."<<endl;
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
		return false;
	}

    ushort imgdata = new(std::nothrow) ushort(height*width*imgbandcount);
	if (NULL == imgdata)
	{
		GDALClose(ReadDataSet); ReadDataSet = NULL;
		cerr<<"Memory Error.\n";
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
		return false;
	}
    //读数据
	if (ReadDataSet->RasterIO(GF_Read, x, y, width, height, imgdata, width, height, GDT_Uint16, imgbandcount, NULL, 0, 0, 0) == CE_Failure)
	{
		GDALClose(ReadDataSet); ReadDataSet = NULL;
		delete[] imgdata; imgdata = NULL;
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
		return false;
	}
    GDALClose(ReadDataSet); ReadDataSet = NULL;


    //保存数据
    char *GType = NULL;  
    GType = findImageTypeGDAL(saveurl.c_str());  
    if (GType == NULL)
    {
        delete[] imgdata; imgdata = NULL;
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl; 
        return false; 
    }  
  
    GDALDriver *pMemDriver = NULL;  
    pMemDriver = GetGDALDriverManager()->GetDriverByName("MEM");  
    if( pMemDriver == NULL )
    {
        delete[] imgdata; imgdata = NULL;
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
        return false; 
    }  
  
    GDALDataset * pMemDataSet = pMemDriver->Create("",width,height,imgbandcount,GDT_Uint16,NULL);
	if (NULL == pMemDataSet)
    {
        delete[] imgdata; imgdata = NULL;
        pMemDriver = NULL;
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
		return false; 
	}

    if (pMemDataSet->RasterIO(GF_Write, 0, 0, width, height, imgdata, width, height, GDT_Uint16, imgbandcount, NULL, 0, 0, 0) == CE_Failure)
    {
		GDALClose(pMemDataSet); pMemDataSet = NULL;
        pMemDriver = NULL;
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
		delete[] imgdata; imgdata = NULL;
		return false;
	}

    delete[] imgdata; imgdata = NULL;//释放内存
    pMemDriver = NULL;

    GDALDriver *pDstDriver = NULL;  
    pDstDriver = (GDALDriver *)GDALGetDriverByName(GType);  
    if (pDstDriver == NULL) {
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
        return false; 
    }
      
    pDstDriver->CreateCopy(saveurl.c_str(),pMemDataSet,FALSE, NULL, NULL, NULL);
	
	//如果存在地理信息则写入
    if(flag)
    {
	    char *WKT = NULL;
	    OGRSpatialReference oSRS;
	    oSRS.SetWellKnownGeogCS("WGS84");
	    oSRS.SetUTM(50,TRUE);
	    oSRS.exportToWkt(&WKT); 
	    pDstDataSet->SetProjection(WKT);
	    CPLFree(WKT);WKT = NULL;
        adfGeoTransform[3] = adfGeoTransform[3] + x * adfGeoTransform[4] + y * adfGeoTransform[5]; //纬度
	    adfGeoTransform[0] = adfGeoTransform[0] + x * adfGeoTransform[1] + y * adfGeoTransform[2]; //经度
	    pDstDataSet->SetGeoTransform(adfGeoTransform);
    }

	//delete pDstDriver;
	pDstDriver = NULL;
    GDALClose(pMemDataSet); pMemDataSet = NULL;
	GDALClose(pDstDataSet); pDstDataSet=NULL;

    return true;
    

    
}