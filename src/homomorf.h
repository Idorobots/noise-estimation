#ifndef __HOMOMORF_H__
#define __HOMOMORF_H__

#include <math.h>

#include "config.h"
#include "image.h"


#define EULER_GAMMA 0.5772156649015328606
#define PI 3.14159265358979323846

#define RICE_CORRECTION_THRESHOLD 7.0

int homomorf_est(const Image *input, Image **SNR, Image **rician_map, Image **gaussian_map, const Config *config);

#endif
