#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "image.h"
#include "homomorf.h"


int run(const Config *config) {
    Image *input = read_image(config->input_filename, config);

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

    if(write_image(config->output_filename_Rician, rician, config) == -1) {
        printf("Couldn't save an image file %s.\n", config->output_filename_Rician);
        return EXIT_FAILURE;
    }

    if(write_image(config->output_filename_Gaussian, gaussian, config) == -1) {
        printf("Couldn't save an image file %s.\n", config->output_filename_Gaussian);
        return EXIT_FAILURE;
    }

    // NOTE No need to cleanup images & windows since we're exitting anyway.
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    char *conf_file = NULL;

    if(argc == 2) {
        conf_file = argv[1];
    } else {
        printf("USAGE: %s CONFIG_FILE\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Using config file %s:\n", conf_file);

    Config config;

    if(read_config(conf_file, &config) < 0) {
        printf("Couldn't load config file %s.\n", conf_file);
        return EXIT_FAILURE;
    }
    print_config(&config);

    return run(&config);
}
