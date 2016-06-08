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
    
    //Load dictionary
    string getdic = "SELECT dicpath,targetname,targetno FROM t7dictionary;";
    result res;
    bool flag = p_pgdb->pg_fetch_sql(getdic,res);
    if(flag == false) {
        delete p_pgdb;
        throw runtime_error("PG DB Execute Error.");
    }
    vector<string> dict_path;
    for (result::const_iterator it = res.begin(); it != res.end(); ++it) {
        if(it[0].as<string>().empty() || it[1].as<string>().empty() || it[2].as<string>().empty()){
            cerr << "Targetno is"<<it[2].as<string>()<<"Targetname is " << it[1].as<string>() << " Dicpath is" << it[0].as<string>();
            delete p_pgdb;
            p_pgdb = NULL;
            throw runtime_error("PG DB Data Error.");
        }
        pair<string, int> tmp = make_pair(it[1].as<string>(), it[2].as<int>());
        dict_path.push_back(it[0].as<string>()); //字典路径
        p_targetinf.push_back(tmp);//目标名称、序号
    }
    flag = p_SRClassify->LoadDic(dict_path);
    if(flag == false) {
        delete p_pgdb;
        p_pgdb = NULL;
        throw runtime_error("Load Dic Error.");
    }
    p_sparsity = std::atoi( argvMap["RETRIEVESPARSITY"].c_str() );
    p_min_residual = std::atof( argvMap["RETRIEVEMINRESIDUAL"].c_str() );
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
    string saveurl = g_ConfMap["RETRIEVEUSERIMGFEATUREDIR"] + filename + ".csv";
//    string saveurl = "/Users/liuguiyang/Documents/CodeProj/ConsoleProj/RetrieveServer/data/retrieve/feature/" + filename + ".csv";
    cout << "Save URL ## " << saveurl << endl;
    vector<vector<float>> imgFeatures;
    bool flag = AsiftFeature(saveurl, purl, imgFeatures);
    if(flag == false) {
        Log::Error("Fetch RetrieveServer Result Struct Failed !");
        obj.status = -1;
    }
    //Sparse Representation
    vector<int> srres;
    flag = p_SRClassify->SRClassify(imgFeatures, p_min_residual, p_sparsity, srres);
    if(flag == false || srres.size() != p_targetinf.size()) {
        Log::Error("Fetch RetrieveServer Result Struct Failed !");
        obj.status = -1;
    }

    //string res = "";
    for(vector<int>::iterator it = srres.begin(); it != srres.end(); it++){
        ImgInfo imginf;
        imginf.id = p_targetinf[*(it)].second;
        imginf.name = p_targetinf[*(it)].first;
        imginf.path = "";
        obj.keyWords.push_back(imginf);
        //res = res+to_string(imginf.id)+"|"
    }
    //Write result to PGDB
    //string insert = "INSERT INTO r5_user_search_img+"+\
    "(imgid, userid, datesearch, res, dateres)VALUES"+" ("\
    ++""\
    ++""\
    ++""\
    ++""\
    ++"";
    
    //flag = p_pgdb->pg_exec_sql(getdic);
    //if(flag == false) {        
    //    Log::Error("PG DB Execute Error.");
    //    obj.status = -1;
    //}
    //string targetname = p_targetname[res];
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
