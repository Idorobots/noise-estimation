#include "image.h"

#define BUF_SIZE = (1024*64)

char *get_extension(char *filename) {
    char *dot = strrchr(filename, '.');

    if(!dot || dot == filename) {
        return "";
    }

    return dot + 1;
}

size_t count(char *string, size_t length, char character) {
    size_t count = 0;

    for(size_t i = 0; i < length; ++i) {
        if(string[i] == character) {
            ++count;
        }
    }

    return count;
}

int get_size(char *filename, char delimiter, size_t *width, size_t *height) {
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

Image *normalize(Image *data) {
    double max = 0;
    cvMinMaxLoc(data, NULL, &max, NULL, NULL, NULL);

#ifdef DEBUG
    printf("max: %f\n", max);
#endif

    Image *normalized = cvCloneMat(data);
    cvScale(data, normalized, 1.0/max, 0);

    return normalized;
}

Image *read_data(char *filename, char delimiter, size_t width, size_t height) {
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
    return data;
}

Image *read_image(char *filename, Config *config) {
    Image *image = NULL;

    char *extension = get_extension(filename);

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

        if(get_size(filename, config->csv_delimiter, &width, &height) != -1) {
#ifdef DEBUG
            printf("width: %ld\nheight: %ld\n", width, height);
#endif

            image = read_data(filename, config->csv_delimiter, width, height);
        }
    } else {
        printf("Unrecognized image file type: %s\n", extension);
    }

    return image;
}

void show_image(char *title, int x, int y, Image *image) {
    Image *normalized = normalize(image);
    cvNamedWindow(title, CV_WINDOW_AUTOSIZE);
    cvMoveWindow(title, x, y);
    cvShowImage(title, normalized);
    cvReleaseMat(&normalized);
}

int write_image(char *filename, Image *image, Config *config) {
    char *extension = get_extension(filename);

#ifdef DEBUG
    printf("Extension: '%s'\n", extension);
#endif

    if(strcmp(extension, "png") == 0) {
        // Easy case:
        return cvSaveImage(filename, image, NULL);
    } else if (strcmp(extension, "csv") == 0) {
        // TODO Save CSV
        return 0;
    } else {
        printf("Unrecognized image file type: %s\n", extension);
    }

    return -1;
}
