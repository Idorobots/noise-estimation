#include "image.h"

#define BUF_SIZE = (1024*64)

const char *get_extension(const char *filename) {
    char *dot = strrchr(filename, '.');

    if(!dot || dot == filename) {
        return "";
    }

    return dot + 1;
}

size_t count(const char *string, size_t length, char character) {
    size_t count = 0;

    for(size_t i = 0; i < length; ++i) {
        if(string[i] == character) {
            ++count;
        }
    }

    return count;
}

int get_csv_size(const char *filename, char delimiter, size_t *width, size_t *height) {
    FILE *file = fopen(filename, "r");

    if(!file) {
        return -1;
    }

    char *line = NULL;
    size_t num_bytes = 0;
    ssize_t read = 0;

    read = getline(&line, &num_bytes, file);

    if(read == -1) {
        return -1;
    }

    // Get image width.
    *width = 1 + count(line, num_bytes, delimiter);

    // Get image height.
    for(size_t i = 1; ; i++) {
        if((read = getline(&line, &num_bytes, file)) == -1) {
            *height = i;
            return 0;
        }
    }

    free(line);
    fclose(file);
    return 0;
}

Image *normalize(const Image *data) {
    double max = 0;
    cvMinMaxLoc(data, NULL, &max, NULL, NULL, NULL);

#ifdef DEBUG
    printf("max: %f\n", max);
#endif

    Image *normalized = cvCloneMat(data);
    cvScale(data, normalized, 1.0/max, 0);

    return normalized;
}

Image *read_csv_data(const char *filename, char delimiter, size_t width, size_t height) {
    Image *data = cvCreateMat(height, width, IMAGE_DEPTH);

    FILE *file = fopen(filename, "r");

    if(!file) {
        return NULL;
    }

    char *line = NULL;
    size_t num_bytes = 0;
    ssize_t read = 0;

    for(size_t i = 0; i < height; ++i) {
        if((read = getline(&line, &num_bytes, file)) != -1) {
            char *rest = line;

            for(size_t j = 0; j < width; ++j) {
                cvmSet(data, i, j, strtof(rest, NULL));
                rest = trim(split(rest, delimiter));
            }
        }
    }

    free(line);
    fclose(file);
    return data;
}

int write_csv_data(const char *filename, char delimiter, const Image *image) {
    if(image == NULL) {
        return -1;
    }

    int size[2];
    cvGetDims(image, size);
    size_t width = size[1];
    size_t height = size[0];

    FILE *file = fopen(filename, "w");

    if(!file) {
        return -1;
    }

    for(size_t i = 0; i < height; ++i) {
        for(size_t j = 0; j < width; ++j) {
            fprintf(file, "%f", cvmGet(image, i, j));

            if(j < width-1) {
                fprintf(file, "%c ", delimiter);
            }
        }
        fprintf(file, "\n");
    }

    fclose(file);
    return 0;
}

Image *read_image(const char *filename, const Config *config) {
    Image *image = NULL;

    const char *extension = get_extension(filename);

#ifdef DEBUG
    printf("Extension: '%s'\n", extension);
#endif

    if(strcmp(extension, "png") == 0) {
        // Easy case:
        Image *data = cvLoadImageM(filename, CV_LOAD_IMAGE_GRAYSCALE);

        if(data != NULL) {
            int size[2];
            cvGetDims(data, size);
            image = cvCreateMat(size[0], size[1], IMAGE_DEPTH);
            cvConvert(data, image);
        }
    } else if(strcmp(extension, "csv") == 0) {
        // We need to parse the CSV manually:
        size_t width = 0, height = 0;

        if(get_csv_size(filename, config->csv_delimiter, &width, &height) != -1) {
#ifdef DEBUG
            printf("width: %ld\nheight: %ld\n", width, height);
#endif

            image = read_csv_data(filename, config->csv_delimiter, width, height);
        }
    } else {
        printf("Unrecognized image file type: %s\n", extension);
    }

    return image;
}

void show_image(const char *title, int x, int y, const Image *image) {
    Image *normalized = normalize(image);
    cvNamedWindow(title, CV_WINDOW_AUTOSIZE);
    cvMoveWindow(title, x, y);
    cvShowImage(title, normalized);
    cvReleaseMat(&normalized);
}

int write_image(const char *filename, const Image *image, const Config *config) {
    const char *extension = get_extension(filename);

#ifdef DEBUG
    printf("Extension: '%s'\n", extension);
#endif

    if(strcmp(extension, "png") == 0) {
        // Easy case:
        return cvSaveImage(filename, image, NULL);
    } else if (strcmp(extension, "csv") == 0) {
        return write_csv_data(filename, config->csv_delimiter, image);
    } else {
        printf("Unrecognized image file type: %s\n", extension);
    }

    return -1;
}
