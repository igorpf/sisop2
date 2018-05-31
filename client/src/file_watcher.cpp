#include "../include/file_watcher.hpp"

#include <sys/inotify.h>

#include <iostream>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

const std::string FileWatcher::LOGGER_NAME = "SyncThread";

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
    // TODO(jfguimaraes) Implement inotify correctly
    int length, i = 0;
    int fd;
    int wd;
    char buffer[EVENT_BUF_LEN];

    fd = inotify_init();

    if ( fd < 0 )
        throw std::runtime_error("Couldn't add file watcher to sync_dir folder");

    wd = inotify_add_watch( fd, client_.local_directory_.c_str(), IN_CREATE | IN_CLOSE_WRITE | IN_DELETE |
            IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF | IN_MOVE_SELF | IN_MODIFY);

    std::cout << "Watching directory: " << client_.local_directory_ << std::endl;

    while (client_.logged_in_) {
        std::cout << "Waiting for event... " << std::endl;

        i = 0;

        length = read(fd, buffer, EVENT_BUF_LEN);

        std::cout << "Processing new event of size: " << length << std::endl;

        if (length < 0)
            throw std::runtime_error("Error processing change on sync_dir folder");

        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];

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
                } else if (event->mask & IN_DELETE_SELF) {
                    throw std::runtime_error("sync_dir folder deleted");
                } else if (event->mask & IN_MOVE_SELF) {
                    throw std::runtime_error("sync_dir folder moved away from default location");
                } else if (event->mask & IN_MODIFY) {
                    if (event->mask & IN_ISDIR) {
                        std::cout << "Directory " << event->name << " was modified" << std::endl;
                    } else {
                        std::cout << "File " << event->name << " was modified" << std::endl;
                    }
                }
            }

            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch( fd, wd );
    close( fd );
}
