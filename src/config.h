#ifndef __CONFIG_H__
#define __CONFIG_H__

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


typedef struct _Config {
    long ex_filter_type;
    long ex_window_size;
    long ex_iterations;

    double lpf_f;
    double lpf_f_SNR;
    double lpf_f_Rice;

    char csv_delimiter;
    char *input_filename;
    char *output_filename_Gaussian;
    char *output_filename_Rician;
} Config;

int read_config(Config *config, char *filename);
void print_config(Config *config);

#endif
