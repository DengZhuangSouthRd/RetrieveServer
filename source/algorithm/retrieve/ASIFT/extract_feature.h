#ifndef EXTRACT_FEATURE_H
#define EXTRACT_FEATURE_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <omp.h>

using namespace std;

bool ASIFT_Ext_Features_Gdal(const string Output_FileName,string Input_FilePath, vector<vector<double>>& feature_vec);

#endif // EXTRACT_FEATURE_H
