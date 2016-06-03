//
// Created by 贵阳 on 6/3/16.
//

#include "retrieveserver.h"

RetrieveServer::RetrieveServer() {

}

RetrieveServer::~RetrieveServer() {

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
