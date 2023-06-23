#include <stdlib.h>
#include "connectionhandler.h"
#include "util.h"
#include <getopt.h>
#include "broadcastagent.h"
#include <signal.h>

static void print_help();

static void sigint_handler();
static void sigsegv_handler();

int main(int argc, char **argv) {
    uint16_t port = 8111;
    utilInit(argv[0]);
    char *strPort;
    infoPrint("Chat server, group 02");    //TODO: Add your group number!

    //TODO: evaluate command line arguments
    debugDisable();
    styleEnable();
    const char *short_options = "dm:p:h";
    int option_index = 0;
    while ((option_index = getopt(argc, argv, short_options)) != -1) {
        switch (option_index) {
            case 'd':
                if (debugEnabled() == 0) {
                    debugEnable();
                }
                break;
            case 'm':
                if (styleEnabled() == 1) {
                    styleDisable();
                }
                break;
            case 'p':
                strPort = strdup(optarg);
                uint16_t strLength = strlen(strPort);
                for (int i = 0; i < strLength; i++) {
                    if (strPort[i] < 0x30 || strPort[i] > 0x39) {
                        errorPrint("Port number must be an integer between 1024 and 65535!");
                        exit(EXIT_FAILURE);
                    }
                }
                port = (uint16_t) atoi(strPort);
                if (port < 1023) {
                    errorPrint("Port number must be an integer between 1024 and 65535!");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
            default:
                errorPrint("Option %c incorrect", option_index);
                exit(EXIT_FAILURE);
        }
    }
    //TODO: perform initialization
    if (broadcastAgentInit() == -1) {
        return EXIT_FAILURE;
    }
    sigset_t mask_sigint;
    sigset_t mask_sigsegv;
    sigemptyset(&mask_sigint);
    sigemptyset(&mask_sigsegv);

    const struct sigaction action_sigint = {
            .sa_handler = sigint_handler,
            .sa_mask = mask_sigint,
            .sa_flags = 0
    };

    const struct sigaction action_sigsegv = {
            .sa_handler = sigsegv_handler,
            .sa_mask = mask_sigsegv,
            .sa_flags = 0
    };

    if (sigaction(SIGINT, &action_sigint, NULL) != 0) {
        errorPrint("Cannot register signal handler!");
        exit(EXIT_FAILURE);
    }
    if(sigaction(SIGSEGV, &action_sigsegv, NULL) != 0) {
        errorPrint("Cannot register signal handler!");
        exit(EXIT_FAILURE);
    }

    //TODO: use port specified via command line
    const int result = connectionHandler((in_port_t) port);
    //TODO: perform cleanup, if required by your implementation

    return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void print_help() {
    infoPrint("Usage:  ./server [-d] [-m] [-p PORT]");
    infoPrint("  -d             enable additional debug output");
    infoPrint("  -m             do  not use colors for output (monochrome)");
    infoPrint("  -p PORT        set TCP port to use (default: 8111)");
}

static void sigint_handler() {
    infoPrint("Caught Signal 2, shutting down!");
    broadcastAgentCleanup();
    exit(EXIT_SUCCESS);
}

static void sigsegv_handler() {
    errorPrint("Caught Segmentation Fault, shutting down");
    broadcastAgentCleanup();
    exit(EXIT_FAILURE);
}
