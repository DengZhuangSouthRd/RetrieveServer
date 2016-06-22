#ifndef RETRIEVEUTILS_H
#define RETRIEVEUTILS_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include "../ASIFT/extract_feature.h"
#include "../detect/regbygeoinf.h"
#include "../../../utils/log.h"
#include "../../../rpc/retrieve/ImageRetrieveRpc.h"
#include "../detect/sr.h"
#include "../../imgcap/imgcap.h"
#include "../../../utils/pgdb.h"

using namespace std;
using namespace RPCImgRecong;

typedef struct _InputInterface{

    string uuid;
    vector<string> p_targetname;
    vector<int> p_targetno;
    vector<vector<double>>* p_targetgeo;
    int p_sparsity;
    float p_min_residual;
    SR<float>* p_SRClassify;
    int upleftx;
    int uplefty;
    int height;
    int width;
    string imgurl;
    string saveurl;
    string featureurl;
    PGDB* p_pgdb;
}InputInterface;


void* retrieveInterface(void* args);
void deepCopyWordRes(WordRes* src, WordRes& dest);


#endif // RETRIEVEUTILS_H
