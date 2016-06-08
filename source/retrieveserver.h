//
// Created by 贵阳 on 6/3/16.
//

#ifndef RETRIEVESERVER_RETRIEVESERVER_H
#define RETRIEVESERVER_RETRIEVESERVER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <Ice/Ice.h>

#include "rpc/retrieve/ImageRetrieveRpc.h"
#include "utils/log.h"
#include "threadpool/threadpool.h"
#include "algorithm/retrieve/utils/retrieveutils.h"
#include "utils/pgdb.h"
#include "algorithm/retrieve/detect/SR.h"

using namespace std;
using namespace RPCImgRecong;
using namespace Ice;

class RetrieveServer : public ImgRetrieval {
public:
    RetrieveServer();
    ~RetrieveServer();

public:
    void log_InputParameters(const DictStr2Str& mapArg);
    void log_OutputResult(const WordWiki& wiki);
    void log_OutputResult(const WordRes& wordres);
    void log_OutputResult(const ImgRes& imgres);

public:
    virtual WordWiki wordGetKnowledge(const string& word, const Ice::Current&);
    virtual WordRes wordSearch(const DictStr2Str& mapArg, const Ice::Current&);
    virtual ImgRes wordSearchImg(const DictStr2Str& mapArg, const Ice::Current&);
    virtual int imgSearchAsync(const DictStr2Str& mapArg, const Ice::Current&);
    virtual WordRes fetchImgSearchResult(const DictStr2Str& mapArg, const Ice::Current&);
    virtual WordRes imgSearchSync(const DictStr2Str& mapArg, const Ice::Current&);

private:
    ThreadPool* p_threadPool;
    PGDB* p_pgdb;
    SR<float> * p_SRClassify;
    int sparsity;       //稀疏表示：稀疏度
    float min_residual; //稀疏表示：最小残差
};


#endif //RETRIEVESERVER_RETRIEVESERVER_H
