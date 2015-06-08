#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "image.h"
#include "homomorf.h"


int run(Config *config) {
    Image *input = read_image(config);

    if(!input) {
        printf("Couldn't load input file %s.\n", config->input_filename);
        return EXIT_FAILURE;
    }

#ifdef DEBUG
    show_image("Input image", 100, 100, input);
#endif

    Image *rician = NULL, *gaussian = NULL;

    if(homomorf_est(input, &rician, &gaussian, config) == -1) {
        printf("Error while processing input file %s.\n", config->input_filename);
        return EXIT_FAILURE;
    }

#ifdef DEBUG
    show_image("Rician noise map", 400, 100, rician);
    show_image("Gaussian noise map", 700, 100, gaussian);
    cvWaitKey(0);
#endif

    // TODO Save CSV files.

    // NOTE No need to cleanup images & windows since we're exitting anyway.
    return EXIT_SUCCESS;
}

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

    return run(config);
}
