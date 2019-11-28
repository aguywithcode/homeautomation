#include <applibs/gpio.h>
#include <applibs/log.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "dht11.h"

///<summary>
///		Reads the temperature and humidity from the DHT11
///</summary>

int readDht(GPIO_Id gpioId, DhtReading* reading)
{
	const struct timespec lowSleepTime = { 0,18000 };
	const struct timespec highSleepTime = { 0,40000 };
	// BUFFER TO RECEIVE
	uint8_t bits[5];
	uint8_t cnt = 7;
	uint8_t idx = 0;

	// EMPTY BUFFER
	for (int i = 0; i < 5; i++) bits[i] = 0;

	// REQUEST SAMPLE
	int fd = GPIO_OpenAsOutput(gpioId, GPIO_OutputMode_OpenDrain, GPIO_Value_High);
	if (fd < 0) {
		Log_Debug("Error: Could not open DHT11 GPIO: %s (%d).\n", strerror(errno), errno);
		return DHT_ERROR;
	}

	GPIO_SetValue(fd, GPIO_Value_Low);
	nanosleep(&lowSleepTime, NULL);
	GPIO_SetValue(fd, GPIO_Value_High);
	nanosleep(&highSleepTime, NULL);
	close(fd);
	fd=GPIO_OpenAsInput(gpioId);

	// ACKNOWLEDGE or TIMEOUT
	GPIO_Value output = GPIO_Value_Low;
	unsigned int loopCnt = 10000;
	while (output == GPIO_Value_Low)
	{
		if (loopCnt-- == 0)
		{
			return DHT_TIMEOUT;
		}
		GPIO_GetValue(fd, &output);
	}
	Log_Debug("Took %d polls", 10000 - loopCnt);

	loopCnt = 10000;
	while (output == GPIO_Value_High)
	{
		if (loopCnt-- == 0)
		{
			return DHT_TIMEOUT;
		}
		if (GPIO_GetValue(fd, &output) < 0)
		{
			Log_Debug("Error: Error reading from DHT: %s (%d).\n", strerror(errno), errno);
			return DHT_ERROR;
		}
	}

	// READ OUTPUT - 40 BITS => 5 BYTES or TIMEOUT
	for (int i = 0; i < 40; i++)
	{
		loopCnt = 10000;
		while (output == GPIO_Value_Low)
		{
			if (loopCnt-- == 0)
			{
				return DHT_TIMEOUT;
			}
			if (GPIO_GetValue(fd, &output) < 0)
			{
				Log_Debug("Error: Error reading from DHT: %s (%d).\n", strerror(errno), errno);
				return DHT_ERROR;
			}
		}

		struct timespec time;
		int result = clock_gettime(CLOCK_MONOTONIC, &time);
		if (result != 0)
		{
			return DHT_ERROR;
		}
		unsigned long t = time.tv_nsec;

		loopCnt = 10000;
		while (output == GPIO_Value_High)
		{
			if (loopCnt-- == 0)
			{
				return DHT_TIMEOUT;
			}
			GPIO_GetValue(fd, &output);
		}

		struct timespec currentTime;
		result = clock_gettime(CLOCK_MONOTONIC, &currentTime);
		if (result != 0)
		{
			return DHT_ERROR;
		}
		if ((currentTime.tv_nsec- t) > 40000) bits[idx] |= (1 << cnt);
		if (cnt == 0)   // next byte?
		{
			cnt = 7;    // restart at MSB
			idx++;      // next byte!
		}
		else cnt--;
	}

	// WRITE TO RIGHT VARS
		// as bits[1] and bits[3] are allways zero they are omitted in formulas.
	reading->humidity= bits[0];
	reading->temperature = bits[2];

	uint8_t sum = bits[0] + bits[2];

	if (bits[4] != sum) return DHT_ERROR;
	close(fd);
	return DHT_OK;
}
//
// END OF FILE