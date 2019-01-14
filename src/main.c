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
#include <fcntl.h>
#include <getopt.h>
#include <linux/random.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>


/** \brief Default name of the file to load from / store to
 */
const char dflt_filename[] = LOCALSTATEDIR "/lib/urandom/random-seed";


/** \brief Upper limit of poolsize in bits
 *
 *  The assumption is that anything larger than 64kiB is probably a bug or an
 *  invalid argument from the user. For comparison: currently the default is
 *  4kiB.
 */
const int max_sane_poolsize = 1<<16;


/** \brief Parse command line options
 *  \param[in]  argc Number of entries in `argv`
 *  \param[in]  argv Array of command line arguments
 *  \param[out] help     Show help-screen
 *  \param[out] version  Show version information
 *  \param[out] load     Load pool from a file
 *  \param[out] store    Store pool to a file
 *  \param[out] filename Name of the file to load from / store to
 *  \param[out] poolsize Number of bits to store or -1 for auto detection
 *  \return `true` on error, `false` on success
 *
 *  If an error is returned, an error message was already printed to stderr.
 */
bool parse_opts(int argc, char* argv[], bool *help, bool *version, bool *load,
    bool *store, const char **filename, int *poolsize)
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
    *filename = dflt_filename;
    *poolsize = -1;

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

    if (*version && (*load || *store || keep || *filename != dflt_filename
        || *poolsize != -1)) {
        fprintf(stderr, "--version cannot be combined with other options\n");
        return true;
    }
    if (!*load && !*store && !*version) {
        fprintf(stderr, "Need one of the commands <load | store>\n");
        return true;
    }
    if (*store && keep) {
        fprintf(stderr, "The --keep flag can only be used with the load"
            " command\n");
        return true;
    }
    if (*load && !keep) {
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


/** \brief Read size of entropy pool
 *  \param[out] poolsize Number of bits in kernel pool
 *  \return `true` on error, `false` on success
 *
 *  If an error is returned, an error message was already printed to stderr.
 */
bool get_poolsize(int *poolsize)
{
    const char filename[] = "/proc/sys/kernel/random/poolsize";
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("Opening file to read poolsize failed");
        return true;
    }

    if (fscanf(f, "%d", poolsize) != 1) {
        perror("Failed to read poolsize");
        fclose(f);
        return true;
    }

    if (fclose(f)) {
        perror("Closing file after reading poolsize failed");
        return true;
    }

    return false;
}


/** \brief Load from a file to the entropy pool
 *  \param[in] filename Name of the file to load from
 *  \return `true` on error, `false` on success
 *
 *  If an error is returned, an error message was already printed to stderr.
 */
bool do_load(const char filename[])
{
    int src = open(filename, O_RDONLY);
    if (src == -1) {
        perror("Opening file for reading failed");
        return true;
    }
    const char dev[] = "/dev/urandom";
    int dst = open(dev, O_WRONLY);
    if (dst == -1) {
        perror("Opening RNG device for writing failed");
        close(src);
        return true;
    }

    const size_t bufsize = 1024;
    struct { // compare rand_pool_info
        int entropy_count;
        int buf_size;
        char buf[bufsize];
    } info;
    for (;;) {
        info.buf_size = read(src, info.buf, bufsize);
        if (info.buf_size == -1) {
            perror("Reading from file failed");
            close(dst);
            close(src);
            return true;
        }
        if (info.buf_size > 0) {
            info.entropy_count = 8 * info.buf_size;
            const int err = ioctl(dst, RNDADDENTROPY, &info);
            if (err == -1) {
                perror("Writing to RNG device failed");
                close(dst);
                close(src);
                return true;
            }
        }
        if (info.buf_size < bufsize) {
            break;
        }
    }

    if (close(dst) == -1) {
        perror("Closing RNG device after writing failed");
        close(src);
        return true;
    }
    if (close(src) == -1) {
        perror("Closing file after reading failed");
        return true;
    }

    return false;
}


/** \brief Store from the entropy pool to a file
 *  \param[in] filename Name of the file to store to
 *  \param[in] poolsize Number of bits to store
 *  \return `true` on error, `false` on success
 *
 *  If an error is returned, an error message was already printed to stderr.
 */
bool do_store(const char filename[], int poolsize)
{
    FILE *dst = fopen(filename, "w");
    if (dst == NULL) {
        perror("Opening file for writing failed");
        return true;
    }
    const char dev[] = "/dev/urandom";
    FILE *src = fopen(dev, "r");
    if (src == NULL) {
        perror("Opening RNG device for reading failed");
        fclose(dst);
        return true;
    }

    const size_t bufsize = 1024;
    char buf[bufsize];
    for(int left=(poolsize+7)/8; left>0; ) {
        const int needed = left>bufsize ? bufsize : left;
        if (fread(buf, 1, needed, src) != needed) {
            perror("Reading from RNG device failed");
            fclose(src);
            fclose(dst);
            return true;
        }
        if (fwrite(buf, 1, needed, dst) != needed) {
            perror("Writing to file failed");
            fclose(src);
            fclose(dst);
            return true;
        }
        left -= needed;
    }

    if (fclose(src)) {
        perror("Closing RNG device after reading failed");
        fclose(dst);
        return true;
    }
    if (fclose(dst)) {
        perror("Closing file after writing failed");
        return true;
    }

    return false;
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
    const char *filename;
    int poolsize;

    if (parse_opts(argc, argv, &help, &version, &load, &store, &filename,
        &poolsize)) {
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

    if (store && poolsize == -1) {
        if (get_poolsize(&poolsize)) {
            return EXIT_FAILURE;
        }
    }
    if (poolsize > max_sane_poolsize) {
        fprintf(stderr, "poolsize seems a bit large (%d > %d): refusing to do"
            " this\n", poolsize, max_sane_poolsize);
            return EXIT_FAILURE;
    }

    if (load) {
        if(do_load(filename)) {
            return EXIT_FAILURE;
        }
    }
    if (store) {
        if(do_store(filename, poolsize)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
