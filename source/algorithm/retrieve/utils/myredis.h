#ifndef MYREDIS_H
#define MYREDIS_H

#include <iostream>
#include <map>
#include <string>
#include <string.h>

using namespace std;

#include <hiredis/hiredis.h>

class MyRedis {
public:
    MyRedis();
    ~MyRedis();

public:
    void init(map<string, string> argvMap);
    void reconnect();
    void reconnect(map<string, string> argvMap);
    bool is_Working();

private:
    redisContext * m_pRedisContext;
    map<string, string> m_conf;
    bool m_isWorking;
};

#endif // MYREDIS_H
