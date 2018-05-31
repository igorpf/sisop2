#include "../include/file_watcher.hpp"

#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/string_formatter.hpp"

#include <iostream>

const std::string FileWatcher::LOGGER_NAME = "FileWatcher";

FileWatcher::FileWatcher(IClient& client) : client_(client)
{
    // TODO This logger should output to a file
    logger_ = spdlog::stdout_color_mt(LOGGER_NAME);
}

FileWatcher::~FileWatcher()
{
    spdlog::drop(LOGGER_NAME);
}

void FileWatcher::Run()
{
    // TODO(jfguimaraes) Check file is valid (not hidden nor a backup one) and add to the buffer of modified files
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
        modification_length = read(inotify_descriptor, buffer, EVENT_BUF_LEN);

        if (modification_length < 0)
            throw std::runtime_error("Error processing change on sync_dir folder");

        for (int64_t i = 0; i < modification_length; i += EVENT_SIZE + event->len) {
            event = reinterpret_cast<struct inotify_event*>(&buffer[i]);

            if (event->len && !(event->mask & IN_ISDIR) && !dropbox_util::should_ignore_file(event->name)) {
                if (event->mask & IN_CREATE) {
                    logger_->debug(static_cast<std::string>(StringFormatter() << "File " << event->name << " created"));
                    std::cout << "New file " << event->name << " was created" << std::endl;
                } else if (event->mask & IN_CLOSE_WRITE) {
                    logger_->debug(static_cast<std::string>(StringFormatter() << "File " << event->name << " written"));
                    std::cout << "File " << event->name << " opened for writing was closed" << std::endl;
                } else if (event->mask & IN_DELETE) {
                    logger_->debug(static_cast<std::string>(StringFormatter() << "File " << event->name << " deleted"));
                    std::cout << "File " << event->name << " was deleted" << std::endl;
                } else if (event->mask & IN_MOVED_FROM) {
                    logger_->debug(static_cast<std::string>(StringFormatter() << "File " << event->name << " moved out"));
                    std::cout << "File " << event->name << " moved out of sync_dir" << std::endl;
                } else if (event->mask & IN_MOVED_TO) {
                    logger_->debug(static_cast<std::string>(StringFormatter() << "File " << event->name << " moved in"));
                    std::cout << "File " << event->name << " moved to sync_dir" << std::endl;
                }
            }
        }
    }

    inotify_rm_watch(inotify_descriptor, inotify_watcher);
    close(inotify_descriptor);
}
