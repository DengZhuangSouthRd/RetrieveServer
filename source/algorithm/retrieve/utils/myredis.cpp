#include "myredis.h"

MyRedis::MyRedis() {
    m_pRedisContext = NULL;
    m_isWorking = false;
}

MyRedis::~MyRedis() {
    if(m_pRedisContext != NULL) {
        redisFree(m_pRedisContext);
        m_pRedisContext = NULL;
    }
}

void MyRedis::init(map<string, string> argvMap) {
    for(map<string, string>::iterator it = argvMap.begin();it != argvMap.end();++it) {
        m_conf[it->first] = it->second;
    }
    if(m_pRedisContext != NULL) {
        redisFree(m_pRedisContext);
        m_pRedisContext = NULL;
    }

    m_pRedisContext = redisConnect(m_conf["host"].c_str(), atoi(m_conf["port"].c_str()));
    if(m_pRedisContext->err) {
        cerr << "Redis Connect Error " << m_pRedisContext->errstr << endl;
        m_isWorking = false;
    }
    m_isWorking = true;
    if(m_conf.find("passwd") != m_conf.end()) {
        string rql = "auth " + m_conf["passwd"];
        redisReply* reply = (redisReply*)redisCommand(m_pRedisContext, rql.c_str());
        cout << "Auth Password Status " << reply->str << ", Str Size is " << reply->len << endl;
        if(strcmp(reply->str, "OK") != 0) {
            cerr << "Redis Auth Error ! Please Check the password !" << endl;
            m_isWorking = false;
        }
        freeReplyObject(reply);
    }
}

void MyRedis::reconnect() {
    init(m_conf);
}

void MyRedis::reconnect(map<string, string> argvMap) {
    init(argvMap);
}

bool MyRedis::is_Working() {
    return m_isWorking;
}

