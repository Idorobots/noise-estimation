#include "homomorf.h"

int homomorf_est(const Image *input, Image **rician_map, Image **gaussian_map, const Config *config) {
    *rician_map = cvCloneMat(input);
    *gaussian_map = cvCloneMat(input);

    // TODO Actually implement the algorithm.

    return 0;
}
