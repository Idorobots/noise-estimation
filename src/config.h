#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdlib.h>

typedef struct _Conf {
    long ex_filter_type;
    long ex_window_size;
    long ex_iterations;

    double lpf_f;
    double lpf_f_SNR;
    double lpf_f_Rice;

    char *input_filename;
    char *output_filename_Gaussian;
    char *output_filename_Rician;
} Conf;

Conf *read_config(char *filename);

#endif
