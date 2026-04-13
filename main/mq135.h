#ifndef _MQ135_H_
#define _MQ135_H_

#include <stdint.h>

void mq135_init(void);
int mq135_read(void);

int mq135_read_raw(void);
int mq135_read_mv(int *out_mv);
float mq135_read_ppm(void);
int mq135_read_data(int *out_raw, int *out_mv, float *out_ppm);

#endif
