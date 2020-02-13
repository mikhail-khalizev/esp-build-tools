#pragma once

extern "C" {

    // TODO https://github.com/espressif/esp-idf/issues/1660#issuecomment-370279500.

    typedef unsigned int fsblkcnt_t;
    typedef unsigned int fsfilcnt_t;

    struct statvfs
    {
        unsigned int  f_bsize;    /* Filesystem block size */
        unsigned int  f_frsize;   /* Fragment size */
        fsblkcnt_t     f_blocks;  /* Size of fs in f_frsize units */
        fsblkcnt_t     f_bfree;   /* Number of free blocks */
        fsblkcnt_t     f_bavail;  /* Number of free blocks for unprivileged users */
        fsfilcnt_t     f_files;   /* Number of inodes */
        fsfilcnt_t     f_ffree;   /* Number of free inodes */
        fsfilcnt_t     f_favail;  /* Number of free inodes for unprivileged users */
        unsigned int  f_fsid;     /* Filesystem ID */
        unsigned int  f_flag;     /* Mount flags */
        unsigned int  f_namemax;  /* Maximum filename length */
    };

    int __attribute__((weak)) statvfs(const char *path, struct statvfs *buf)
    { return -1; }

    int __attribute__((weak)) lstat(const char *pathname, struct stat *statbuf)
    { return -1; }
}
