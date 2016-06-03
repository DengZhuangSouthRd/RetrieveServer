#include "threadpool.h"

extern map<string, string> g_ConfMap;

ThreadPool* ThreadPool::p_ThreadPool = new ThreadPool();
ThreadPool* ThreadPool::getSingleInstance() {
    return p_ThreadPool;
}

void ThreadPool::revokeSingleInstance() {
    delete p_ThreadPool;
    p_ThreadPool = NULL;
}

ThreadPool::ThreadPool() : m_pool_size(DEFAULT_POOL_SIZE), m_task_size(DEFAULT_POOL_SIZE*1.5) {

    m_threads.clear();
    m_run_threads.clear();

    if(initialize_threadpool() != 0) {
        cerr << "Initialize ThreadPool failed !, Please Check and restart the Service !" << endl;
        Log::Error("Failed to initialize the Thread Pool !");
        throw runtime_error("Initialize ThreadPool failed !, Please Check and restart the Service !");
    }
}

ThreadPool::~ThreadPool() {
    if (m_pool_state != STOPPED) {
        Log::Info("~ ThreadPool and Still Running !");
        cout << "~ ThreadPool and Still Running !" << endl;
        destroy_threadpool();
    }
}

void ThreadPool::setPoolSize(const int pool_size) {
    m_pool_size = pool_size;
    m_task_size = m_pool_size * 1.5;
}

// We can't pass a member function to pthread_create.
// So created the wrapper function that calls the member function
// we want to run in the thread.
extern "C" {

void* start_thread(void* arg) {
    ThreadPool* tp = (ThreadPool*) arg;
    pthread_t tid = pthread_self();
    tp->execute_task(tid);
    return NULL;
}

}

int ThreadPool::initialize_threadpool() {
    m_pool_state = STARTED;
    int ret = -1;
    for(int i=0; i<m_pool_size; ++i) {
        pthread_t tid;
        ret = pthread_create(&tid, NULL, start_thread, (void*) this);
        if (ret != 0) {
            Log::Error("pthread_create() failed %d !", ret);
            cerr << "pthrad_create() failed " << ret << endl;
            return -1;
        }
        m_threads.push_back(tid);
    }
    return 0;
}

int ThreadPool::destroy_threadpool() {
    m_task_mutex.lock();
    m_pool_state = STOPPED;
    m_task_mutex.unlock();
    cout << "Broadcasting STOP signal to all threads..." << endl;
    Log::Info("Broadcasting STOP signal to all threads...");
    m_task_cond_var.broadcast(); // notify all threads we are shttung down

    for(set<pthread_t>::iterator it=m_run_threads.begin(); it!=m_run_threads.end();++it) {
        void * result;
        pthread_join(*it, &result);
        m_task_cond_var.broadcast(); // try waking up a bunch of threads that are still waiting
    }

    return 0;
}

int ThreadPool::runningNumbers() {
    return m_tasks.size();
}

int ThreadPool::getPoolCapacity() {
    return m_pool_size;
}

void* ThreadPool::execute_task(pthread_t thread_id) {
    Task* task = NULL;
    while(true) {
        // Try to pick a task
        m_task_mutex.lock();
        while((m_pool_state != STOPPED) && (m_tasks.empty())) {
            m_task_cond_var.wait(m_task_mutex.get_mutex_ptr());
        }

        // If the thread was woken up to notify process shutdown, return from here
        if(m_pool_state == STOPPED) {
            m_task_mutex.unlock();
            pthread_exit(NULL);
        }

        cout << "Residue the task numebr " << m_tasks.size() << endl;
        Log::Info("Residue the task numebr %d !", m_tasks.size());

        task = m_tasks.front();
        m_tasks.pop_front();
        if(m_run_threads.count(thread_id) == 0) {
            m_run_threads.insert(thread_id);
        } else {
            Log::Error("Thread id %d has already run !", thread_id);
            cerr << "Thread id " << thread_id << " has already run !" << endl;
        }
        m_task_mutex.unlock();

        task->run();
        string tmp_id = task->getTaskID();
        TaskPackStruct tmp_SaveTask;
        task->packTaskStaticStatus(tmp_SaveTask);
        if(TASKCOMPELETE == task->getTaskStatus()) {
            m_finishMap_mutex.lock();
                    m_finishMap[tmp_id] = tmp_SaveTask;
                    Log::Info("Finish Task size is %d !", m_finishMap.size());
            m_finishMap_mutex.unlock();
        } else {
            Log::Error("TaskID %s RunStatus Failed !", tmp_id.c_str());
        }
        m_taskMap_mutex.lock();
            delete task;
            m_taskMap[tmp_id] = NULL;
            m_taskMap.erase(tmp_id);
            Log::Info("TaskID %s have removed from TaskMap !", tmp_id.c_str());
        m_taskMap_mutex.unlock();

        m_run_threads.erase(thread_id);
    }
    return NULL;
}

int ThreadPool::add_task(Task* task, const string &task_id) {
    m_task_mutex.lock();
    task->setTaskID(task_id);
    m_tasks.push_back(task);
    Log::Info("Now the task size is %d !", m_tasks.size());
    m_taskMap[task_id] = task;
    // wake up one thread that is waiting for a task to be available
    m_task_cond_var.signal();
    m_task_mutex.unlock();
    return 0;
}

int ThreadPool::fetchResultByTaskID(const string task_id, TaskPackStruct& res) {
    // first step find in m_finishMap, if not in this Map
    // erase the finishMap[key] from the Map
    // second step find in m_taskMap, search it process status

    if(m_finishMap.count(task_id) != 0) {
        res.input = m_finishMap.at(task_id).input;
        res.output = m_finishMap.at(task_id).output;

        m_finishMap_mutex.lock();
                m_finishMap.erase(task_id);
        m_finishMap_mutex.unlock();

        return 1;
    } else if(m_taskMap.count(task_id) != 0) {
        Log::Info("Fetch task id %s not finished !", task_id.c_str());
        return 0; // running
    } else {
        Log::Error("Fetch task_id %s have not been push to this pool !", task_id.c_str());
        return -1; // not in
    }
    return false;
}

