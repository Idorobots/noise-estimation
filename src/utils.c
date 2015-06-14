#include "utils.h"

char *trim(char *str) {
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if(str == NULL) {
        return NULL;
    } else if(str[0] == '\0') {
        return str;
    }

    len = strlen(str);
    endp = str + len;

    while(isspace(*frontp)) {
        ++frontp;
    }

    if(endp != frontp) {
        while(isspace(*(--endp)) && endp != frontp) {}
    }

    if(str + len - 1 != endp) {
        *(endp + 1) = '\0';

    } else if(frontp != str && endp == frontp) {
        *str = '\0';
    }

    endp = str;
    if(frontp != str) {
        while(*frontp) {
            *endp++ = *frontp++;
        }
        *endp = '\0';
    }

    return str;
}

char *split(char *string, char delimiter) {
    size_t length = strlen(string);

    for(size_t i = 0; i < length; ++i) {
        if(string[i] == delimiter) {
            string[i] = '\0';
            return &string[i+1];
        }
    }

    return NULL;
}

CvMat *normalize(const CvMat *data) {
    double max = 0;
    cvMinMaxLoc(data, NULL, &max, NULL, NULL, NULL);

#ifdef DEBUG
    printf("max: %f\n", max);
#endif

    CvMat *normalized = cvCloneMat(data);
    cvScale(data, normalized, 1.0/max, 0);

    return normalized;
}

double checksum(const CvMat *data) {
    return cvSum(data).val[0];
}
