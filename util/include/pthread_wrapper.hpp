#ifndef SISOP2_UTIL_INCLUDE_PTHREAD_WRAPPER_HPP
#define SISOP2_UTIL_INCLUDE_PTHREAD_WRAPPER_HPP

#include <pthread.h>

/**
 * Classe que encapsula a gerência de uma thread utilizando a biblioteca pthread
 *
 * Para utilizá-la deve-se criar uma subclasse e implementar o método Run
 */
class PThreadWrapper {
public:
    PThreadWrapper() = default;
    virtual ~PThreadWrapper() = default;

    /**
     * Método que inicializa a thread
     * @return Indica se a thread foi iniciada com sucesso
     */
    bool Start()
    {
        return pthread_create(&thread_, nullptr, Run, this) == 0;
    }

    /**
     * Aguarda o fim da thread
     */
    void Join()
    {
        pthread_join(thread_, nullptr);
    }

protected:
    /**
     * Método que deve ser implementado pela subclasse para ser executado no loop da thread
     */
    virtual void Run() = 0;

private:
    /**
     * Wrapper para a chamada da função de loop
     */
    static void* Run(void* This) {
        static_cast<PThreadWrapper*>(This)->Run();
        return nullptr;
    }

    /// Objeto pthread interno
    pthread_t thread_;
};

#endif // SISOP2_UTIL_INCLUDE_PTHREAD_WRAPPER_HPP
