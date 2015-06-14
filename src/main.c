#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "image.h"
#include "homomorf.h"

typedef struct _Options {
    char *conf_file;
    int no_gui;
    int time;
} Options;


int run(const Config *config, const Options *options) {
    Image *input = read_image(config->input_filename, config);

    if(!input) {
        printf("ERROR: Couldn't load input file %s.\n", config->input_filename);
        return EXIT_FAILURE;
    }

    size_t offset = 0;
    size_t width = input->cols + 20; // NOTE 20 px margin

    if(!options->no_gui) {
        printf("GUI mode, press any key to exit.\n");

        show_image(config->title_input, 100 + width * offset, 100, input);
        ++offset;
    }

    Image *SNR = read_image(config->input_filename_SNR, config);

    if(!SNR) {
        printf("Not using an SNR map.\n");
    }

    Image *rician = NULL, *gaussian = NULL;

    double t_min = 1e30;
    double t_max = 0.0;
    double t_total = 0.0;

    clock_t diff, start;
    size_t num_times = options->time ? options->time : 1;
    int ret = -1;

    if(options->time) {
        printf("Running %ld tests:\n", num_times);
    }

    for(size_t i = 0; i < num_times; ++i) {
        start= clock();
        ret = homomorf_est(input, &SNR, &rician, &gaussian, config);
        diff = clock() - start;
        double ms = diff * 1000.0 / CLOCKS_PER_SEC;

        t_total += ms;
        t_max = ms > t_max ? ms : t_max;
        t_min = ms < t_min ? ms : t_min;
    }

    if(options->time) {
        printf("  Total time:   %lf ms\n", t_total);
        printf("  Average time: %lf ms\n", t_total / num_times);
        printf("  Maximal time: %lf ms\n", t_max);
        printf("  Minimal time: %lf ms\n", t_min);
    }

    if(ret == -1) {
        printf("ERROR: Error while processing input file %s.\n", config->input_filename);
        return EXIT_FAILURE;
    }

    if(!options->no_gui) {
        show_image(config->title_SNR, 100 + width * offset, 100, SNR);
        ++offset;
        show_image(config->title_Rician, 100 + width * offset, 100, rician);
        ++offset;
        show_image(config->title_Gaussian, 100 + width * offset, 100, gaussian);
        ++offset;
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
    printf("\t%-15s%-30s\n", "--help", "Display this message.");
    printf("\t%-15s%-30s\n", "--no-gui", "Disable GUI images.");
    printf("\t%-15s%-30s\n", "--time N", "Disable GUI images & time N executions.");
}

int main(int argc, char **argv) {
    Options options = {
        .no_gui = 0,
        .time = 0,
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
                } else if(strcmp(argv[i], "--time") == 0) {
                    if(i == (size_t) argc-1) {
                        print_usage(argv[0]);
                        return EXIT_FAILURE;
                    }
                    options.no_gui = 1;
                    options.time = atoi(argv[++i]);
                } else if(strcmp(argv[i], "--help") == 0) {
                    print_usage(argv[0]);
                    return EXIT_SUCCESS;
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
