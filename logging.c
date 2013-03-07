/*
 * Simple logging features, inspired by earlier versions of libucw ( http://www.ucw.cz/libucw/ ) .
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include "logging.h"

static const char *log_path = NULL;

void set_msg_log_path(const char *path) {
    log_path = path;
}

static int open_log_file() {
    if (NULL == log_path) {
        return -1;
    }
    const int fd = open(log_path, O_CREAT | O_APPEND | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (-1 == fd) {  // This is not necessary, but I find code more readable
        return -1;
    }
    return fd;
}

int msg_to_syslog_level(const int c) {
    switch (c) {
        case 'I':
            return LOG_INFO;
        case 'w':
        case 'W':
            return LOG_WARNING;
        case 'e':
        case 'E':
            return LOG_ERR;
        case 'c':
            return LOG_CRIT;
        case 'C':
            return LOG_CRIT;
        case 'D':
            return LOG_DEBUG;
        default:
            return LOG_INFO;
    }
}

void msg(const int level, const char *fmt, ...) {
    static const size_t time_part_size = (4 + 2 + 2) + 1 + (2 + 1 + 2 + 1 + 2); // YYYYMMDD HH:MM:SS

    const int file_fd = open_log_file();
    const int fd = file_fd >= 0 ? file_fd : 2;

    va_list lst, lst2;
    va_start(lst, fmt);
    va_copy(lst2, lst);

    const int isize = vsnprintf(NULL, 0, fmt, lst);
    const intmax_t pid = (intmax_t) getpid();
    const size_t pid_part_size = snprintf(NULL, 0, "[%jd]", pid);

    char buf[1 + 1 + time_part_size + 1 + pid_part_size + 1 + isize + 1];

    size_t sz = 0;
    buf[0] = level;
    buf[1] = '\t';
    sz = 2;

    time_t timer;
    struct tm tm_info;
    strftime(&buf[sz], time_part_size + 1, "%Y%m%d %H:%M:%S", localtime_r((time(&timer), &timer), &tm_info) );
    sz += time_part_size;

    buf[sz] = ' ';
    ++sz;

    sprintf(&buf[sz], "[%jd]", pid);
    sz += pid_part_size;

    buf[sz] = '\t';
    ++sz;

    vsnprintf(&buf[sz], isize + 1, fmt, lst2);
    sz += isize;

    buf[sz] = '\n';
    ++sz;

    va_end(lst);
    va_end(lst2);

    assert(sz == sizeof(buf));

    for (size_t i = 0; i < sizeof(buf); ) {
        const ssize_t ret = write(fd, buf, sz);
        if (ret < 0) {
            return;
        }
        assert(ret > 0);
        i += (size_t)ret;
    }

    if (file_fd >= 0) {
        close(file_fd);
    }
}
