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

int get_size(char *filename, size_t *width, size_t *height) {
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
    *width = 1 + count(line, num_bytes, ','); // FIXME This shouldn't be hardcoded.

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

CvMat *normalize(CvMat *data) {
    double max = 0;
    cvMinMaxLoc(data, NULL, &max, NULL, NULL, NULL);

#ifdef DEBUG
    printf("max: %f\n", max);
#endif

    CvMat *normalized = cvCloneMat(data);
    cvScale(data, normalized, 1.0/max, 0);

    return normalized;
}

CvMat *read_data(char *filename, size_t width, size_t height) {
    CvMat *data = cvCreateMat(height, width, CV_32F);

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
                cvmSet(data, i, j, strtof(rest, &rest));
                rest++; // Skip the delimiter. // FIXME Actually do this properly.
            }
        }
    }

    CvMat *normalized = normalize(data);
    cvReleaseData(data);
    return normalized;
}

Image *read_image(char *filename) {
    IplImage *image = NULL;

    char *extension = get_extension(filename);

#ifdef DEBUG
    printf("Extension: '%s'\n", extension);
#endif

    if(strcmp(extension, "png") == 0) {
        // Easy case:
        image = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
    } else if(strcmp(extension, "csv") == 0) {
        // We need to parse the CSV manually:
        size_t width = 0, height = 0;

        if(get_size(filename, &width, &height) != -1) {
#ifdef DEBUG
            printf("width: %ld\nheight: %ld\n", width, height);
#endif

            CvMat *data = NULL;
            if((data = read_data(filename, width, height)) != NULL) {
                image = cvGetImage(data, cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1));
            }
        }
    } else {
        printf("Unrecognized image file type: %s\n", extension);
    }

    return image;
}

void show_image(char *title, int x, int y, Image *image) {
    cvNamedWindow(title, CV_WINDOW_AUTOSIZE);
    cvMoveWindow(title, x, y);
    cvShowImage(title, image);
}
