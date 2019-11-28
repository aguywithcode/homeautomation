#ifndef dht11_h
#define dht11_h

#include <applibs/gpio.h>

#define DHT_OK  0
#define DHT_ERROR  -1
#define DHT_TIMEOUT  -2



typedef struct DhtReading {
	int humidity;
	int temperature;
}DhtReading;

int readDht(GPIO_Id gpioId, DhtReading* reading);
#endif