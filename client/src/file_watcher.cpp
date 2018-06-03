#include "../include/file_watcher.hpp"

#include <boost/filesystem.hpp>

#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/LoggerFactory.hpp"
#include "../../util/include/string_formatter.hpp"
#include "../../util/include/lock_guard.hpp"

#include <iostream>

namespace fs = boost::filesystem;
namespace util = dropbox_util;

const std::string FileWatcher::LOGGER_NAME = "FileWatcher";

FileWatcher::FileWatcher(IClient& client) : client_(client)
{
    logger_ = LoggerFactory::getLoggerForName(LOGGER_NAME);
}

FileWatcher::~FileWatcher()
{
    spdlog::drop(LOGGER_NAME);
}

void FileWatcher::Run()
{
    // TODO(jfguimaraes) Check file is valid (not hidden nor a backup one) and add to the buffer of modified files
    // TODO(jfguimaraes) First check if the modification is more recent than the registries in modified_list and user_file_list
    int64_t modification_length;
    int inotify_descriptor;
    int inotify_watcher;
    char buffer[EVENT_BUF_LEN];
    struct inotify_event* event;

    inotify_descriptor = inotify_init();

    if (inotify_descriptor < 0)
        throw std::runtime_error("Couldn't add file watcher to sync_dir folder");

    inotify_watcher = inotify_add_watch(inotify_descriptor, client_.local_directory_.c_str(),
            IN_CREATE | IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);

    logger_->debug(static_cast<std::string>(StringFormatter() << "Watching directory: " << client_.local_directory_));

    while (client_.logged_in_) {
        // TODO(jfguimaraes) Usar versão não bloqueante para poder finalizar o programa
        modification_length = read(inotify_descriptor, buffer, EVENT_BUF_LEN);
        event_timestamp_ = std::time(nullptr);

        if (modification_length < 0)
            throw std::runtime_error("Error processing change on sync_dir folder");

        for (int64_t i = 0; i < modification_length; i += EVENT_SIZE + event->len) {
            event = reinterpret_cast<struct inotify_event*>(&buffer[i]);

            if (event->len && !(event->mask & IN_ISDIR) && !dropbox_util::should_ignore_file(event->name)) {
                if (event->mask & IN_CREATE) {
                    logger_->debug(static_cast<std::string>(StringFormatter() << "File " << event->name << " created"));
                    std::cout << "New file " << event->name << " was created" << std::endl;
                    AddModifiedFile(event->name);
                } else if (event->mask & IN_CLOSE_WRITE) {
                    logger_->debug(static_cast<std::string>(StringFormatter() << "File " << event->name << " written"));
                    std::cout << "File " << event->name << " opened for writing was closed" << std::endl;
                    AddModifiedFile(event->name);
                } else if (event->mask & IN_DELETE) {
                    logger_->debug(static_cast<std::string>(StringFormatter() << "File " << event->name << " deleted"));
                    std::cout << "File " << event->name << " was deleted" << std::endl;
                    AddModifiedFile(event->name);
                } else if (event->mask & IN_MOVED_FROM) {
                    logger_->debug(static_cast<std::string>(StringFormatter() << "File " << event->name << " moved out"));
                    std::cout << "File " << event->name << " moved out of sync_dir" << std::endl;
                    AddModifiedFile(event->name);
                } else if (event->mask & IN_MOVED_TO) {
                    logger_->debug(static_cast<std::string>(StringFormatter() << "File " << event->name << " moved in"));
                    std::cout << "File " << event->name << " moved to sync_dir" << std::endl;
                    AddModifiedFile(event->name);
                }
            }
        }
    }

    inotify_rm_watch(inotify_descriptor, inotify_watcher);
    close(inotify_descriptor);
}

void FileWatcher::AddModifiedFile(const std::string &filename)
{
    // Se o arquivo existe obtém o modification time do arquivo, senão usa o timestamp do evento
    fs::path filepath {StringFormatter() << client_.local_directory_ << "/" << filename};
    std::time_t modification_time {fs::exists(filepath) ? fs::last_write_time(filepath) : event_timestamp_};

    // Se o arquivo existe obtém o seu tamanho
    int64_t file_size {fs::exists(filepath) ? static_cast<int64_t>(fs::file_size(filepath)) : 0};

    // Cria o file_info
    util::file_info modified_file_info {filename, file_size, modification_time};

    // TODO
    // Verifica se o arquivo já está na lista de arquivos do usuário
    // Se estiver e a versão do usuário é mais recente ignora a modificação
    // Do contrário verifica se deve adicionar à lista de arquivos modificados
    // Se o arquivo já está na lista de arquivos modificados com uma modificação mais recente ignora a atual
    // Se não atualiza a lista de modificados com a nova modificação
}
