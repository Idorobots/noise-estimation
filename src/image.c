#include "image.h"

#define BUF_SIZE = (1024*64)

const char *get_extension(const char *filename) {
    char *dot = strrchr(filename, '.');

    if(!dot || dot == filename) {
        return "";
    }

#ifdef DEBUG
    printf("extension: '%s'\n", dot + 1);
#endif

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

int get_csv_size(const char *filename, size_t *width, size_t *height, const Config *config) {
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
    *width = 1 + count(line, num_bytes, config->csv_delimiter);

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

Image *read_csv_data(const char *filename, const Config *config) {
    size_t width = 0, height = 0;

    if(get_csv_size(filename, &width, &height, config) == -1) {
        return NULL;
    }

#ifdef DEBUG
    printf("width: %ld\nheight: %ld\n", width, height);
#endif

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
                rest = trim(split(rest, config->csv_delimiter));
            }
        }
    }

    free(line);
    fclose(file);
    return data;
}

int write_csv_data(const char *filename, const Image *image, const Config *config) {
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
            fprintf(file, "%.15lf", cvmGet(image, i, j));

            if(j < width-1) {
                fprintf(file, "%c ", config->csv_delimiter);
            }
        }
        fprintf(file, "\n");
    }

    fclose(file);
    return 0;
}

Image *read_image(const char *filename, const Config *config) {
    Image *image = NULL;

    if(filename == NULL) {
        return image;
    }

    const char *extension = get_extension(filename);

    if(strcmp(extension, "png") == 0) {
        // Easy case:
        Image *data = cvLoadImageM(filename, CV_LOAD_IMAGE_GRAYSCALE);

        if(data != NULL) {
            int size[2];
            cvGetDims(data, size);
            image = cvCreateMat(size[0], size[1], IMAGE_DEPTH);
            cvConvert(data, image);
            cvReleaseMat(&data);
        }
    } else if(strcmp(extension, "csv") == 0) {
        // We need to parse the CSV manually:
        image = read_csv_data(filename, config);
    } else {
        printf("ERROR: Unrecognized image file type: %s\n", extension);
    }

    return image;
}

double interpolate(double val, double y0, double x0, double y1, double x1) {
    return (val-x0)*(y1-y0)/(x1-x0) + y0;
}

double jet_base(double val) {
    if(val <= 0.125) return 0.0;
    else if(val <= 0.375) return interpolate(val, 0.0, 0.125, 1.0, 0.375);
    else if(val <= 0.625) return 1.0;
    else if(val <= 0.875) return interpolate(val, 1.0, 0.625, 0.0, 0.875);
    else return 0.0;
}

CvScalar jet(double gray) {
    // NOTE OpenCV stores pixels as BGR not RGB.
    CvScalar ret = {
        jet_base(gray + 0.25),
        jet_base(gray),
        jet_base(gray - 0.25)
    };

    return ret;
}

IplImage *apply_color_map(const Image *image, int colormap) {
    if(colormap == COLORMAP_GRAYSCALE) {
        IplImage *output = cvCreateImage(cvGetSize(image), IPL_DEPTH_32F, 1);
        cvGetImage(image, output);
        return cvCloneImage(output);
    } else if(colormap == COLORMAP_JET) {
        IplImage *output = cvCreateImage(cvGetSize(image), IPL_DEPTH_32F, 3);

        size_t width = image->cols;
        size_t height = image->rows;

        for(size_t i = 0; i < height; ++i) {
            for(size_t j = 0; j < height; ++j) {
                cvSet2D(output, i, j, jet(cvmGet(image, i, j)));
            }
        }

        return output;
    } else {
        printf("ERROR: Unrecognized colormap parameter: %d\n", colormap);
        return NULL;
    }
}

void show_image(const char *title, int x, int y, const Image *image) {
    Image *normalized = normalize(image);
    IplImage *processed = apply_color_map(normalized, COLORMAP_JET);

    cvNamedWindow(title, CV_WINDOW_AUTOSIZE);
    cvMoveWindow(title, x, y);
    cvShowImage(title, processed);

    cvReleaseImage(&processed);
    cvReleaseMat(&normalized);
}

void print_image(const Image *image) {
    size_t width = image->cols;
    size_t height = image->rows;

    for(size_t i = 0; i < height; ++i) {
        for(size_t j = 0; j < width; ++j) {
            printf("%.4lf ", cvmGet(image, i, j));
        }
        printf("\n");
    }
}

int write_image(const char *filename, const Image *image, const Config *config) {
    const char *extension = get_extension(filename);

    if(strcmp(extension, "png") == 0) {
        // Easy case:
        return cvSaveImage(filename, image, NULL);
    } else if (strcmp(extension, "csv") == 0) {
        return write_csv_data(filename, image, config);
    } else {
        printf("ERROR: Unrecognized image file type: %s\n", extension);
    }

    return -1;
}
