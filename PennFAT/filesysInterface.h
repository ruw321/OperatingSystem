#ifndef FILESYS_INTERFACE_H
#define FILESYS_INTERFACE_H

int f_open(const char * fname, int mode);

int f_read(int fd, int n, char * buf);

int f_write(int fd, const char * str, int n);

int f_close(int fd);

int f_unlink(const char * fname);

int f_lseek(int fd, int offset, int whence);

int f_ls(const char * filename);

#endif