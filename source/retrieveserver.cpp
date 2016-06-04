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

}

void RetrieveServer::log_OutputResult(const WordWiki &wiki) {

}

void RetrieveServer::log_OutputResult(const WordRes &wordres) {

}

void RetrieveServer::log_OutputResult(const ImgRes &imgres) {

}

WordWiki RetrieveServer::wordGetKnowledge(const string &word, const Ice::Current &) {

}

WordRes RetrieveServer::wordSearch(const DictStr2Str &mapArg, const Ice::Current &) {

}

ImgRes RetrieveServer::wordSearchImg(const DictStr2Str &mapArg, const Ice::Current &) {

}

ImgRes RetrieveServer::imgSearchSync(const DictStr2Str &mapArg, const Ice::Current &) {

}

int RetrieveServer::imgSearchAsync(const DictStr2Str &mapArg, const Ice::Current &) {

}

ImgRes RetrieveServer::fetchImgSearchResult(const DictStr2Str &mapArg, const Ice::Current &) {

}
