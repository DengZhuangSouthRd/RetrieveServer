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

using namespace std;
using namespace RPCImgRecong;

typedef struct _InputInterface{
    string imgurl;
    string saveurl;
    string uuid;
    vector<string> p_targetname;
    vector<int> p_targetno;
    int p_sparsity;
    float p_min_residual;
    SR<float>* p_SRClassify;
}InputInterface;


void* retrieveInterface(void* args);
void deepCopyWordRes(WordRes* src, WordRes& dest);

#endif // RETRIEVEUTILS_H
