#include "mutex.h"

Mutex::Mutex() {
    pthread_mutex_init(&m_lock, NULL);
    is_locked = false;
}

Mutex::~Mutex() {
    while(is_locked);
    unlock(); // Unlock Mutex after shared resource is safe
    pthread_mutex_destroy(&m_lock);
}

void Mutex::lock() {
    pthread_mutex_lock(&m_lock);
    is_locked = true;
}

void Mutex::unlock() {
    is_locked = false; // do it BEFORE unlocking to avoid race condition
    pthread_mutex_unlock(&m_lock);
}

pthread_mutex_t* Mutex::get_mutex_ptr(){
    return &m_lock;
}

CondVar::CondVar() {
    pthread_cond_init(&m_cond_var, NULL);
}

CondVar::~CondVar() {
    pthread_cond_destroy(&m_cond_var);
}

void CondVar::wait(pthread_mutex_t* mutex) {
    pthread_cond_wait(&m_cond_var, mutex);
}

void CondVar::signal() {
    pthread_cond_signal(&m_cond_var);
}

void CondVar::broadcast() {
    pthread_cond_broadcast(&m_cond_var);
}
