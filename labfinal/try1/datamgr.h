#ifndef DATAMGR_H_
#define DATAMGR_H_

#include "config.h"
//#include "dplist.c"
//#include "datamgr.c"

#ifndef RUN_AVG_LENGTH
  #define RUN_AVG_LENGTH 5
#endif

#ifndef SET_MAX_TEMP
  #error SET_MAX_TEMP not set
#endif

#ifndef SET_MIN_TEMP
  #error SET_MIN_TEMP not set
#endif
#define NUM_SENSORS	8	// also defines number of rooms (currently 1 room = 1 sensor)

/*
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 */
#define ERROR_HANDLER(condition,...) 	do { \
					  if (condition) { \
					    printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
					    exit(EXIT_FAILURE); \
					  }	\
					} while(0)



void * sen_element_copy(void * src_element);// the function for callback functions			  
void sen_element_free(void ** element);
int sen_element_compare(void * x, void * y);//return 2,	1, 0	
void sen_element_print(void* element);			
sensor_data_t* findsensordatat(sensor_id_t id);//find the sensor_data_t by the id
sensor_id_t datamgr_get_id_index(int index);// get the sensor id from the snsor list by the index
/*
 *  This method holds the core functionality of your datamgr. It takes in 2 file pointers to the sensor files and parses them. 
 *  When the method finishes all data should be in the internal pointer list and all log messages should be printed to stderr.
 */
void  datamgr_parse_sensor_files(FILE * fp_sensor_map, FILE * fp_sensor_data);


/*
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors 
 * will not return a valid result
 */
void datamgr_free();
    
/*   
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid 
 */
uint16_t datamgr_get_room_id(sensor_id_t sensor_id);


/*
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid 
 */
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id);


/*
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid 
 */
time_t datamgr_get_last_modified(sensor_id_t sensor_id);


/*
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 */
int datamgr_get_total_sensors();
   


#endif  //DATAMGR_H_

