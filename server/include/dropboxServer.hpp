#ifndef SISOP2_SERVER_INCLUDE_DROPBOXSERVER_H
#define SISOP2_SERVER_INCLUDE_DROPBOXSERVER_H

#include <string>

/**
 * Sincroniza o servidor com o diretório "sync_dir_<user_id>"
 * TODO(jfguimaraes) Como a função sabe o user_id?
 */
void sync_server();

/**
 * Recebe um arquivo do cliente (upload)
 * @param filename Nome do arquivo a ser recebido
 * TODO(jfguimaraes) Onde salvar o arquivo? Sempre no diretório root? Ou recebe essa informação do cliente?
 */
void receive_file(const std::string& filename);

/**
 * Envia um arquivo para o cliente (download)
 * @param filename Nome do arquivo a ser enviado
 * TODO(jfguimaraes) Acho que deveríamos utilizar um campo fonte e um destino pra todas as transferências de arquivos
 */
void send_file(const std::string& filename);


//test functions
void start_server();

#endif // SISOP2_SERVER_INCLUDE_DROPBOXSERVER_H
