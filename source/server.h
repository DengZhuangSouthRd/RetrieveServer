#ifndef _SERVER_H
#define _SERVER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include <signal.h>
#include <stdio.h>

#include <Ice/Ice.h>
#include "utils/log.h"

using namespace std;

class Server {
public:
    Server();
    ~Server();

public:
    void initRpc(int argc, char** argv, string conn);
    void restart();
    void close();

private:
    int status;
    Ice::CommunicatorPtr ic;
    int m_argc;
    char** m_argv;
    string connParam;
    Ice::ObjectAdapterPtr adapter;
};

#endif
