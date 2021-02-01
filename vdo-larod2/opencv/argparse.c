/**
 * Copyright 2018 Axis Communications
 * SPDX-License-Identifier: Apache-2.0
 */

#include "argparse.h"

#include <argp.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "utils.h"
#include "version.h"

#define KEY_USAGE (127)

/**
 * @brief Parses a single input key and saves to state.
 *
 * In the case where this is not called by the interactive client this function
 * will call @c argp_failure() with EXIT_FAILURE if an input is deemed invalid
 * or something goes wrong. If the interactive client is used however it will
 * simply return an error code since the interactive client doesn't exit upon
 * argp_failures.
 *
 * @param key Describing which input key has been used.
 * @param arg Pointer to the value assigned to input referenced by @p key.
 * @param state Pointer to argparse state (including inputs state).
 * @return Positive errno style return code (zero means success).
 */
static int parseOpt(int key, char* arg, struct argp_state* state);

/**
 * @brief Parses a string as a non-negative unsigned long long.
 *
 * @param arg String to parse.
 * @param i Pointer to the number being the result of parsing.
 * @param limit Max limit for data type integer will be saved to.
 * @return Positive errno style return code (zero means success).
 */
static int parseNonNegInt(char* arg, unsigned long long* i,
                          unsigned long long limit);

const struct argp_option opts[] = {
    {"debug", 'd', NULL, 0, "Enable debug output.", 0},
    {"model-file", 'g', "FILE", 0, "Load model file.", 0},
    {"width", 'x', "WIDTH", 0, "Video stream width.", 0},
    {"height", 'y', "HEIGHT", 0, "Video stream height.", 0},
    {"help", 'h', NULL, 0, "Print this help text and exit.", 0},
    {"usage", KEY_USAGE, NULL, 0, "Print short usage message and exit.", 0},
    {"version", 'v', NULL, 0, "Print version information and exit.", 0},
    {0}};

/**
 * @brief Used by Argp interface to indicate who maintains this code.
 *
 * This is a global variable declared by Argp, used to indicate to end users
 * of this application who to report bugs to.
 */
const char* argp_program_bug_address = BUG_REPORT_ADDRESS;

const struct argp argp = {
    opts, parseOpt, "OPTION", "\nlarod example using opencv", NULL, NULL, NULL};

bool parseArgs(int argc, char** argv, args_t* args) {
    int ret = argp_parse(&argp, argc, argv, ARGP_NO_EXIT | ARGP_NO_HELP, NULL,
                         args);
    if (ret) {
        return false;
    }

    return true;
}

int parseOpt(int key, char* arg, struct argp_state* state) {
    args_t* args = state->input;
    switch (key) {
    case 'd':
        args->debug = true;
        break;
    case 'g':
        args->modelFile = arg;
        break;
    case 'j': {
        unsigned long long chip;
        int ret = parseNonNegInt(arg, &chip, INT_MAX);
        if (ret) {
            argp_failure(state, EXIT_FAILURE, ret, "invalid chip value");

            return ret;
        } else if (chip == 0) {
            argp_failure(state, EXIT_FAILURE, EINVAL,
                         "chip value must be greater than 0");

            return EINVAL;
        }
        args->chip = (larodChip) chip;
        break;
    }
    case 'x': {
        unsigned long long width;
        int ret = parseNonNegInt(arg, &width, INT_MAX);
        if (ret) {
            argp_failure(state, EXIT_FAILURE, ret, "invalid width value");

            return ret;
        } else if (width == 0) {
            argp_failure(state, EXIT_FAILURE, EINVAL,
                         "width value must be greater than 0");

            return EINVAL;
        }
        args->width = (size_t) width;
        break;
    }
    case 'y': {
        unsigned long long height;
        int ret = parseNonNegInt(arg, &height, INT_MAX);
        if (ret) {
            argp_failure(state, EXIT_FAILURE, ret, "invalid height value");

            return ret;
        } else if (height == 0) {
            argp_failure(state, EXIT_FAILURE, EINVAL,
                         "height value must be greater than 0");

            return EINVAL;
        }
        args->height = (size_t) height;
        break;
    }
    case 'h':
        argp_state_help(state, stdout, ARGP_HELP_STD_HELP);
        exit(0);
        break;
    case KEY_USAGE:
        argp_state_help(state, stdout, ARGP_HELP_USAGE | ARGP_HELP_EXIT_OK);
        break;
    case ARGP_KEY_ARG:
        argp_error(state, "too many arguments");

        return EINVAL;
    case ARGP_KEY_INIT:
        args->modelFile = NULL;
        args->chip = LAROD_CHIP_INVALID;
#ifdef DEBUG
        args->debug = true;
#else
        args->debug = false;
#endif
        args->width = 1280;
        args->height = 720;
        break;
    case ARGP_KEY_END:
        if (state->argc == 1) {
            argp_error(state, "no options given");
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static int parseNonNegInt(char* arg, unsigned long long* i,
                          unsigned long long limit) {
    char* endPtr;
    *i = strtoull(arg, &endPtr, 0);
    if (*endPtr != '\0') {
        return EINVAL;
    } else if (arg[0] == '-') {
        return EINVAL;
    } else if (*i == ULLONG_MAX || *i > limit) {
        return ERANGE;
    }

    return 0;
}
