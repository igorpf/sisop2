#include "../include/file_watcher.hpp"

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

    std::cout << "Watching directory: " << client_.local_directory_ << std::endl;

    while (client_.logged_in_) {
        std::cout << "Waiting for event... " << std::endl;

        modification_length = read(inotify_descriptor, buffer, EVENT_BUF_LEN);

        std::cout << "Processing new event of size: " << modification_length << std::endl;

        if (modification_length < 0)
            throw std::runtime_error("Error processing change on sync_dir folder");

        for (int64_t i = 0; i < modification_length; i += EVENT_SIZE + event->len) {
            event = reinterpret_cast<struct inotify_event*>(&buffer[i]);

            if (event->len) {
                if (event->mask & IN_CREATE) {
                    if (event->mask & IN_ISDIR) {
                        std::cout << "New directory " << event->name << " was created" << std::endl;
                    } else {
                        std::cout << "New file " << event->name << " was created" << std::endl;
                    }
                } else if (event->mask & IN_CLOSE_WRITE) {
                    std::cout << "File " << event->name << " opened for writing was closed" << std::endl;
                } else if (event->mask & IN_DELETE) {
                    if (event->mask & IN_ISDIR) {
                        std::cout << "Directory " << event->name << " was deleted" << std::endl;
                    } else {
                        std::cout << "File " << event->name << " was deleted" << std::endl;
                    }
                } else if (event->mask & IN_MOVED_FROM) {
                    std::cout << "File " << event->name << " moved out of sync_dir" << std::endl;
                } else if (event->mask & IN_MOVED_TO) {
                    std::cout << "File " << event->name << " moved to sync_dir" << std::endl;
                }
            }
        }
    }

    inotify_rm_watch(inotify_descriptor, inotify_watcher);
    close(inotify_descriptor);
}
