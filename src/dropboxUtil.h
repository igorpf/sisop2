#ifndef SISOP2_DROPBOXUTIL_H
#define SISOP2_DROPBOXUTIL_H

#define MAXNAME 255
#define MAXFILES 255

struct file_info {
    char name[MAXNAME];
    char extension[MAXNAME];
    char last_modified[MAXNAME];
    int size;
};

#endif
