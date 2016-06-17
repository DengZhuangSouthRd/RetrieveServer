//
// Created by 贵阳 on 6/3/16.
//

#include "retrieveserver.h"

extern  map<string, string> g_ConfMap;
RetrieveServer::RetrieveServer() {
    p_threadPool = NULL;
    p_threadPool = ThreadPool::getSingleInstance();
    p_pgdb = NULL;
    string pg_con_info = getPGConfInfo(g_ConfMap);
    p_pgdb = new PGDB(pg_con_info);
    if(p_pgdb->is_Working() == false) {
        delete p_pgdb;
        p_pgdb = NULL;
        throw runtime_error("PG DB Not Working ! Please Check !");
    }
    p_SRClassify = NULL;
    p_SRClassify = new SR<float>();
    
    //读取目标信息
    string getdic = "SELECT t7.dicpath,t7.targetname,t7.targetno,t3.lu,t3.rd FROM t3targetinfo AS t3,t7dictionary AS t7 WHERE t3.targetno = t7.targetno AND t3.status_ = '1' AND t7.status_ = '1';";
    result res;
    bool flag = p_pgdb->pg_fetch_sql(getdic,res);
    if(flag == false) {
        delete p_pgdb;
        throw runtime_error("PG DB Execute Error.");
    }
    vector<string> dict_path;
    for (result::const_iterator it = res.begin(); it != res.end(); ++it) {
        if(it[0].as<string>().empty() || it[1].as<string>().empty() || it[2].as<string>().empty() || it[3].as<string>().empty() ||it[4].as<string>().empty() ){
            cerr << "Targetno is " << it[2].as<string>() << ", Targetname is " << it[1].as<string>() << ", Dicpath is " << it[0].as<string>();
            cerr << ", Geoinf is" << it[3].as<string>().empty() << " " <<it[4].as<string>().empty()<<endl;
            delete p_pgdb;
            p_pgdb = NULL;
            throw runtime_error("PG DB Data Error.");
        }
        dict_path.push_back(it[0].as<string>()); //字典路径
        p_targetname.push_back(it[1].as<string>());//目标名称
        p_targetno.push_back(it[2].as<int>());//目标序号
        vector<double> tmp;
        string str = it[3].as<string>(); //以","分割     
        tmp.push_back(std::stod( str.substr(0,str.find_last_of(",")).c_str() ));//左上角经度
        tmp.push_back(std::stod( str.substr(str.find_last_of(",")+1,str.length()-str.find_last_of(",")-1).c_str() ));//左上角纬度
        str = it[4].as<string>();        //以","分割   
        tmp.push_back(std::stod( str.substr(0,str.find_last_of(",")).c_str() ));//右下角经度
        tmp.push_back(std::stod( str.substr(str.find_last_of(",")+1,str.length()-str.find_last_of(",")-1).c_str() ));//右下角纬度

        p_targetgeo.push_back(tmp);//地理信息

    }

    //加载字典
    flag = p_SRClassify->LoadDic(dict_path);
    if(flag == false) {
        delete p_pgdb;
        p_pgdb = NULL;
        throw runtime_error("Load Dic Error.");
    }
    p_sparsity = std::atoi( g_ConfMap["RETRIEVESPARSITY"].c_str() );
    p_min_residual = std::atof( g_ConfMap["RETRIEVEMINRESIDUAL"].c_str() );
}

RetrieveServer::~RetrieveServer() {
    p_threadPool->revokeSingleInstance();
    p_threadPool = NULL;
    if(p_pgdb)
        delete p_pgdb;
    if(p_SRClassify)
        delete p_SRClassify;
}

void RetrieveServer::log_InputParameters(const DictStr2Str &mapArg) {
    string str = "RetrieveServer ## ";
    for(DictStr2Str::const_iterator it=mapArg.begin(); it!=mapArg.end(); ++it) {
        str += (it->first + "=" + it->second + "#");
    }
    Log::Info(str);
}

void RetrieveServer::log_OutputResult(const WordWiki &wiki) {
    string str = "RetrieveServer ## ";
    str += ("Key is " + wiki.key);
    str += ("Abstr is " + wiki.abstr);
    str += ("Descr is " + wiki.descr);
    Log::Info(str);
}

void RetrieveServer::log_OutputResult(const WordRes &wordres) {
    string str = "RetrieveServer ## ";
    str = ("status " + to_string(wordres.status));
    Log::Info(str);
    for(auto it=wordres.keyWords.begin(); it!=wordres.keyWords.end(); it++) {
        str = (" id:" + to_string((*it).id) + " path:" + (*it).path + " name:" + (*it).name);
        Log::Info(str);
    }

}

void RetrieveServer::log_OutputResult(const ImgRes &imgres) {
    string str = "RetrieveServer ## ";
    str = ("status " + to_string(imgres.status));
    Log::Info(str);
    for(auto it=imgres.imgRemote.begin(); it!=imgres.imgRemote.end(); it++) {
        str = (" id:" + to_string((*it).id) + " path:" + (*it).path + " name:" + (*it).name);
        Log::Info(str);
    }
    for(auto it=imgres.imgPic.begin(); it!=imgres.imgPic.end(); it++) {
        str = (" id:" + to_string((*it).id) + " path:" + (*it).path + " name:" + (*it).name);
        Log::Info(str);
    }
    
}

WordWiki RetrieveServer::wordGetKnowledge(const string &word, const Ice::Current &) {
    WordWiki obj;
    obj.key.assign(word.c_str());
    obj.descr = "";
    obj.abstr = "";
    log_OutputResult(obj);
    return obj;
}

WordRes RetrieveServer::wordSearch(const DictStr2Str &mapArg, const Ice::Current &) {
    WordRes obj;
    obj.status = 1;
    log_InputParameters(mapArg);
    string word = mapArg.at("w");
    string pi = mapArg.at("pi");
    string pn = mapArg.at("pn");
    //字符串匹配
    string getimginf = "SELECT id,targetname FROM t3targetinfo WHERE targetname = '" + word + "' AND status_ = '1' ORDER BY id LIMIT "\
                       + pn + " OFFSET "+to_string((std::atoi(pi.c_str())-1)*std::atoi(pn.c_str()))+";";
    result res;
    bool flag = p_pgdb->pg_fetch_sql(getimginf,res);
    if(flag == false) {
        Log::Error("Word Search Failed !");
        obj.status = -1;
        return obj;
    }

    for (result::const_iterator it = res.begin(); it != res.end(); ++it) {
        if(it[0].as<string>().empty()||it[1].as<string>().empty()){
            Log::Error("id is "+it[0].as<string>()+", targetname is "+it[1].as<string>());
            obj.status = -1;
        }
        ImgInfo imginf;
        imginf.id = it[0].as<int>();
        imginf.name = it[1].as<string>();
        imginf.path = "";
        obj.keyWords.push_back(imginf);
    }
    log_OutputResult(obj);        
    return obj;
}

ImgRes RetrieveServer::wordSearchImg(const DictStr2Str &mapArg, const Ice::Current &) {
    ImgRes obj;
    obj.status = 1;
    log_InputParameters(mapArg);
    string word = mapArg.at("w");
    string pi = mapArg.at("pi");
    string pn = mapArg.at("pn");
    //
    string getimginf = "SELECT id,picname,picpath FROM t4pic WHERE targetname = '" + word + "' AND status_ = '1' ORDER BY id LIMIT "\
                       + pn + " OFFSET "+to_string((std::atoi(pi.c_str())-1)*std::atoi(pn.c_str()))+";";
    result res;
    bool flag = p_pgdb->pg_fetch_sql(getimginf,res);
    if(flag == false) {
        Log::Error("Word Search Img Failed !");
        obj.status = -1;
        return obj;
    }

    for (result::const_iterator it = res.begin(); it != res.end(); ++it) {
        if(it[0].as<string>().empty()||it[1].as<string>().empty()||it[2].as<string>().empty()){
            Log::Error("pic:id is " + it[0].as<string>() + ", name is " + it[1].as<string>() + ", path is " + it[2].as<string>());
            obj.status = -1;
        }
        ImgInfo imginf;
        imginf.id = it[0].as<int>();
        imginf.name = it[1].as<string>();
        imginf.path = it[2].as<string>();
        obj.imgPic.push_back(imginf);
    }

    //
    getimginf = "SELECT id,capname,cappath FROM t5remotecap WHERE targetname = '" + word + "' AND status_ = '1' ORDER BY id LIMIT "\
                + pn + " OFFSET "+to_string((std::atoi(pi.c_str())-1)*std::atoi(pn.c_str()))+";";
    flag = p_pgdb->pg_fetch_sql(getimginf,res);
    if(flag == false) {
        Log::Error("Word Search RemoteImg Failed !");
        obj.status = -1;
        return obj;
    }

    for (result::const_iterator it = res.begin(); it != res.end(); ++it) {
        if(it[0].as<string>().empty()||it[1].as<string>().empty()||it[2].as<string>().empty()){
            Log::Error("remote image:id is " + it[0].as<string>() + ", name is " + it[1].as<string>() + ", path is " + it[2].as<string>());
            obj.status = -1;
        }
        ImgInfo imginf;
        imginf.id = it[0].as<int>();
        imginf.name = it[1].as<string>();
        imginf.path = it[2].as<string>();
        obj.imgRemote.push_back(imginf);
    }
    log_OutputResult(obj);
    return obj;
}

WordRes RetrieveServer::imgSearchSync(const DictStr2Str &mapArg, const Ice::Current &) {
    string task_id = mapArg.at("uuid");
    WordRes obj;
    obj.status = 1;
    log_InputParameters(mapArg);
    if(mapArg.count("purl") == 0) {
        obj.status = -1;
        Log::Error("RetrieveServer ## imgSearchSync, Parameters Error !");
        return obj;
    }
    string purl(mapArg.at("purl"));
    string filename = purl.substr(purl.find_last_of('/')+1, purl.find_last_of('.')-purl.find_last_of('/')-1);
    string saveurl = g_ConfMap["RETRIEVEUSERIMGFEATUREDIR"] + filename + ".csv";
//    string saveurl = "/Users/liuguiyang/Documents/CodeProj/ConsoleProj/RetrieveServer/data/retrieve/feature/" + filename + ".csv";
    cout << "Save URL ## " << saveurl << endl;
    
    /*Recognition：geographic information*/
    vector<int> gires;
    int regflag = RegByGeoInf(purl,p_targetgeo,gires);
    if(regflag == -1){
        obj.status = -1;
        Log::Error("RegByGeoInf Error !");
        return obj;        
    }
    if(regflag == 1){ //图像包含地理信息
        if(gires.size() != 0){ //地理范围内存在已知目标
            for(vector<int>::iterator it = gires.begin(); it != gires.end(); it++){
                ImgInfo imginf;
                imginf.id = p_targetno[*(it)];
                imginf.name = p_targetname[*(it)];
                imginf.path = "";
                obj.keyWords.push_back(imginf);
            }
        }
        else{
            Log::Warn(purl+" This target can not be recognized.");
            obj.status = 0; //没有找到目标
        }
        return obj;
    }
    /*Recognition：ASIFT and Sparse Representation*/
    time_t now;
    struct tm * timenow;
    //ASIFT
    time(&now);
    timenow = localtime(&now);
    time_t start = mktime(timenow);
    cout << "ASIFT Start." << endl;
    vector<vector<float>> imgFeatures;
    bool flag = AsiftFeature(saveurl, purl, imgFeatures);
    if(flag == false) {
        Log::Error("Fetch RetrieveServer Result Struct Failed !");
        obj.status = -1;
        return obj;
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
    flag = p_SRClassify->SRClassify(imgFeatures, p_min_residual, p_sparsity, srres);
    if(flag == false ) { //|| srres.size() != p_targetname.size()
        Log::Error("Fetch RetrieveServer Result Struct Failed !");
        obj.status = -1;
        return obj;
    }
    time(&now);
    timenow = localtime(&now);
    end = mktime(timenow);
    cout << "Sparse Representation Done. Running time:" << difftime(end,start) << endl;

    for(vector<int>::iterator it = srres.begin(); it != srres.end(); it++){
        ImgInfo imginf;
        imginf.id = p_targetno[*(it)];
        imginf.name = p_targetname[*(it)];
        imginf.path = "";
        obj.keyWords.push_back(imginf);

    }

    log_OutputResult(obj);
    return obj;
}

int RetrieveServer::imgSearchAsync(const DictStr2Str &mapArg, const Ice::Current &) {
    int status = 2;
    return status;
}

WordRes RetrieveServer::fetchImgSearchResult(const DictStr2Str &mapArg, const Ice::Current &) {
    WordRes obj;
    obj.status = 1;
    return obj;
}
