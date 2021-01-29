/**
 * Copyright 2018 Axis Communications
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "larod.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct args_t {
    char* modelFile;
    larodChip chip;
    bool hasModelParams;
    bool debug;
    bool runInference;
    size_t width;
    size_t height;
} args_t;

/**
 * @brief Parses command line arguments.
 *
 * This allocates necessary members in @c args_t.
 *
 * @param argc argc from main(int argc, char** argv).
 * @param argv argv from main(int argc, char** argv).
 * @param args Pointer to @c args_t. This will be filled in and allocated after
 * a successful call. Deallocate it with @c destroyArgs() when no longer needed.
 * not. If false, then exit() will be called upon parsing errors (otherwise
 * not).
 * @return Returns true if no errors occurred, otherwise false.
 */
bool parseArgs(int argc, char** argv, args_t* args);

#ifdef __cplusplus
}
#endif
