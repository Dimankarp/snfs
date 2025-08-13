#ifndef SNFS_HTTP_H
#define SNFS_HTTP_H

#include <linux/inet.h>
#include <linux/net.h>

int64_t snfs_http_call(
    const char* token,
    const char* method,
    char* response_buffer,
    size_t buffer_size,
    size_t arg_size,
    ...
);

void encode(const char*, char*);

#endif  // SNFS_HTTP_H
