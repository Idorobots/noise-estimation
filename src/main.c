#include <stdio.h>

#include "config.h"

int main(int argc, char **argv) {
    char *conf_file = "config.conf";

    if(argc > 1) {
        conf_file = argv[1];
    }

    printf("Using config file: %s\n", conf_file);

    Conf *config = read_config(conf_file);

    // TODO

    return 0;
}
