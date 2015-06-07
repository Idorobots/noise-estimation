#include "config.h"

#define BUF_SIZE 256

char *preprocess(char *string, size_t length) {
    for(size_t i = 0; i < length; ++i) {
        if(string[i] == '#' || string[i] == '\n') { // Strip comments & newlines.
            string[i] = '\0';
        }
    }

    return trim(string);
}

char *replace(char *string, char from, char to) {
    size_t length = strlen(string);

    for(size_t i = 0; i < length; ++i) {
        if(string[i] == from) {
            string[i] = to;
        }
    }

    return string;
}

char *dup(char *string) {
    return strdup(string);
}

void build_config(Config *config, char *key, char *value) {
    if(strcmp(key, "ex_filter_type") == 0) {
        config->ex_filter_type = atoi(value);
    } else if(strcmp(key, "ex_window_size") == 0) {
        config->ex_window_size = atoi(value);
    } else if(strcmp(key, "ex_iterations") == 0) {
        config->ex_iterations = atoi(value);
    } else if(strcmp(key, "lpf_f") == 0) {
        config->lpf_f = strtod(value, NULL);
    } else if(strcmp(key, "lpf_f_SNR") == 0) {
        config->lpf_f_SNR = strtod(value, NULL);
    } else if(strcmp(key, "lpf_f_Rice") == 0) {
        config->lpf_f_Rice = strtod(value, NULL);
    } else if(strcmp(key, "input_filename") == 0) {
        config->input_filename = dup(trim(replace(value, '\'', ' ')));
    } else if(strcmp(key, "output_filename_Gaussian") == 0) {
        config->output_filename_Gaussian = dup(trim(replace(value, '\'', ' ')));
    } else if(strcmp(key, "output_filename_Rician") == 0) {
        config->output_filename_Rician = dup(trim(replace(value, '\'', ' ')));
    } else if(strcmp(key, "csv_delimiter") == 0) {
        config->csv_delimiter = trim(replace(value, '\'', ' '))[0];
    } else {
#ifdef DEBUG
        printf("Unknown key: '%s', value: '%s'\n", key, value);
#endif
    }
}

void add_defaults(Config *config) {
    config->csv_delimiter = ',';
}

int read_config(Config *config, char *filename) {
    if(config == NULL) {
        return -1;
    }

    FILE *file = fopen(filename, "r");

    if(!file) {
        return -1;
    }

    char *line = NULL;
    size_t num_bytes = 0;
    ssize_t read = 0;

    add_defaults(config);

    while((read = getline(&line, &num_bytes, file)) != -1) {
        line = preprocess(line, num_bytes);

        if(strlen(line) != 0) {
            char *value = NULL;
            value = trim(split(line, '='));
            line = trim(line);

#ifdef DEBUG
            printf("key: %s, value: %s\n", line, value);
#endif
            build_config(config, line, value);
        }
    }

    free(line);
    fclose(file);

    return 0;
}

void print_config(Config *config) {
    printf("ex_filter_type = %ld\n", config->ex_filter_type);
    printf("ex_window_size = %ld\n", config->ex_window_size);
    printf("ex_iterations = %ld\n", config->ex_iterations);
    printf("lpf_f = %lf\n", config->lpf_f);
    printf("lpf_f_SNR = %lf\n", config->lpf_f_SNR);
    printf("lpf_f_Rice = %lf\n", config->lpf_f_Rice);
    printf("csv_delimiter = '%c'\n", config->csv_delimiter);
    printf("input_filename = '%s'\n", config->input_filename);
    printf("output_filename_Gaussian = '%s'\n", config->output_filename_Gaussian);
    printf("outut_filename_Rician = '%s'\n", config->output_filename_Rician);
}
