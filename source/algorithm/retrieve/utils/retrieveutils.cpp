#include "retrieveutils.h"

void* retrieveInterface(void *args) {
    InputInterface* inputArgs= (InputInterface*)args;

    WordRes* pObj = new(std::nothrow) WordRes;
    pObj->status = 0;
    /*Recognition：geographic information*/
    vector<int> gires;
    vector<vector<double>> p_targetgeo;
    int regflag = RegByGeoInf(inputArgs->imgurl, p_targetgeo, gires);
    if(regflag == -1) {
        pObj->status = -1;
        Log::Error("RegByGeoInf Error !");
        return (void*)(pObj);
    }

    if(regflag == 1) { //图像包含地理信息
        if(gires.size() != 0) { //地理范围内存在已知目标
            for(vector<int>::iterator it = gires.begin(); it != gires.end(); it++){
                ImgInfo imginf;
                imginf.id = inputArgs->p_targetno[*(it)];
                imginf.name = inputArgs->p_targetname[*(it)];
                imginf.path = "";
                pObj->keyWords.push_back(imginf);
            }
        }
        else{
            Log::Warn(inputArgs->imgurl + " This target can not be recognized.");
            pObj->status = 0; //没有找到目标
        }
        return (void*)pObj;
    }
    /*Recognition：ASIFT and Sparse Representation*/
    time_t now;
    struct tm * timenow;
    //ASIFT
    time(&now);
    timenow = localtime(&now);
    time_t start = mktime(timenow);
    Log::Info("imgSearchSync ## ASIFT Start");
    vector<vector<float>> imgFeatures;
    bool flag = AsiftFeature(inputArgs->saveurl, inputArgs->imgurl, imgFeatures);
    if(flag == false) {
        pObj->status = -1;
        Log::Error("Fetch RetrieveServer Result Struct Failed !");
        return (void*)(pObj);
    }
    time(&now);
    timenow = localtime(&now);
    time_t end = mktime(timenow);
    Log::Info("imgSearchSync ## ASIFT Done. Running time : %s", difftime(end,start));
    //Sparse Representation
    time(&now);
    timenow = localtime(&now);
    start = mktime(timenow);
    Log::Info("imgSearchSync ## Sparse Representation Start .");
    vector<int> srres;
    flag = inputArgs->p_SRClassify->SRClassify(imgFeatures, inputArgs->p_min_residual, inputArgs->p_sparsity, srres);
    if(flag == false ) {
        pObj->status = -1;
        Log::Error("Fetch RetrieveServer Result Struct Failed !");
        return (void*)(pObj);
    }
    time(&now);
    timenow = localtime(&now);
    end = mktime(timenow);
    Log::Info("imgSearchSync ## Sparse Representation Done. Running time: %s", difftime(end, start));

    for(vector<int>::iterator it = srres.begin(); it != srres.end(); it++){
        ImgInfo imginf;
        imginf.id = inputArgs->p_targetno[*(it)];
        imginf.name = inputArgs->p_targetname[*(it)];
        imginf.path = "";
        pObj->keyWords.push_back(imginf);
    }
    pObj->status = 1;
    return (void*)pObj;
}
