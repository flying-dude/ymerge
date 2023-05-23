#pragma once

#include <stdio.h>

/// debugging helper

// https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
#include <string.h>
#define _FILE_ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define D printf("DBG :: [%s:%s():%d]\n", _FILE_, __func__, __LINE__);

// print debug message
#define DBG(msg, ...)                               \
  printf("DBG :: [%s] [%d] :: ", _FILE_, __LINE__); \
  printf(msg __VA_OPT__(, ) __VA_ARGS__);           \
  printf("\n");
