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
        throw runtime_error("PG DB Not Working ! Please Check !");
    }
    p_SRClassify = NULL;
    p_SRClassify = new SR<float>();
    
    //加载字典
    string getdic="SELECT dicpath FROM t7dictionary;";
    result res;
    p_pgdb->pg_fetch_sql(getdic,res);
    
    vector<string> dict_path;
    for (result::const_iterator c = res.begin(); c != res.end(); ++c) {
        dict_path.push_back(C[0].as<string>()); //dic path
    }
    bool flag = p_SRClassify->LoadDic(dict_path);
    if(flag == false) {
        delete p_pgdb;
        throw runtime_error("Load Dic Error.");
    }
}

RetrieveServer::~RetrieveServer() {
    p_threadPool->revokeSingleInstance();
    p_threadPool = NULL;
    delete p_pgdb;
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
    str += ("status " + to_string(wordres.status));
    for(auto it=wordres.keyWords.begin(); it!=wordres.keyWords.end(); it++) {
        str += (" id:" + to_string((*it).id) + " path:" + (*it).path + " name:" + (*it).name);
    }
    Log::Info(str);
}

void RetrieveServer::log_OutputResult(const ImgRes &imgres) {
    string str = "RetrieveServer ## ";
    str += ("status " + to_string(imgres.status));
    for(auto it=imgres.imgRemote.begin(); it!=imgres.imgRemote.end(); it++) {
        str += (" id:" + to_string((*it).id) + " path:" + (*it).path + " name:" + (*it).name);
    }
    for(auto it=imgres.imgPic.begin(); it!=imgres.imgPic.end(); it++) {
        str += (" id:" + to_string((*it).id) + " path:" + (*it).path + " name:" + (*it).name);
    }
    Log::Info(str);
}

WordWiki RetrieveServer::wordGetKnowledge(const string &word, const Ice::Current &) {
    WordWiki obj;
    obj.key.assign(word.c_str());
    obj.descr = "";
    obj.abstr = "";
    return obj;
}

WordRes RetrieveServer::wordSearch(const DictStr2Str &mapArg, const Ice::Current &) {
    WordRes obj;
    obj.status = 1;
    return obj;
}

ImgRes RetrieveServer::wordSearchImg(const DictStr2Str &mapArg, const Ice::Current &) {
    ImgRes obj;
    obj.status = 1;
    return obj;
}

WordRes RetrieveServer::imgSearchSync(const DictStr2Str &mapArg, const Ice::Current &) {
    string task_id = mapArg.at("uuid");
    WordRes obj;
    obj.status = 0;
    log_InputParameters(mapArg);
    if(mapArg.count("purl") == 0) {
        obj.status = -1;
        Log::Error("RetrieveServer ## imgSearchSync, Parameters Error !");
        return obj;
    }
    string purl(mapArg.at("purl"));
    string filename = purl.substr(purl.find_last_of('/'), purl.find_last_of('.'));
//    string saveurl = g_ConfMap["RETRIEVEUSERIMGFEATUREDIR"] + filename + ".csv";
    string saveurl = "/Users/liuguiyang/Documents/CodeProj/ConsoleProj/RetrieveServer/data/retrieve/feature/" + filename + ".csv";
    cout << "Save URL ## " << saveurl << endl;
    vector<vector<float>> imgFeatures;
    bool flag = AsiftFeature(saveurl, purl, imgFeatures);
    if(flag == false) {
        Log::Error("Fetch RetrieveServer Result Struct Failed !");
        obj.status = -1;
    }
    //vector<string> dict_path;
    //dict_path.push_back("/Users/liuguiyang/Documents/CodeProj/ConsoleProj/RetrieveServer/data/retrieve/dict/13_国家图书馆_f.csv");
    //dict_path.push_back("/Users/liuguiyang/Documents/CodeProj/ConsoleProj/RetrieveServer/data/retrieve/dict/14_国家大剧院_f.csv");
    //flag = p_SRClassify->LoadDic(dict_path);
    //if(flag == false) {
    //    Log::Error("Fetch RetrieveServer Result Struct Failed !");
    //    obj.status = -1;
    //}
    int res = p_SRClassify->SRClassify(imgFeatures, 1, 1);
    if(res == -1) {
        Log::Error("Fetch RetrieveServer Result Struct Failed !");
        obj.status = -1;
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
