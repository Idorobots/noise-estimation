#ifndef __HOMOMORF_H__
#define __HOMOMORF_H__

#include "config.h"
#include "image.h"

int homomorf_est(Image *input, Image **rician_map, Image **gaussian_map, Config *config);

#endif
