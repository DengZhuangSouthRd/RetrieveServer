//
// Created by 贵阳 on 5/18/16.
//

#ifndef ALGORITHMSERVER_IMAGETHUMB_H
#define ALGORITHMSERVER_IMAGETHUMB_H

#include <gdal_priv.h>
#include <gdal_pam.h>
#include <string>
#include <iostream>

using namespace std;

bool CreateSquareThumb(const string input , const string output,  int bandID = 1);

bool CreateSquareThumb(const string input , const string output,int redBand,int greenBand, int blueBand);

#endif //ALGORITHMSERVER_IMAGETHUMB_H
