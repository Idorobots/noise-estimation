#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "image.h"
#include "homomorf.h"

typedef struct _Options {
    char *conf_file;
    int no_gui;
} Options;


int run(const Config *config, const Options *options) {
    Image *input = read_image(config->input_filename, config);

    if(!input) {
        printf("ERROR: Couldn't load input file %s.\n", config->input_filename);
        return EXIT_FAILURE;
    }

    if(!options->no_gui) {
        show_image("Input image", 100, 100, input);
    }

    Image *rician = NULL, *gaussian = NULL;

    if(homomorf_est(input, &rician, &gaussian, config) == -1) {
        printf("ERROR: Error while processing input file %s.\n", config->input_filename);
        return EXIT_FAILURE;
    }

    if(!options->no_gui) {
        show_image("Rician noise map", 400, 100, rician);
        show_image("Gaussian noise map", 700, 100, gaussian);
        cvWaitKey(0);
    }

    printf("Saving file %s.\n", config->output_filename_Rician);
    if(write_image(config->output_filename_Rician, rician, config) == -1) {
        printf("ERROR: Couldn't save an image file %s.\n", config->output_filename_Rician);
        return EXIT_FAILURE;
    }

    printf("Saving file %s.\n", config->output_filename_Gaussian);
    if(write_image(config->output_filename_Gaussian, gaussian, config) == -1) {
        printf("ERROR: Couldn't save an image file %s.\n", config->output_filename_Gaussian);
        return EXIT_FAILURE;
    }

    // NOTE No need to cleanup images & windows since we're exitting anyway.
    return EXIT_SUCCESS;
}

void print_usage(const char *name) {
    printf("USAGE: %s [OPTIONS] CONFIG_FILE\n", name);
    printf("OPTIONS:\n");
    printf("\t--no-gui\tDisables GUI images\n");
}

int main(int argc, char **argv) {
    Options options = {
        .no_gui = 0,
        .conf_file = NULL
    };

    if(argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    } else {
        for(size_t i = 1; i < (size_t) argc; ++i) {
#ifdef DEBUG
            printf("option: %s\n", argv[i]);
#endif
            if(argv[i][0] == '-') {
                if(strcmp(argv[i], "--no-gui") == 0) {
                    options.no_gui = 1;
                } else {
                    printf("ERROR: Unrecognized option: %s\n", argv[i]);
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
            } else if(i == (size_t) argc-1) {
                options.conf_file = argv[i];
            } else {
                printf("ERROR: Too many input arguments, starting at: %s\n", argv[i+1]);
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
        }
    }

    if(options.conf_file == NULL) {
        printf("ERROR: No input file specified\n");
        return EXIT_FAILURE;
    }

    printf("Using config file %s.\n", options.conf_file);

    Config config;

    if(read_config(options.conf_file, &config) == -1) {
        printf("ERROR: Couldn't load config file %s.\n", options.conf_file);
        return EXIT_FAILURE;
    }

#ifdef DEBUG
    print_config(&config);
#endif

    return run(&config, &options);
}
