/*=============================================================================
#     FileName: driver.h
#         Desc: global header
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-09 08:59:54
=============================================================================*/
#ifndef DRIVER
#define DRIVER

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define DEBUG 0
#define LOG_OUTPUT 0
#define LOG_ONLY_CLIENT 0

#define CLIENT_COUNT 200
#define BUFFER_LEN 256

void segfault_handler(int);

#endif
