#include "../include/backup_sync_thread.hpp"

#include <chrono>
#include <thread>
#include <arpa/inet.h>

#include "../../util/include/lock_guard.hpp"
#include "../../util/include/string_formatter.hpp"

const std::string BackupSyncThread::LOGGER_NAME = "SyncThread";

BackupSyncThread::BackupSyncThread(bool& is_primary, pthread_mutex_t& socket_mutex, dropbox_util::SOCKET& socket,
                                   pthread_mutex_t& replica_managers_mutex, std::vector<dropbox_util::replica_manager>& replica_managers,
                                   pthread_mutex_t& clients_buffer_mutex, std::vector<dropbox_util::client_info>& clients_buffer) :
        is_primary_(is_primary), socket_mutex_(socket_mutex), socket_(socket),
        replica_managers_mutex_(replica_managers_mutex), replica_managers_(replica_managers),
        clients_buffer_mutex_(clients_buffer_mutex), clients_buffer_(clients_buffer), logger_(LOGGER_NAME) {}

void BackupSyncThread::Run()
{
    while (is_primary_) {
        try {
            std::this_thread::sleep_for(std::chrono::microseconds(sync_interval_in_microseconds_));

            if (!is_primary_)
                break;

            logger_->debug("Sync running\n");

            LockGuard replica_managers_lock(replica_managers_mutex_);
            LockGuard client_buffer_lock(clients_buffer_mutex_);

            if (!clients_buffer_.empty() && !replica_managers_.empty()) {
                replica_managers_lock.Unlock();

                std::string backup_command = StringFormatter() << "backup_sync" << dropbox_util::COMMAND_SEPARATOR_TOKEN;

                for (const auto &info : clients_buffer_) {
                    backup_command.append(info.user_id);

                    if (!info.user_devices.empty() or !info.user_files.empty())
                        backup_command.append(dropbox_util::USER_INFO_SEPARATOR_TOKEN);

                    if(!info.user_devices.empty()) {
                        backup_command.append(dropbox_util::DEVICE_INITIAL_TOKEN);
                        for (const auto &device : info.user_devices) {
                            backup_command.append(StringFormatter() << device.device_id << ","
                                                                    << device.ip << "," << device.port << ","
                                                                    << device.frontend_port << ":");
                        }
                        backup_command.pop_back();
                        backup_command.append(dropbox_util::DEVICE_FILE_SEPARATOR_TOKEN);
                    }

                    if (!info.user_files.empty()) {
                        backup_command.append(dropbox_util::FILE_INITIAL_TOKEN);
                        for (const auto &file : info.user_files) {
                            backup_command.append(StringFormatter() << file.name << ","
                                                                    << file.size << ","
                                                                    << file.last_modification_time << ":");
                        }
                        backup_command.pop_back();
                    }

                    backup_command.append(dropbox_util::USER_LIST_SEPARATOR_TOKEN);
                }

                backup_command.pop_back();
                logger_->info(backup_command);

                clients_buffer_.clear();

                client_buffer_lock.Unlock();
                replica_managers_lock.Lock();

                LockGuard socket_lock(socket_mutex_);

                for (auto &manager : replica_managers_) {
                    struct sockaddr_in replica_addr{0};
                    replica_addr.sin_family = AF_INET;
                    replica_addr.sin_port = htons(static_cast<uint16_t>(manager.port));
                    replica_addr.sin_addr.s_addr = inet_addr(manager.ip.c_str());

                    sendto(socket_, backup_command.c_str(), backup_command.size(), 0, (struct sockaddr *) &replica_addr,
                           sizeof(replica_addr));
                }
            }
        } catch (std::exception &exception) {
            logger_->error(exception.what());
        }
    }
}
