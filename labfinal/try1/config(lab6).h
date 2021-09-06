#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint16_t sensor_id_t;      //uint16_6 is just a long int of 16bits
typedef uint16_t room_id_t;
typedef double sensor_value_t;     
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

typedef struct {
  sensor_id_t id;  //id is 16 bits
  sensor_value_t value;  //value is a double
  sensor_ts_t ts;  //      ts is time_t, the seconds have past since 1970.01.01.0.00:00
  room_id_t rmid;
  double* lavase; // last value sequency, a pointer to the array of the last RUN_AVG_LENGTH temperature value
  int	sesize;// the variable takes down the size of the array lavase
} sensor_data_t;

#endif /* _CONFIG_H_ */

