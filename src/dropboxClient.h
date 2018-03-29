#ifndef SISOP2_DROPBOXCLIENT_H
#define SISOP2_DROPBOXCLIENT_H

struct	client	{
    int devices[2];
    char userid[MAXNAME];
    struct file_info file_info[MAXFILES];
    int logged_in;
};

int login_server(char *host, int port);
void sync_client();
void send_file(char *file);
void get_file(char *file);
void delete_file(char *file);
void close_session();

#endif
