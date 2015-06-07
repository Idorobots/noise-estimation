#include <stdio.h>
#include <stdlib.h>

#include "config.h"

int main(int argc, char **argv) {
    char *conf_file = "config.conf";

    if(argc > 1) {
        conf_file = argv[1];
    }

    printf("Using config file %s:\n", conf_file);

    Config *config = malloc(sizeof(Config));

    if(read_config(config, conf_file) < 0) {
        printf("Couldn't load config file %s.\n", conf_file);
        return EXIT_FAILURE;
    }
    print_config(config);


    // TODO Implement the actual algorithm.

    free(config);
    return EXIT_SUCCESS;
}
