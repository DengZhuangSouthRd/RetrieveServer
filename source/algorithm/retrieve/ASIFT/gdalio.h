#ifndef _GDALIO_H_
#define _GDALIO_H_

#include <gdal_priv.h>
#include <iostream>
#include <proj_api.h> //proj4
#include <vector>
using namespace std;

#ifndef DATA2D
#define DATA2D(Matrix,m,n,SizeCol) Matrix[(m)*(SizeCol)+(n)]
#endif

int RegByGeoInf(const char* InputFileName, vector<vector<double>> targetgeo, vector<int> &res){
    /*
    * @brief    RegByGeoInf.
    * 根据地理信息识别目标
    * @param    InputFileName       输入文件路径
    * @param    targetgeo           目标地理信息
    * @param    res                 影像中包含的目标索引
    * @exception                    无
    * @return                       1:正确 0：没有地理信息 -1：错误       
    */
    GDALAllRegister();         //利用GDAL读取图片，先要进行注册
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径

    //准备读取图片
    GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(InputFileName, GA_ReadOnly);
    //获取地理信息
    double adfGeoTransform[6];
    if(ReadDataSet->GetGeoTransform(adfGeoTransform) != CE_None){
        delete ReadDataSet; ReadDataSet = NULL;
        return 0;
    }

    //Proj4 
	projPJ pj_utm = NULL;
	projPJ pj_latlon = NULL;
	if(!(pj_utm = pj_init_plus("+proj=utm +zone=50 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"))){
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		GDALClose(ReadDataSet);ReadDataSet = NULL; //释放内存
		return -1;
	}
	if(!(pj_latlon = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs"))){
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;		
		//释放内存		
		pj_free(pj_utm);pj_utm = NULL;  
		GDALClose(ReadDataSet);ReadDataSet = NULL; 
		return -1;
	}
    int nXsize= ReadDataSet->GetRasterXSize();
	int nYsize= ReadDataSet->GetRasterYSize();
    
    vector<double> x;//经度:左上角，右上角，左下角，右下角
    vector<double> y;//纬度:左上角，右上角，左下角，右下角
	//左上角
	double lat = adfGeoTransform[3]; //纬度
	double lon = adfGeoTransform[0]; //经度
	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);
	x.push_back(lon/DEG_TO_RAD);	//左上角经度坐标
	y.push_back(lat/DEG_TO_RAD);    //左上角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//右上角
	//lon = adfGeoTransform[0] + nXsize * adfGeoTransform[1];
	//lat = adfGeoTransform[3];
	//pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);
	//x.push_back(lon/DEG_TO_RAD);	//右上角经度坐标
	//y.push_back(lat/DEG_TO_RAD);    //右上角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//左下角
	//lon = adfGeoTransform[0];
	//lat = adfGeoTransform[3] + nYsize * adfGeoTransform[5];
	//pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);
	//x.push_back(lon/DEG_TO_RAD);	//左下角经度坐标
	//y.push_back(lat/DEG_TO_RAD);    //左下角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//右下角
	lat = adfGeoTransform[3] + nXsize * adfGeoTransform[4] + nYsize * adfGeoTransform[5]; //纬度
	lon = adfGeoTransform[0] + nXsize * adfGeoTransform[1] + nYsize * adfGeoTransform[2]; //经度
	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);
	x.push_back(lon/DEG_TO_RAD);	//左上角经度坐标
	y.push_back(lat/DEG_TO_RAD);    //左上角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	
    int i;
    for(i = 0; i < targetgeo.size(); i++){
        if ((x[0]-0.001<=targetgeo[i][0]&&targetgeo[i][0]<=x[1]+0.001)&&(y[1]-0.001<=targetgeo[i][1]&&targetgeo[i][1]<=y[0]+0.001)||
			(x[0]-0.001<=targetgeo[i][0]&&targetgeo[i][0]<=x[1]+0.001)&&(y[1]-0.001<=targetgeo[i][3]&&targetgeo[i][1]<=y[0]+0.001)||
			(x[0]-0.001<=targetgeo[i][0]&&targetgeo[i][2]<=x[1]+0.001)&&(y[1]-0.001<=targetgeo[i][1]&&targetgeo[i][1]<=y[0]+0.001)||
			(x[0]-0.001<=targetgeo[i][0]&&targetgeo[i][2]<=x[1]+0.001)&&(y[1]-0.001<=targetgeo[i][3]&&targetgeo[i][1]<=y[0]+0.001)||
			(targetgeo[i][0]-0.001<=x[0]&&x[0]<=targetgeo[i][2]+0.001)&&(targetgeo[i][3]-0.001<=y[0]&&y[0]<=targetgeo[i][1]+0.001)||
			(targetgeo[i][0]-0.001<=x[0]&&x[0]<=targetgeo[i][2]+0.001)&&(targetgeo[i][3]-0.001<=y[1]&&y[1]<=targetgeo[i][1]+0.001)||
			(targetgeo[i][0]-0.001<=x[1]&&x[1]<=targetgeo[i][2]+0.001)&&(targetgeo[i][3]-0.001<=y[0]&&y[0]<=targetgeo[i][1]+0.001)||
			(targetgeo[i][0]-0.001<=x[1]&&x[1]<=targetgeo[i][2]+0.001)&&(targetgeo[i][3]-0.001<=y[1]&&y[1]<=targetgeo[i][1]+0.001)){
            res.push_back(i);
        }

    }

	//释放内存
	pj_free(pj_utm);pj_utm = NULL;
	pj_free(pj_latlon);pj_latlon = NULL; 
    delete ReadDataSet; ReadDataSet = NULL;
    return 1;

}

int ReadImageToBuff(const char* InputFileName, float **pImageBuf, int &height,int &width,int &bandcount) {

    /*
    * @brief    ReadImageToBuff.
    * 把影像读入到内存空间
    * @param    InputFileName       输入文件路径
    * @param    pImageBuf           内存块指针
    * @exception    无
    * @return   0   成功
    * @return   -1  内存分配失败
    * @return   -2  读取数据失败
    */

    GDALAllRegister();         //利用GDAL读取图片，先要进行注册
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径

    //准备读取图片
    GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(InputFileName, GA_ReadOnly);
    
    //输出图像的坐标和分辨率信息
    //double adfGeoTransform[6];
    //if(ReadDataSet->GetGeoTransform(adfGeoTransform)==CE_None){
    //    return 1;
    //}
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
