#pragma once

#include <pthread.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <vector>
#include <errno.h>
#include <string.h>
#include <string>

#include "../algorithm/fusion/PgInf.h"
#include "../utils/utils.h"

using namespace std;

//!!! Attention! This Task charge off the the memory management,
//!!! to delete the all heap memory about the task

class Task {
public:
    Task(void* (*fn_ptr)(void*), void* arg);
    ~Task();

public:
    void run();

public:
    void setTaskID(string task_id);
    const string getTaskID();
    pthread_t getThreadID();
    TASKSTATUS getTaskStatus();

    /*
     * pack the task input and output parameters
     * only pack the void* input and void* output parameters
    */
    void packTaskStaticStatus(TaskPackStruct& res);

private:
    void* (*m_fn_ptr)(void*);
    void* m_arg;
    void* m_result;

private:
    string m_task_id;
    TASKSTATUS m_status;

};
