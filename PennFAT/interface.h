#ifndef FILESYS_INTERFACE_H
#define FILESYS_INTERFACE_H

#define F_STDIN_FD 0
#define F_STDOUT_FD 1
#define F_ERROR 2

#define F_WRITE 0
#define F_READ 1
#define F_APPEND 2

#define F_SEEK_SET 0
#define F_SEEK_CUR 1
#define F_SEEK_END 2

#define F_SUCCESS 0
#define F_FAILURE -1

#include "../kernel/utils.h"

int f_open(const char *fname, int mode);

int f_close(int fd);

int f_read(int fd, int n, char *buf);

int f_write(int fd, const char *str, int n);

int f_lseek(int fd, int offset, int whence);

int f_unlink(const char *fname);

int f_ls(const char *filename);

bool f_find(const char *filename);

bool f_isExecutable(const char *filename);

#endif