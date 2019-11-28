#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <applibs/log.h>
#include <applibs/gpio.h>
#include <hw/den_thermostat.h>
#include "dht11.h"

int main(void)
{
    const struct timespec sleepTime = {1, 0};
    while (true) {
		Log_Debug("Reading data from DHT11.\n");
		DhtReading reading;
		if(readDht(DEN_DHT11, &reading)!=DHT_OK)
		{
			Log_Debug("Error reading data from DHT: %s (%d).\n", strerror(errno), errno);
			return -1;
		}
		Log_Debug("Temperature: %d\n", reading.temperature);
		Log_Debug("Humidity: %d\n", reading.humidity);
		nanosleep(&sleepTime, NULL);
    }
}
