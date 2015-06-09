#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc_c.h>


char *trim(char *str);
char *split(char *string, char delimiter);
CvMat *normalize(const CvMat *data);

#endif
