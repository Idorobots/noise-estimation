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
    FILE *file = NULL;

    file = fopen(filename, "r");

    if (!file) {
        return -1;
    }

    char *line = NULL;
    size_t num_bytes = 0;
    ssize_t read;

    read = getline(&line, &num_bytes, file);
    if (read == -1) {
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

CvMat *read_data(char *filename, size_t width, size_t height) {
    CvMat *data = cvCreateMat(height, width, CV_32F);
    // TODO Actually read the data.
    return data;
}

IplImage *read_image(char *filename) {
    IplImage *image = NULL;

    char *extension = get_extension(filename);

#ifdef DEBUG
    printf("Extension: '%s'\n", extension);
#endif

    if(strcmp(extension, "png") == 0) {
        image = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
    } else if (strcmp(extension, "csv") == 0) {
        size_t width, height;

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

void show_image(char *title, IplImage *image) {
    cvNamedWindow(title, CV_WINDOW_AUTOSIZE);
    cvMoveWindow(title, 100, 100);
    cvShowImage(title, image);
}
