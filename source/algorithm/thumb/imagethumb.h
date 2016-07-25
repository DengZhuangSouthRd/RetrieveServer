/********************************************************************
	created:	2016/05/16
	created:	16:5:2016   15:36
	filename: 	D:\code\vs2010\C\Fusion\Thumb.h
	file path:	D:\code\vs2010\C\Fusion
	file base:	Thumb
	file ext:	h
	author:		YS
	
	purpose:	实现影像缩略图(.jpg)
*********************************************************************/
#ifndef _IMAGETHUMB_H_
#define _IMAGETHUMB_H_
#include "gdal_priv.h"  //c++ 语法
#include "ogr_spatialref.h"
#include "gdal_pam.h"
#include "gdalwarper.h"
#include <io.h>
#include <string>
#include <sstream>
#include <iostream>
using namespace std;

typedef unsigned short int ushort;
typedef unsigned char uchar;

void BufArrayUint16ToByte(ushort * &arrayIn, int dataLength, ushort lower, ushort upper); 
		
void GetHistogramUInt16(ushort * &pDataBuffer,int size); 
	
int GetHistogramCutLower(vector<long> histogram, long total); 
	
int GetHistogramCutUpper(vector<long> histogram, long total); 

//生成单波段灰度缩略图

bool CreateSquareThumb( const string input , const string output,  int bandID = 1); //填充成正方形

//生成RGB彩色缩略图

bool CreateSquareThumb( const string input , const string output,int redBand,int greenBand, int blueBand);//填充成正方形

#endif