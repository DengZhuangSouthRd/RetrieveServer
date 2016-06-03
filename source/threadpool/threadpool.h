#pragma once

#include <pthread.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <fstream>
#include <vector>
#include <errno.h>
#include <string.h>
#include <string>
#include <map>
#include <set>

#include "mutex.h"
#include "task.h"

#include "../utils/log.h"
#include "../utils/utils.h"

#include "../utils/json/json.h"
#include "../utils/json/json-forwards.h"

/*
 * This thread pool not manage the memory delete and free
 * Who Malloc and New, who should pay attention to the memory management !
*/

const int DEFAULT_POOL_SIZE = 5;
const int STARTED = 0;
const int STOPPED = 1;

using namespace std;
using namespace Json;

class ThreadPool {
public:
    static ThreadPool* getSingleInstance();
    static void revokeSingleInstance();

private:
    ThreadPool();
    ~ThreadPool();

private:
    static ThreadPool* p_ThreadPool;

public:
    void setPoolSize(const int pool_size);

public:
    int initialize_threadpool();
    int destroy_threadpool();

    int runningNumbers();
    int getPoolCapacity();
    int add_task(Task* task, const string& task_id);
    int fetchResultByTaskID(const string task_id, TaskPackStruct& res);

    void* execute_task(pthread_t thread_id);

private:
    int m_pool_size;
    int m_task_size;
    volatile int m_pool_state;
    ofstream *m_pOutLog;

    Mutex m_task_mutex;
    Mutex m_finishMap_mutex;
    Mutex m_taskMap_mutex;
    CondVar m_task_cond_var;

    std::vector<pthread_t> m_threads;
    std::set<pthread_t> m_run_threads;

    map<string, Task*> m_taskMap; // for <task_id, task*>
    map<string, TaskPackStruct> m_finishMap; // for <task_id, task_all_parameters>
    std::deque<Task*> m_tasks;
};

