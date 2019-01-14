/** \file
 *  \brief Main function for rngminder
 *  \author Olaf Mandel <o.mandel@menlosystems.com>
 *  \copyright Copyright 2019  MenloSystems GmbH
 *  \par License
 *  All rights reserved.
 */
#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/** \brief Default name of the file to load from / store to
 */
const char dflt_filename[] = LOCALSTATEDIR "/lib/urandom/random-seed";


/** \brief Parse command line options
 *  \param[in]  argc Number of entries in `argv`
 *  \param[in]  argv Array of command line arguments
 *  \param[out] help     Show help-screen
 *  \param[out] version  Show version information
 *  \param[out] load     Load pool from a file
 *  \param[out] store    Store pool to a file
 *  \param[out] poolsize Number of bits to store or -1 for auto detection
 *  \param[out] filename Name of the file to load from / store to
 *  \return `true` on error, `false` on success
 *
 *  If an error is returned, an error message was already printed to
 *  stderr.
 */
bool parse_opts(int argc, char* argv[], bool *help, bool *version, bool *load,
    bool *store, int *poolsize, const char **filename)
{
    bool keep = false;
    bool hadCmd = false;

    const char optstring[] = "-:b:f:hkv";
    const struct option longopts[] = {
        {"bits",    required_argument, 0, 'b'},
        {"file",    required_argument, 0, 'f'},
        {"help",    no_argument,       0, 'h'},
        {"keep",    no_argument,       0, 'k'},
        {"version", no_argument,       0, 'v'},
        {NULL,      0,                 0, '\0'}
    };
    opterr = 1; // We do error messages outselves

    *help = false;
    *version = false;
    *load = false;
    *store = false;
    *poolsize = -1;
    *filename = dflt_filename;

    for (;;) {
        int opt = getopt_long(argc, argv, optstring, longopts, 0);
        if (opt == -1)
            break;

        switch (opt) {
        case 'b':
            *poolsize = atoi(optarg);
            if (*poolsize <= 0) {
                fprintf(stderr, "Not a valid number of bits: \"%s\"\n",
                    optarg);
                return true;
            }
            break;
        case 'f':
            *filename = optarg;
            break;
        case 'h':
            *help = true;
            break;
        case 'k':
            keep = true;
            break;
        case 'v':
            *version = true;
            break;
        case '\1':  // Only if optstring starts with '-'
            if (hadCmd) {
                fprintf(stderr, "Invalid additional command: \"%s\"\n",
                    optarg);
                return true;
            }
            if (strcmp(optarg, "load") == 0) {
                *load = true;
            } else if (strcmp(optarg, "store") == 0) {
                *store = true;
            } else {
                fprintf(stderr, "Invalid non-option: \"%s\"\n", optarg);
                return true;
            }
            hadCmd = true;
            break;
        case ':':  // Only if optstring has ':' at beginning
            fprintf(stderr, "Missing required argument to option: \"-%c\"\n",
                optopt);
            return true;
            break;
        case '?':
            if (optopt) {
                fprintf(stderr, "Unknown option: \"-%c\"\n", optopt);
            } else {
                fprintf(stderr, "Unknown or ambiguous long option: \"%s\"\n",
                     argv[optind-1]);
            }
            return true;
            break;
        default:
            assert(false);
        }
    }

    if (*help) {
        return false;
    }

    if (*version && (*load || *store || keep || *poolsize != -1
        || *filename != dflt_filename)) {
        fprintf(stderr, "--version cannot be combined with other options\n");
        return true;
    }
    if (!*load && !*store && !*version) {
        fprintf(stderr, "Need one of the commands <load | store>\n");
        return true;
    }
    if (!*version && keep) {
        if (*store) {
            fprintf(stderr, "The --keep flag can only be used with the load"
                " command\n");
            return true;
        }
        *store = true;
    }

    return false;
}


/** \brief Print a usage message
 */
void print_usage(void)
{
    printf("Usage:\n"
        "    %s [OPTIONS] <COMMAND>\n"
        "    %s <-h | --help>\n"
        "    %s <-v | --version>\n"
        "\n"
        "Here, COMMAND is either \"load\" or \"store\" and then OPTIONS can"
        " be any of:\n"
        "  -b<num>  | --bits <num>  : number of bits to load/store\n"
        "                             defaults to full pool size\n"
        "  -f<name> | --file <name> : name of the file to load from or store"
        " to\n"
        "                             (default: %s)\n"
        "  -k       | --keep        : keep the old file after loading from"
        " it\n",
        PACKAGE_NAME,
        PACKAGE_NAME,
        PACKAGE_NAME,
        dflt_filename);
}


/** \brief Print the version information
 */
void print_version(void)
{
    printf("%s\n"
        "Copyright (C) 2019  MenloSystems GmbH\n"
        "This program comes with ABSOLUTELY NO WARRANTY.\n"
        "This is free software, and you are welcome to redistribute it\n"
        "under certain conditions.\n",
        PACKAGE_STRING);
}


/** \brief Main function of rngminder program
 *  \param[in] argc Number of entries in `argv`
 *  \param[in] argv Array of command line arguments
 *  \return Program return value
 *
 *  Stores the entropy-pool on system shutdown and restores it on next boot.
 */
int main(int argc, char *argv[])
{
    bool help;
    bool version;
    bool load;
    bool store;
    int poolsize;
    const char *filename;

    if (parse_opts(argc, argv, &help, &version, &load, &store, &poolsize,
        &filename)) {
        return EXIT_FAILURE;
    }

    if (help) {
        print_usage();
        return EXIT_SUCCESS;
    }
    if (version) {
        print_version();
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
