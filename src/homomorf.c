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
        double x = (double) i - (height - 1.0)/2.0;

        for(size_t j = 0; j < width; ++j) {
            double y = (double) j - (width - 1.0)/2.0;

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

void clamp_lower(const Image *input, Image *output, double value) {
    CvSize size = cvGetSize(input);
    Image *mask = cvCreateMat(size.width, size.height, CV_8U);

    cvCopy(input, output, NULL);
    cvCmpS(input, value, mask, CV_CMP_LT);
    cvSet(output, cvScalar(value, 0, 0, 0), mask);

    cvReleaseMat(&mask);
}

double besseli0(double x) {
    double ax = fabs(x);
    double ans, y;

    if (ax < 3.75) {
        y = x / 3.75;
        y = y * y;
        ans = 1.0+y*(3.5156229+y*(3.0899424+y*(1.2067492+y*(0.2659732+y*(0.360768e-1+y*0.45813e-2)))));
    } else {
        y = 3.75 / ax;
        ans = (exp(ax)/sqrt(ax))*(0.39894228+y*(0.1328592e-1+y*(0.225319e-2+y*(-0.157565e-2+y*(0.916281e-2+y*(-0.2057706e-1+y*(0.2635537e-1+y*(-0.1647633e-1+y*0.392377e-2))))))));
    }
    return ans;
}

double besseli1(double x) {
    double ax = fabs(x);
    double ans, y;

    if (ax < 3.75) {
        y = x/3.75;
        y = y*y;
        ans=ax*(0.5+y*(0.87890594+y*(0.51498869+y*(0.15084934+y*(0.2658733e-1+y*(0.301532e-2+y*0.32411e-3))))));
    } else {
        y = 3.75 / ax;
        ans = 0.2282967e-1+y*(-0.2895312e-1+y*(0.1787654e-1-y*0.420059e-2));
        ans = 0.39894228+y*(-0.3988024e-1+y*(-0.362018e-2+y*(0.163801e-2+y*(-0.1031555e-1+y*ans))));
        ans *= (exp(ax)/sqrt(ax));
    }
    return x < 0.0 ? -ans : ans;
}

void divide(const Image *input1, const Image *input2, Image *output, double scale) {
    // NOTE Nonsaturating element-wise division.
    size_t width = input2->cols;
    size_t height = input2->rows;

    for(size_t i = 0; i < height; ++i) {
        for(size_t j = 0; j < width; ++j) {
            double value1 = input1 == NULL ? 1.0 : cvGet2D(input1, i, j).val[0];
            double value2 = cvGet2D(input2, i, j).val[0];
            cvSet2D(output, i, j, cvScalar(scale * value1 / value2, 0, 0, 0));
        }
    }
}

void bessel_approx(const Image *input, const Image *mean, const Image *sigma, Image *output) {
    Image *pre_z = cvCloneMat(input);
    cvMul(mean, input, pre_z, 1.0);

    Image *z = cvCloneMat(input);
    cvDiv(pre_z, sigma, z, 1.0);

    Image *eight = cvCloneMat(input);
    cvScale(z, eight, 8.0, 0);

    Image *square = cvCloneMat(input);
    cvMul(eight, eight, square, 1.0);

    Image *cube = cvCloneMat(input);
    cvMul(square, eight, cube, 1.0);

    Image *a = cvCloneMat(input);
    Image *b = cvCloneMat(input);
    Image *c = cvCloneMat(input);
    Image *d = cvCloneMat(input);

    cvSet(a, cvScalar(1.0, 0, 0, 0), NULL);
    divide(NULL, eight, b, -3.0);
    divide(NULL, square, c, -7.5);
    divide(NULL, cube, d, -52.5);

    Image *numerator = cvCloneMat(input);
    cvAdd(a, b, numerator, NULL);
    cvAdd(numerator, c, b, NULL);
    cvAdd(b, d, numerator, NULL);

    Image *denominator = cvCloneMat(input);
    divide(NULL, eight, b, 1.0);
    divide(NULL, square, c, 4.5);
    divide(NULL, cube, d, 37.5);

    cvAdd(a, b, denominator, NULL);
    cvAdd(denominator, c, b, NULL);
    cvAdd(b, d, denominator, NULL);

    Image *bessel = cvCloneMat(input);
    divide(numerator, denominator, bessel, 1.0);

    cvReleaseMat(&a);
    cvReleaseMat(&b);
    cvReleaseMat(&c);
    cvReleaseMat(&d);
    cvReleaseMat(&numerator);
    cvReleaseMat(&denominator);

    size_t width = input->cols;
    size_t height = input->rows;

    for(size_t i = 0; i < height; ++i) {
        for(size_t j = 0; j < width; ++j) {
            double value = cvGet2D(z, i, j).val[0];
            if(value < 1.5) {
                CvScalar b = cvScalar(besseli1(value) / besseli0(value), 0, 0, 0);
                cvSet2D(bessel, i, j, b);
            }
        }
    }

    CvSize size = cvGetSize(input);
    Image *mask = cvCreateMat(size.width, size.height, CV_8U);
    cvCmpS(z, 0.0, mask, CV_CMP_EQ);
    cvSet(bessel, cvScalar(0, 0, 0, 0), mask);

    cvMul(bessel, input, output, 1.0);

    cvReleaseMat(&bessel);
    cvReleaseMat(&numerator);
    cvReleaseMat(&denominator);
    cvReleaseMat(&eight);
    cvReleaseMat(&z);
    cvReleaseMat(&pre_z);
}

void em_mean(const Image *image, Image *mean, Image *sigma, size_t size, size_t iterations) {
    Image *square = cvCloneMat(image);
    cvMul(image, image, square, 1.0);

    Image *square_f = cvCloneMat(image);
    smooth_mean(square, square_f, size);

    Image *dssf = cvCloneMat(image);
    cvMul(square_f, square_f, dssf, 2.0);

    Image *quad = cvCloneMat(image);
    cvMul(square, square, quad, 1.0);

    Image *quad_f = cvCloneMat(image);
    smooth_mean(quad, quad_f, size);

    Image *diff = cvCloneMat(image);
    cvSub(dssf, quad_f, diff, NULL);

    Image *max = cvCloneMat(image);
    clamp_lower(diff, max, 0.0);

    Image *root = cvCloneMat(image);
    cvPow(max, root, 0.5);

    Image *mean_k = cvCloneMat(image);
    cvPow(root, mean_k, 0.5);

    Image *sigma_k = cvCloneMat(image);
    cvSub(square_f, root, diff, NULL);
    clamp_lower(diff, max, 0.01);
    cvScale(max, sigma_k, 0.5, 0);

#ifdef DEBUG
    printf("sum(mean_k): %lf\n", cvSum(mean_k).val[0]);
    printf("sum(sigma_k): %lf\n", cvSum(sigma_k).val[0]);
#endif

    cvReleaseMat(&quad_f);
    cvReleaseMat(&quad);
    cvReleaseMat(&dssf);

    // NOTE Temporary variables for the loop.
    Image *approx = cvCloneMat(image);
    Image *smooth = cvCloneMat(image);
    Image *mean_square = cvCloneMat(image);
    Image *half = cvCloneMat(image);

    while(iterations --> 0) {
        // Mean
        bessel_approx(image, mean_k, sigma_k, approx);
        smooth_mean(approx, smooth, size);
        clamp_lower(smooth, mean_k, 0.0);

        // Sigma
        cvMul(mean_k, mean_k, mean_square, 1.0);
        cvSub(square_f, mean_square, diff, NULL);
        cvScale(diff, half, 0.5, 0);
        clamp_lower(half, sigma_k, 0.01);

#ifdef DEBUG
        printf("sum(mean_k): %lf\n", cvSum(mean_k).val[0]);
        printf("sum(sigma_k): %lf\n", cvSum(sigma_k).val[0]);
#endif
    }
    cvReleaseMat(&smooth);
    cvReleaseMat(&approx);
    cvReleaseMat(&half);
    cvReleaseMat(&mean_square);

    cvPow(sigma_k, sigma, 0.5);
    cvCopy(mean_k, mean, NULL);

#ifdef DEBUG
    printf("sum(mean): %lf\n", cvSum(mean).val[0]);
    printf("sum(sigma): %lf\n", cvSum(sigma).val[0]);
    show_image("mean", 100, 500, mean);
#endif

    cvReleaseMat(&sigma_k);
    cvReleaseMat(&mean_k);
    cvReleaseMat(&max);
    cvReleaseMat(&diff);
    cvReleaseMat(&square_f);
    cvReleaseMat(&square);
}

Image *estimated_SNR(const Image *mean, const Image *sigma, const Config *config) {
    Image *sigma_f = cvCloneMat(mean);
    lpf(sigma, sigma_f, config->lpf_f_SNR);

    Image *SNR = cvCloneMat(mean);
    cvDiv(mean, sigma_f, SNR, 1.0);

#ifdef DEBUG
    show_image("SNR", 300, 500, SNR);
#endif

    cvReleaseMat(&sigma_f);
    return SNR;
}

void correct_rice(const Image *image, const Image *SNR, Image *correct) {
    double coefs[] = {
        -0.289549906258443,   -0.0388922575606330,   0.409867108141953,
        -0.355237628488567,    0.149328280945610,   -0.0357861117942093,
        0.00497952893859122, -0.000374756374477592, 0.0000118020229140092
    };

    Image *fc = cvCloneMat(image);
    cvSet(fc, cvScalar(coefs[0], 0, 0, 0), NULL);

    Image *pow = cvCloneMat(image);
    for(size_t i = 1; i < sizeof(coefs) / sizeof(coefs[0]); ++i) {
        cvPow(SNR, pow, (double) i);

        Image *scale = cvCloneMat(image);
        cvScale(pow, scale, coefs[i], 0);

        Image *fc_copy = cvCloneMat(fc);
        cvAdd(fc_copy, scale, fc, NULL);

        cvReleaseMat(&scale);
        cvReleaseMat(&fc_copy);
    }

    CvSize size = cvGetSize(image);
    Image *mask = cvCreateMat(size.width, size.height, CV_8U);
    cvCmpS(SNR, RICE_CORRECTION_THRESHOLD, mask, CV_CMP_GT);
    cvSet(fc, cvScalar(0, 0, 0, 0), mask);

    cvSub(image, fc, correct, NULL);

#ifdef DEBUG
    printf("sum(fc): %lf\n", cvSum(fc).val[0]);
    show_image("Corrected", 700, 500, correct);
#endif

    cvReleaseMat(&pow);
    cvReleaseMat(&mask);
    cvReleaseMat(&fc);
}

int homomorf_rice(const Image *input, const Image *SNR, Image **output, const Config *config) {
    Image *snr = NULL;

    if(SNR != NULL) {
        snr = cvCloneMat(SNR);
    }

    Image *mean = cvCloneMat(input);

    // NOTE We need to compute EM anyway if SNR is not supplied.
    if(SNR == NULL || config->ex_filter_type == 2) {
        // Expectation maximization.
        Image *sigma = cvCloneMat(input);
        em_mean(input, mean, sigma, config->ex_window_size, config->ex_iterations);

        snr = estimated_SNR(mean, sigma, config);
        cvReleaseMat(&sigma);
    }

    if(config->ex_filter_type == 1) {
        // Local mean.
        smooth_mean(input, mean, config->ex_window_size);
    } else if(config->ex_filter_type > 2) {
        printf("ERROR: Unknown filter type: %ld.\n", config->ex_filter_type);
        return -1;
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

    Image *filter2 = cvCloneMat(input);
    lpf(correct, filter2, config->lpf_f_Rice);

    Image *diff_exp = cvCloneMat(input);
    cvExp(filter2, diff_exp);

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
