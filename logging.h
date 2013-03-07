#ifndef SYSLOG_LOGGING
#   define SYSLOG_LOGGING

void set_msg_log_path(const char *path);
int msg_to_syslog_level(const int c);
void msg(const int level, const char *fmt, ...);

#endif
