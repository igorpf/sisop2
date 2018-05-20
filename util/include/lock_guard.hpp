#ifndef SISOP2_UTIL_INCLUDE_LOCK_GUARD_HPP
#define SISOP2_UTIL_INCLUDE_LOCK_GUARD_HPP

#include <pthread.h>

/**
 * RAII wrapper para pthread_mutex
 * Trava o mutex no construtor e o libera no destrutor
 */
class LockGuard {
public:
    /**
     * Salva uma referência para o mutex e o trava
     */
    explicit LockGuard(pthread_mutex_t& mutex) : mutex_(mutex) {
        pthread_mutex_lock(&mutex_);
    }

    /**
     * Libera o mutex
     */
    ~LockGuard() {
        pthread_mutex_unlock(&mutex_);
    }

    /**
     * Desabilita o construtor por cópia
     */
    LockGuard(const LockGuard&) = delete;

    /**
     * Permite lock manual, na maioria dos casos não deve ser necessário
     */
    void Lock() {
        pthread_mutex_lock(&mutex_);
    }

    /**
     * Permite unlock manual, na maioria dos casos não deve ser necessário
     */
    void Unlock() {
        pthread_mutex_unlock(&mutex_);
    }

private:
    pthread_mutex_t& mutex_;
};

#endif // SISOP2_UTIL_INCLUDE_LOCK_GUARD_HPP
