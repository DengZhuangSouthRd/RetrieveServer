#include "retrieveutils.h"

extern  map<string, string> g_ConfMap;

void* retrieveInterface(void *args) {
    InputInterface* inputArgs= (InputInterface*)args;
    string purl = inputArgs->imgurl, imgsaveurl = inputArgs->saveurl;
    int upleftx = inputArgs->upleftx, uplefty = inputArgs->uplefty, height = inputArgs->height, width = inputArgs->width;
    WordRes* pObj = new(std::nothrow) WordRes;
    pObj->status = 0;
    bool flag = false;
    time_t now;
    struct tm * timenow;
    //裁剪图像
    flag = imgcap(purl,upleftx,uplefty,height,width,imgsaveurl);
    if(flag == false){
        pObj->status = -1;
        Log::Error("ImgCap Error !");
        return (void*)pObj;
    }

    string insert;   //将裁剪后的图像写入数据库
    string filename = imgsaveurl.substr(imgsaveurl.find_last_of('/')+1, imgsaveurl.find_last_of('.')-imgsaveurl.find_last_of('/')-1);

    /*Recognition：geographic information*/
    vector<int> gires;
    int regflag = RegByGeoInf(imgsaveurl,*(inputArgs->p_targetgeo),gires);
    if(regflag == -1){
        pObj->status = -1;
        Log::Error("RegByGeoInf Error !");
        return (void*)pObj;
    }
    if(regflag == 1){ //图像包含地理信息
        if(gires.size() != 0){ //地理范围内存在已知目标
            for(vector<int>::iterator it = gires.begin(); it != gires.end(); it++){
                ImgInfo imginf;
                imginf.id = inputArgs->p_targetno[*(it)];
                imginf.name = inputArgs->p_targetname[*(it)];
                imginf.path = "";
                pObj->keyWords.push_back(imginf);
            }
            time(&now);
            timenow = localtime(&now);
            insert = "INSERT INTO t5remotecap(cappath, capname, imgpath, timeadd, imgpixelscale, isusercap)VALUES ('"
                     +imgsaveurl.substr(0,imgsaveurl.find_last_of('/'))+"','"
                     +filename+"','"
                     +purl.substr(0,imgsaveurl.find_last_of('/'))+"','"
                     +asctime(timenow)+"','"
                     +to_string(upleftx)+","+to_string(uplefty)+","+to_string(upleftx+width)+","+to_string(uplefty+height)+"',"
                     +"'1');";
        }
        else{
            Log::Warn(imgsaveurl+" This target can not be recognized.");
            pObj->status = 0; //没有找到目标
            return (void*)pObj;
        }
    }
    else{
        /*Recognition：ASIFT and Sparse Representation*/

        //ASIFT
        time(&now);
        timenow = localtime(&now);
        time_t start = mktime(timenow);
        cout << "ASIFT Start." << endl;
        string featuresaveurl = g_ConfMap["RETRIEVEUSERIMGFEATUREDIR"] + filename + ".csv";
        cout << "Feature Save URL ## " << featuresaveurl << endl;
        vector<vector<float>> imgFeatures;
        flag = AsiftFeature(featuresaveurl, imgsaveurl, imgFeatures);
        if(flag == false) {
            Log::Error("Fetch RetrieveServer Result Struct Failed !");
            pObj->status = -1;
            return (void*)pObj;
        }
        time(&now);
        timenow = localtime(&now);
        time_t end = mktime(timenow);
        cout << "ASIFT Done. Running time:" << difftime(end,start) << endl;

        //Sparse Representation
        time(&now);
        timenow = localtime(&now);
        start = mktime(timenow);
        cout << "Sparse Representation Start." << endl;
        vector<int> srres;
        float p_min_residual = inputArgs->p_min_residual;
        int p_sparsity = inputArgs->p_sparsity;
        flag = inputArgs->p_SRClassify->SRClassify(imgFeatures, p_min_residual, p_sparsity, srres);
        if(flag == false ) { //|| srres.size() != p_targetname.size()
            Log::Error("Fetch RetrieveServer Result Struct Failed !");
            pObj->status = -1;
            return (void*)pObj;
        }
        time(&now);
        timenow = localtime(&now);
        end = mktime(timenow);
        cout << "Sparse Representation Done. Running time:" << difftime(end,start) << endl;

        for(vector<int>::iterator it = srres.begin(); it != srres.end(); it++){
            ImgInfo imginf;
            imginf.id = inputArgs->p_targetno[*(it)];
            imginf.name = inputArgs->p_targetname[*(it)];
            imginf.path = "";
            pObj->keyWords.push_back(imginf);
        }

        insert = "INSERT INTO t5remotecap(cappath, capname, imgpath, timeadd, imgpixelscale, isusercap)VALUES ('"
                 +imgsaveurl.substr(0,imgsaveurl.find_last_of('/'))+"','"
                 +filename+"','"
                 +purl.substr(0,imgsaveurl.find_last_of('/'))+"','"
                 +featuresaveurl+"','"
                 +asctime(timenow)+"','"
                 +to_string(upleftx)+","+to_string(uplefty)+","+to_string(upleftx+width)+","+to_string(uplefty+height)+"',"
                 +"'1');";
    }

    //写入数据库
    flag = inputArgs->p_pgdb->pg_exec_sql(insert);
    if(flag == false) {
        Log::Error("Insert Img Failed !");
        pObj->status = -1;
        return (void*)pObj;
    }
    pObj->status = 1;
    return (void*)pObj;
}

void deepCopyWordRes(WordRes* src, WordRes& dest) {
    dest.status = src->status;
    for(vector<ImgInfo>::iterator it=src->keyWords.begin(); it != src->keyWords.end(); ++it) {
        dest.keyWords.push_back(*it);
    }
}

