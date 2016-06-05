//
// Created by 贵阳 on 6/3/16.
//

#include "retrieveserver.h"

RetrieveServer::RetrieveServer() {
    p_threadPool = NULL;
    p_threadPool = ThreadPool::getSingleInstance();
}

RetrieveServer::~RetrieveServer() {
    p_threadPool->revokeSingleInstance();
    p_threadPool = NULL;
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
    for(vector<string>::const_iterator it=wordres.keyWords.begin(); it!=wordres.keyWords.end(); it++) {
        str += (" " + *it);
    }
    Log::Info(str);
}

void RetrieveServer::log_OutputResult(const ImgRes &imgres) {
    string str = "RetrieveServer ## ";
    str += ("status " + to_string(imgres.status));
    for(vector<string>::const_iterator it=imgres.imgRemote.begin(); it!=imgres.imgRemote.end(); it++) {
        str += (" " + *it);
    }
    for(vector<string>::const_iterator it=imgres.imgPic.begin(); it!=imgres.imgPic.end(); it++) {
        str += (" " + *it);
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
    string task_id = mapArg.at("id");
    WordRes obj;
    log_InputParameters(mapArg);
    if(mapArg.count("imgurl") == 0) {
        obj.status = -1;
        Log::Error("RetrieveServer ## imgSearchSync, Parameters Error !");
        return obj;
    }
    string imgurl(mapArg.at("imgurl"));
    string saveurl("");
    vector<vector<double>> imgFeatures;
    bool flag = ASIFT_Ext_Features_Gdal(saveurl, imgurl, imgFeatures);
    if(flag == false) {
        Log::Error("Fetch Fusion Result Struct Failed !");
        obj.status = -1;
    }
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
