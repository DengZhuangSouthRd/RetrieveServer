#ifndef ALGORITHMSERVER_IMGCAP_H
#define ALGORITHMSERVER_IMGCAP_H

#include <gdal_priv.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>

using namespace std;

typedef unsigned short int ushort;
string findImageTypeGDAL(string pDstImgFileName);
//图像裁剪
bool imgcap(const string imgurl, int upleftx, int uplefty, int height, int width, const string saveurl);

#endif //ALGORITHMSERVER_IMGCAP_H
