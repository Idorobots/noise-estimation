#include "image.h"


char *get_extension(char *filename) {
    char *dot = strrchr(filename, '.');

    if(!dot || dot == filename) {
        return "";
    }

    return dot + 1;
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
        // TODO Load the CSV file.
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
