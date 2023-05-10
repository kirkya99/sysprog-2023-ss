#include <stdlib.h>
#include "connectionhandler.h"
#include "util.h"
#include <getopt.h>
#include <stdio.h>

static void print_help();
static uint16_t input(int argc, char **argv);

int main(int argc, char **argv)
{
    uint16_t port = 8111;
    utilInit(argv[0]);
    infoPrint("Chat server, group 02");	//TODO: Add your group number!

    //TODO: evaluate command line arguments
    debugDisable();
    styleDisable();
    const char *short_options = "a:dmn:p:rs";
    char *adminName = "admin";
    char *serverName = "reference_server";
    int option_index = 0;
    while((option_index = getopt(argc, argv, short_options)) != -1)
    {
        switch (option_index) {
            case 'a':
                strcpy(adminName, optarg);
                break;
            case 'd':
                if(debugEnabled() == 0)
                {
                    debugEnable();
                }
                break;
            case 'm':
                if(styleEnabled() == 1)
                {
                    styleDisable();
                }
                break;
            case 'n':
                break;
            case 'p':
                port = (uint16_t) atoi(optarg);
                break;
            case 'r':
                break;
            case 's':
                break;
            case 'h':
                print_help();
                break;
            default:
                infoPrint("Option %c incorrect", option_index);
                break;
        }
    }
    //TODO: perform initialization

    //TODO: use port specified via command line
    const int result = connectionHandler((in_port_t)port);

    //TODO: perform cleanup, if required by your implementation
    return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void print_help()
{
    infoPrint("Usage:  ./server [-a ADMIN] [-d] [-m] [-n SERVERNAME] [-p PORT] [-r] [-s]\n");
    infoPrint("  -a ADMIN       set name of the administrator (default: admin)\n");
    infoPrint("  -d             enable additional debug output\n");
    infoPrint("  -m             do  not use colors for output (monochrome)\n");
    infoPrint("  -n SERVERNAME  set server name (default: reference-server)\n");
    infoPrint("  -p PORT        set TCP port to use (default: 8111)\n");
    infoPrint("  -r             hexdump received packets\n");
    infoPrint("  -s             hexdump sent packets");
}
