#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "image.h"

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

    Image *image = read_image(config);
    if(!image) {
        printf("Couldn't load input file %s.\n", config->input_filename);
        return EXIT_FAILURE;
    }

    show_image("Noise Estimation", 100, 100, image);

    // TODO Implement the actual algorithm.

    cvWaitKey(0);

    // NOTE No need to cleanup images & windows since we're exitting anyway.
    return EXIT_SUCCESS;
}
