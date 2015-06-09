#include "homomorf.h"

void smooth_mean(const Image *input, Image *output, size_t size) {
    // TODO Add border.
    cvSmooth(input, output, CV_BLUR, size, size, 0, 0);
}

void lpf(const Image *input, Image *output, const Config *config) {
    // TODO Actually implement this.
    cvCopy(input, output, NULL);
}

int homomorf_gauss(const Image *input, Image **output, const Config *config) {
    Image *mean = cvCloneMat(input);
    smooth_mean(input, mean, config->ex_window_size);

    Image *diff = cvCloneMat(input);
    cvAbsDiff(input, mean, diff);

    // NOTE No need to remove zeros, since cvLog handles them properly.
    Image *diff_log = cvCloneMat(input);
    cvLog(diff, diff_log);

    Image *filter = cvCloneMat(input);
    lpf(diff_log, filter, config);

    Image *diff_exp = cvCloneMat(input);
    cvExp(filter, diff_exp);

    *output = cvCloneMat(input);
    cvScale(diff_exp, *output, 2 / sqrt(2) * exp(EULER_GAMMA/2), 0);

    cvReleaseMat(&diff_exp);
    cvReleaseMat(&filter);
    cvReleaseMat(&diff_log);
    cvReleaseMat(&diff);
    cvReleaseMat(&mean);
    return 0;
}

int homomorf_rice(const Image *input, Image **output, const Config *config) {
    // TODO Actually implement the algorithm.
    *output = cvCloneMat(input);
    return 0;
}

int homomorf_est(const Image *input, Image **rician_map, Image **gaussian_map, const Config *config) {
    if(homomorf_gauss(input, gaussian_map, config) == -1) {
        printf("Couldn't compute the Gaussian map.\n");
        return -1;
    }

    if(homomorf_rice(input, rician_map, config) == -1) {
        printf("Couldn't compute the Rician map.\n");
        return -1;
    }

    return 0;
}
