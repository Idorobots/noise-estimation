#ifndef __CONFIG_H__
#define __CONFIG_H__

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"


typedef struct _Config {
    long ex_filter_type;
    long ex_window_size;
    long ex_iterations;

    double lpf_f;
    double lpf_f_SNR;
    double lpf_f_Rice;

    char csv_delimiter;

    char *input_filename;
    char *input_filename_SNR;
    char *output_filename_Gaussian;
    char *output_filename_Rician;

    char *title_input;
    char *title_SNR;
    char *title_Gaussian;
    char *title_Rician;
} Config;

int read_config(const char *filename, Config *config);
void print_config(const Config *config);

#endif
