#include "../include/file_watcher.hpp"

#include <sys/poll.h>

#include <boost/filesystem.hpp>

#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/LoggerFactory.hpp"
#include "../../util/include/string_formatter.hpp"
#include "../../util/include/lock_guard.hpp"

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

    pollfd poll_descriptor {inotify_descriptor, POLLIN, 0};

    while (client_.logged_in_) {
        // Timeout de 50ms
        int64_t poll_result = poll(&poll_descriptor, 1, 50);

        if (poll_result < 0) {
            throw std::runtime_error("Error monitoring changes on sync_dir folder");
        } else if (poll_result == 0) {
            continue;
        } else {
            int64_t modification_length = read(inotify_descriptor, buffer, EVENT_BUF_LEN);
            event_timestamp_ = std::time(nullptr);

            if (modification_length < 0)
                throw std::runtime_error("Error processing change on sync_dir folder");

            for (int64_t i = 0; i < modification_length; i += EVENT_SIZE + event->len) {
                event = reinterpret_cast<struct inotify_event *>(&buffer[i]);

                // Ignora eventos em diretórios e em arquivos ocultos
                if (event->len && !(event->mask & IN_ISDIR) && !dropbox_util::should_ignore_file(event->name)) {
                    if (event->mask & IN_CREATE) {
                        logger_->debug(static_cast<std::string>(StringFormatter()
                                << "File " << event->name << " created"));
                    } else if (event->mask & IN_CLOSE_WRITE) {
                        logger_->debug(static_cast<std::string>(StringFormatter()
                                << "File " << event->name << " written"));
                        AddModifiedFile(event->name);
                    } else if (event->mask & IN_DELETE) {
                        logger_->debug(static_cast<std::string>(StringFormatter()
                                << "File " << event->name << " deleted"));
                        AddModifiedFile(event->name);
                    } else if (event->mask & IN_MOVED_FROM) {
                        logger_->debug(static_cast<std::string>(StringFormatter()
                                << "File " << event->name << " moved out"));
                        AddModifiedFile(event->name);
                    } else if (event->mask & IN_MOVED_TO) {
                        logger_->debug(static_cast<std::string>(StringFormatter()
                                << "File " << event->name << " moved in"));
                        AddModifiedFile(event->name);
                    }
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
    logger_->debug("{} exists? {} timestamp {} last_write {}",
                   filename,
                   fs::exists(filepath),
                   modification_time,
                   fs::exists(filepath) ? fs::last_write_time(filepath) : 0);

    // Verifica se o arquivo já está na lista de arquivos do usuário
    LockGuard user_files_lock(client_.user_files_mutex_);
    auto user_files_iterator = std::find_if(client_.user_files_.begin(), client_.user_files_.end(),
            [&filename] (const util::file_info& info) -> bool {return filename == info.name;});

    // Se estiver e a versão do usuário é mais recente ignora a modificação
    if (user_files_iterator == client_.user_files_.end()
        || user_files_iterator->last_modification_time <= modification_time) {
        // Do contrário verifica se deve adicionar à lista de arquivos modificados
        user_files_lock.Unlock();
        LockGuard modified_files_lock(client_.modification_buffer_mutex_);
        auto modified_files_iterator = std::find_if(client_.modified_files_.begin(), client_.modified_files_.end(),
                [&filename] (const util::file_info& info) -> bool {return filename == info.name;});

        // Se o arquivo já está na lista de arquivos modificados com uma modificação mais recente ignora a atual
        if (modified_files_iterator == client_.modified_files_.end()
            || modified_files_iterator->last_modification_time <= modification_time) {
            // Senão atualiza a lista de modificados com a nova modificação
            util::remove_filename_from_list(filename, client_.modified_files_);
            client_.modified_files_.emplace_back(modified_file_info);
            modified_files_lock.Unlock();

            // Atualiza também a lista de arquivos do cliente
            user_files_lock.Lock();
            util::remove_filename_from_list(filename, client_.user_files_);
            client_.user_files_.emplace_back(modified_file_info);

            logger_->info(static_cast<std::string>(StringFormatter() << "Modification added: filename: " << filename
                                                                     << " size: " << file_size
                                                                     << " timestamp: "<< modification_time));
        }
    }
}
