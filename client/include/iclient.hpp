#ifndef SISOP2_CLIENT_INCLUDE_ICLIENT_HPP
#define SISOP2_CLIENT_INCLUDE_ICLIENT_HPP

#include <string>
#include <vector>

/**
 * Interface genérica para um objeto do tipo cliente
 * Usada para facilitar o teste de módulos que interagem com a classe Client
 */
class IClient {
public:
    virtual ~IClient() = default;

    virtual void sync_client() = 0;

    virtual void send_file(const std::string &filename) = 0;
    virtual void get_file(const std::string &filename) = 0;
    virtual void delete_file(const std::string& filename) = 0;

    virtual std::vector<std::vector<std::string>> list_server() = 0;
    virtual std::vector<std::vector<std::string>> list_client() = 0;

    virtual void close_session() = 0;
};

#endif // SISOP2_CLIENT_INCLUDE_ICLIENT_HPP
