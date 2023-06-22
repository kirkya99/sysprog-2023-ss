#include <stdlib.h>
#include "connectionhandler.h"
#include "util.h"
#include <getopt.h>
#include "broadcastagent.h"
#include <signal.h>
#include <unistd.h>

static void print_help();

static void quit();

int main(int argc, char **argv) {
    uint16_t port = 8111;
    utilInit(argv[0]);
    char *strPort;
    infoPrint("Chat server, group 02");    //TODO: Add your group number!

    //TODO: evaluate command line arguments
    debugDisable();
    styleEnable();
    const char *short_options = "dm:p:h";
    char *adminName = "admin";
    char *serverName = "reference_server";
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
                        exit(EXIT_SUCCESS);
                    }
                }
                port = (uint16_t) atoi(strPort);
                if (port < 1024) {
                    errorPrint("Port number must be an integer between 1024 and 65535!");
                    exit(EXIT_SUCCESS);
                }
                break;
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
            default:
                errorPrint("Option %c incorrect", option_index);
                exit(EXIT_SUCCESS);
        }
    }
    //TODO: perform initialization
    debugEnable();

    if (broadcastAgentInit() == -1) {
        return EXIT_FAILURE;
    }

    //TODO: use port specified via command line
    signal(SIGINT, quit);
    const int result = connectionHandler((in_port_t) port);
    //TODO: perform cleanup, if required by your implementation

    broadcastAgentCleanup();
    return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void print_help() {
    styleDisable();
    infoPrint("Usage:  ./server [-d] [-m] [-p PORT]");
    infoPrint("  -d             enable additional debug output");
    infoPrint("  -m             do  not use colors for output (monochrome)");
    infoPrint("  -p PORT        set TCP port to use (default: 8111)");
    styleEnable();
}

static void quit() {
    infoPrint("Caught Signal 2, shutting down!");
    broadcastAgentCleanup();
    exit(EXIT_SUCCESS);
}
