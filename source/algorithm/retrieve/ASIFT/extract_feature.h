#ifndef EXTRACT_FEATURE_H
#define EXTRACT_FEATURE_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <omp.h>


using namespace std;


bool AsiftFeature(const string Output_FileName,string Input_FilePath, vector<vector<float>>& features);

#endif // EXTRACT_FEATURE_H
