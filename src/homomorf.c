#include "homomorf.h"

void smooth_mean(const Image *input, Image *output, size_t size) {
    size_t width = input->cols + size;
    size_t height = input->rows + size;
    size_t border_size = floor(size / 2);

    Image *border = cvCreateMat(width, height, IMAGE_DEPTH);
    CvPoint offset = cvPoint(border_size, border_size);
    cvCopyMakeBorder(input, border, offset, IPL_BORDER_REPLICATE, cvScalarAll(0));

#ifdef DEBUG
    show_image("With border", 100, 500, border);
#endif

    Image *smooth = cvCloneMat(border);
    cvSmooth(border, smooth, CV_BLUR, size, size, 0, 0);

    Image *no_border = cvCloneMat(input);
    cvGetSubRect(smooth, no_border, cvRect(border_size, border_size, input->cols, input->rows));

#ifdef DEBUG
    show_image("Without border", 300, 500, no_border);
#endif

    cvCopy(no_border, output, NULL);
    cvReleaseMat(&no_border);
    cvReleaseMat(&border);
    cvReleaseMat(&smooth);
}

Image *gaussian_kernel(size_t width, size_t height, double sigma) {
    Image *kernel = cvCreateMat(width, height, IMAGE_DEPTH);

    for(size_t i = 0; i < height; ++i) {
        double x = i;
        x -= height/2;

        for(size_t j = 0; j < width; ++j) {
            double y = j;
            y -= width/2;

            double g = exp(-(x*x + y*y)/(2 * sigma * sigma)) / (2 * PI * sigma * sigma);
            cvmSet(kernel, i, j, g);
        }
    }

    Image *normalized = normalize(kernel);
    cvReleaseMat(&kernel);

    return normalized;
}

void lpf(const Image *input, Image *output, double sigma) {
    // TODO FFT?

    Image *dct = cvCloneMat(input);
    cvDCT(input, dct, CV_DXT_FORWARD);

#ifdef DEBUG
    show_image("Unfiltered", 500, 500, normalize(dct));
#endif

    Image *kernel = gaussian_kernel(2*input->cols, 2*input->rows, 2*sigma);
    Image *g = cvCloneMat(input);
    cvGetSubRect(kernel, g, cvRect(input->cols, input->rows, input->cols, input->rows));

    Image *filter = cvCloneMat(input);
    cvMul(g, dct, filter, 1);

#ifdef DEBUG
    print_image(gaussian_kernel(5, 5, sigma));
    show_image("Filter", 700, 500, g);
#endif

    Image *idct = cvCloneMat(input);
    cvDCT(filter, idct, CV_DXT_INVERSE);

    cvCopy(idct, output, NULL);

#ifdef DEBUG
    show_image("Filtered", 900, 500, idct);
#endif

    cvReleaseMat(&idct);
    cvReleaseMat(&filter);
    cvReleaseMat(&g);
    cvReleaseMat(&kernel);
    cvReleaseMat(&dct);
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
    lpf(diff_log, filter, config->lpf_f);

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

void em_mean(const Image *input, Image *mean, Image *sigma, size_t size, size_t iterations) {
    // TODO Actually implement this.
    smooth_mean(input, mean, size);
}

Image *estimated_SNR(const Image *mean, const Image *sigma, const Config *config) {
    // TODO Actually implement this.
    return cvCloneMat(mean);
}

void correct_rice(const Image *image, const Image *SNR, Image *correct) {
    // TODO Actually implement this.
    cvCopy(image, correct, NULL);
}

int homomorf_rice(const Image *input, const Image *SNR, Image **output, const Config *config) {
    Image *snr = NULL;

    if(SNR != NULL) {
        snr = cvCloneMat(SNR);
    }

    Image *mean = cvCloneMat(input);

    if(config->ex_filter_type == 1) {
        // Local mean.
        smooth_mean(input, mean, config->ex_window_size);
    } else {
        // Expectation maximization.
        Image *sigma = cvCloneMat(input);
        em_mean(input, mean, sigma, config->ex_window_size, config->ex_iterations);

        if(SNR == NULL) {
            snr = estimated_SNR(mean, sigma, config);
        }

        cvReleaseMat(&sigma);
    }

    Image *diff = cvCloneMat(input);
    cvAbsDiff(input, mean, diff);

    // NOTE No need to remove zeros, since cvLog handles them properly.
    Image *diff_log = cvCloneMat(input);
    cvLog(diff, diff_log);

    Image *filter = cvCloneMat(input);
    lpf(diff_log, filter, config->lpf_f);

    Image *correct = cvCloneMat(input);
    correct_rice(filter, snr, correct);

    Image *diff_exp = cvCloneMat(input);
    cvExp(correct, diff_exp);

    *output = cvCloneMat(input);
    cvScale(diff_exp, *output, 2 / sqrt(2) * exp(EULER_GAMMA/2), 0);

    cvReleaseMat(&diff_exp);
    cvReleaseMat(&correct);
    cvReleaseMat(&snr);
    cvReleaseMat(&filter);
    cvReleaseMat(&diff_log);
    cvReleaseMat(&diff);
    cvReleaseMat(&mean);
    return 0;
}

int homomorf_est(const Image *input, const Image *SNR, Image **rician_map, Image **gaussian_map, const Config *config) {
    if(homomorf_gauss(input, gaussian_map, config) == -1) {
        printf("ERROR: Couldn't compute the Gaussian map.\n");
        return -1;
    }

    if(homomorf_rice(input, SNR, rician_map, config) == -1) {
        printf("ERROR: Couldn't compute the Rician map.\n");
        return -1;
    }

    return 0;
}
