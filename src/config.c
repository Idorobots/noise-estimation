#include "config.h"

char *trim(char *str) {
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if(str == NULL) {
        return NULL;
    }
    if(str[0] == '\0') {
        return str;
    }

    len = strlen(str);
    endp = str + len;

    while(isspace(*frontp)) {
        ++frontp;
    }
    if(endp != frontp) {
        while(isspace(*(--endp)) && endp != frontp) {}
    }

    if(str + len - 1 != endp) {
        *(endp + 1) = '\0';

    }
    else if(frontp != str &&  endp == frontp) {
        *str = '\0';
    }

    endp = str;
    if(frontp != str) {
        while(*frontp) {
            *endp++ = *frontp++;
        }
        *endp = '\0';
    }


    return str;
}

char *preprocess(char *string, size_t length) {
    for(size_t i = 0; i < length; ++i) {
        if(string[i] == '#' || string[i] == '\n') { // Strip comments & newlines.
            string[i] = '\0';
        }
    }

    return trim(string);
}

char *split(char *string, char delimiter) {
    size_t length = strlen(string);

    for(size_t i = 0; i < length; ++i) {
        if(string[i] == delimiter) {
            string[i] = '\0';
            return &string[i+1];
        }
    }

    return NULL;
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

Config *build_config(Config *config, char *key, char *value) {
    if(strcmp(key, "ex_filter_type") == 0) {
        config->ex_filter_type = atoi(value);
    } else if (strcmp(key, "ex_window_size") == 0) {
        config->ex_window_size = atoi(value);
    } else if (strcmp(key, "ex_iterations") == 0) {
        config->ex_iterations = atoi(value);
    } else if (strcmp(key, "lpf_f") == 0) {
        config->lpf_f = strtod(value, NULL);
    } else if (strcmp(key, "lpf_f_SNR") == 0) {
        config->lpf_f_SNR = strtod(value, NULL);
    } else if (strcmp(key, "lpf_f_Rice") == 0) {
        config->lpf_f_Rice = strtod(value, NULL);
    } else if (strcmp(key, "input_filename") == 0) {
        config->input_filename = dup(trim(replace(value, '\'', ' ')));
    } else if (strcmp(key, "output_filename_Gaussian") == 0) {
        config->output_filename_Gaussian = dup(trim(replace(value, '\'', ' ')));
    } else if (strcmp(key, "output_filename_Rician") == 0) {
        config->output_filename_Rician = dup(trim(replace(value, '\'', ' ')));
    } else {
#ifdef DEBUG
        printf("Unknown key: '%s', value: '%s'\n", key, value);
#endif
    }

    return config;
}

int read_config(Config *config, char *filename) {
    if(config == NULL) {
        return -1;
    }

    FILE *file = NULL;

    file = fopen(filename, "r");

    if (!file) {
        return -1;
    }

    char *line = NULL;
    size_t num_bytes = 0;
    ssize_t read;

    while((read = getline(&line, &num_bytes, file)) != -1) {
        line = preprocess(line, num_bytes);

        if(strlen(line) != 0) {
            char *value = NULL;
            value = trim(split(line, '='));
            line = trim(line);

#ifdef DEBUG
            printf("key: %s, value: '%s'\n", line, value);
#endif
            config = build_config(config, line, value);
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
    printf("input_filename = '%s'\n", config->input_filename);
    printf("output_filename_Gaussian = '%s'\n", config->output_filename_Gaussian);
    printf("outut_filename_Rician = '%s'\n", config->output_filename_Rician);
}
