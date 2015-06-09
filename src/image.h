#ifndef __IMAGE_H__
#define __IMAGE_H__

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>

#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>

#include "config.h"
#include "utils.h"


#define IMAGE_DEPTH CV_32F

typedef CvMat Image;

Image *read_image(const char *filename, const Config *config);
void show_image(const char *title, int x, int y, const Image *image);
void print_image(const Image *image);
int write_image(const char *filename, const Image *image, const Config *config);

#endif
