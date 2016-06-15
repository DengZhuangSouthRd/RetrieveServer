#ifndef EXTRACT_FEATURE_H
#define EXTRACT_FEATURE_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#if defined(__clang__)
#include <libiomp/omp.h>
#elif defined(__GNUG__) || defined(__GNUC__)
#include <omp.h>
#endif

using namespace std;

bool AsiftFeature(const string Output_FileName,string Input_FilePath, vector<vector<float>>& features);

#endif // EXTRACT_FEATURE_H
