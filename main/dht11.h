#ifndef _DHT11_H_
#define _DHT11_H_

#include <stdint.h>

void dht11_init(void);
int dht11_read(float *temp, float *humi);

#endif
