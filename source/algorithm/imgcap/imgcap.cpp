#include "imgcap.h"

string findImageTypeGDAL(string pDstImgFileName) {
    size_t ind = pDstImgFileName.rfind('.');
    if(ind == string::npos) {
        return NULL;
    }
    string dstExtension = pDstImgFileName.substr(ind+1);
    std::transform(dstExtension.begin(), dstExtension.end(), dstExtension.begin(), ::tolower);
    string Gtype;
    if      (0 == strcmp(dstExtension.c_str(),"bmp")) Gtype = "BMP";
    else if (0 == strcmp(dstExtension.c_str(),"jpg") ||0 == strcmp(dstExtension.c_str(),"jpeg")||0 == strcmp(dstExtension.c_str(),"JPG") ||0 == strcmp(dstExtension.c_str(),"JEPG")) Gtype = "JPEG";
    else if (0 == strcmp(dstExtension.c_str(),"png")) Gtype = "PNG";
    else if (0 == strcmp(dstExtension.c_str(),"tif")) Gtype = "GTiff";

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
    
    GDALDataset *ReadDataSet = (GDALDataset *) GDALOpen( imgurl.c_str(), GA_ReadOnly );
    if( NULL == ReadDataSet )
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

    cout << "imgwidth is " << imgwidth << ", imgheight is " << imgheight << endl;
    width = (int)(imgwidth/ 604.0 * width);
	height = (int)(imgheight / 403.0 * height);
    upleftx = (int)(imgwidth / 604.0 * upleftx);
    uplefty = (int)(imgheight / 403.0 * uplefty);
    cout << "width is " << width << "height is " << height << "upleftx is " << upleftx << "uplefty is " << uplefty << endl;

    if (upleftx + width > imgwidth || uplefty + height > imgheight)
	{
		cerr<<"Param Error."<<endl;
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
		return false;
	}

    //待修改-------------根据图像数据类型定义数组
    //char* DataType = GDALGetDataTypeName(ReadDataSet->GetRasterBand(1)->GetRasterDataType);//获取图像数据类型
    //
    //
    //
    unsigned char imgdata[height*width*imgbandcount];
    //-----------------------------------------
    
    //读数据
	if (ReadDataSet->RasterIO(GF_Read, upleftx, uplefty, width, height, imgdata, width, height, GDT_Byte, imgbandcount, NULL, 0, 0, 0) == CE_Failure)
	{
		GDALClose(ReadDataSet); ReadDataSet = NULL;
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
		return false;
	}
    GDALClose(ReadDataSet); ReadDataSet = NULL;


    //保存数据
    string GType;
    GType = findImageTypeGDAL(saveurl);
    if (GType.empty()) {
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
        return false; 
    }  
  
    GDALDriver *pMemDriver = NULL;  
    pMemDriver = GetGDALDriverManager()->GetDriverByName("MEM");  
    if( pMemDriver == NULL )
    {
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
        return false; 
    }  
  
    GDALDataset * pMemDataSet = pMemDriver->Create("",width,height,imgbandcount,GDT_Byte,NULL);
	if (NULL == pMemDataSet)
    {
        pMemDriver = NULL;
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
		return false; 
	}
 
    if (pMemDataSet->RasterIO(GF_Write, 0, 0, width, height, imgdata, width, height, GDT_Byte, imgbandcount, NULL, 0, 0, 0) == CE_Failure)
    {
		GDALClose(pMemDataSet); pMemDataSet = NULL;
        pMemDriver = NULL;
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
		return false;
	}

    pMemDriver = NULL;

    GDALDriver *pDstDriver = NULL;  
    pDstDriver = (GDALDriver *)GDALGetDriverByName(GType.c_str());
    if (pDstDriver == NULL) {
        cerr<<"file: "<<__FILE__<<"line: "<<__LINE__<<"time: "<<__DATE__<<" "<<__TIME__<<endl;
        return false; 
    }
      
	//如果存在地理信息则写入
    if(flag) {
	    char *WKT = NULL;
	    OGRSpatialReference oSRS;
	    oSRS.SetWellKnownGeogCS("WGS84");
	    oSRS.SetUTM(50,TRUE);
	    oSRS.exportToWkt(&WKT);
        pMemDataSet->SetProjection(WKT);
	    CPLFree(WKT);WKT = NULL;
        adfGeoTransform[3] = adfGeoTransform[3] + upleftx * adfGeoTransform[4] + uplefty * adfGeoTransform[5]; //纬度
	    adfGeoTransform[0] = adfGeoTransform[0] + upleftx * adfGeoTransform[1] + uplefty * adfGeoTransform[2]; //经度
        pMemDataSet->SetGeoTransform(adfGeoTransform);
    }

    //  delete pDstDriver;
    //	pDstDriver = NULL;

    if(pDstDriver->CreateCopy(saveurl.c_str(),pMemDataSet,FALSE, NULL, NULL, NULL) == NULL) {
        GDALClose(pMemDataSet); pMemDataSet = NULL;
        return false;
    }
    GDALClose(pMemDataSet); pMemDataSet = NULL;
    return true;
}