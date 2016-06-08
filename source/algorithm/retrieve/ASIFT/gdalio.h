#ifndef _IO_H_
#define _IO_H_

#include <gdal_priv.h>

#ifndef BuildingNum 
#define BuildingNum 7
#endif

#ifndef DATA2D
#define DATA2D(Matrix,m,n,SizeCol) Matrix[(m)*(SizeCol)+(n)]
#endif

#include <iostream>

using namespace std;

int ReadImageToBuff(const char* InputFileName, float **pImageBuf, int &height,int &width,int &bandcount) {

    /*
    * @brief    ReadImageToBuff.
    * 把影像读入到内存空间
    * @param    inputFileName       输入文件路径
    * @param    pImageBuf           内存块指针
    * @exception    无
    * @return   0   成功
    * @return   1   图像含有地理信息
    * @return   -1  内存分配失败
    * @return   -2  读取数据失败
    */

    GDALAllRegister();         //利用GDAL读取图片，先要进行注册
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径

    //准备读取图片
    GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(InputFileName, GA_ReadOnly);
    //输出图像的坐标和分辨率信息
    double adfGeoTransform[6];
    if(ReadDataSet->GetGeoTransform(adfGeoTransform)==CE_None){
        return 1;
    }

    width  = ReadDataSet->GetRasterXSize();
    height = ReadDataSet->GetRasterYSize();
    bandcount = ReadDataSet->GetRasterCount();

    *pImageBuf = NULL;
    *pImageBuf = new(std::nothrow) float[width*height*bandcount];
    if (NULL == *pImageBuf) {
        cerr << "Memory Error !" << " file: " << __FILE__ << "line: " << __LINE__ << "Time: " << __TIME__ << endl;
        delete ReadDataSet;
        ReadDataSet = NULL;
        return -1;
    }

    if (ReadDataSet->RasterIO(GF_Read, 0, 0, width, height, *pImageBuf, width, height, GDT_Float32, bandcount, NULL, 0, 0, 0) == CE_Failure) {
        cerr << "Memory Error !" << " file: " << __FILE__ << "line: " << __LINE__ << "Time: " << __TIME__ << endl;
        delete ReadDataSet; ReadDataSet = NULL;
        delete[] *pImageBuf; *pImageBuf = NULL;
        return -2;
    }
    delete ReadDataSet; ReadDataSet = NULL;
    return 0;
}

//彩色图转灰度图
float* Gdal_rgb2gray(float *rgb,int height,int width,int bandcount){
    if (bandcount !=3) {
        cerr<<"This is not RGB image."<<endl;
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;

    }
    float *gray=new float[height*width];
    for (int i=0;i<height*width;i++)
        gray[i]=DATA2D(rgb,0,i,height*width)*0.299+DATA2D(rgb,1,i,height*width)*0.587+DATA2D(rgb,2,i,height*width)*0.114;
    return gray;
}

#endif
