#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define SET_MAX_TEMP 25		//in order to save my time 
#define SET_MIN_TEMP 0

#include "datamgr.h" 
#include "datamgr.c" // I don't know why but I have to include this file or the compiler doesn't work


// #define FILE_ERROR(fp,error_msg) 	do { \
// 					  if ((fp)==NULL) { \
// 					    printf("%s\n",(error_msg)); \
// 					    exit(EXIT_FAILURE); \
// 					  }	\
// 					} while(0)
	
					
// #define NUM_MEASUREMENTS 100
// #define SLEEP_TIME	30	// every SLEEP_TIME seconds, sensors wake up and measure temperature
// #define NUM_SENSORS	8	// also defines number of rooms (currently 1 room = 1 sensor)
// #define TEMP_DEV 	5	// max afwijking vorige temperatuur in 0.1 celsius

// uint16_t room_id[NUM_SENSORS] = {1,2,3,4,11,12,13,14};
// uint16_t sensor_id[NUM_SENSORS] = {15,21,37,49,112,129,132,142};
// double sensor_temperature[NUM_SENSORS] = {15,17,18,19,20,23,24,25}; // starting temperatures

void printallsensor();// show all the data in the sensor_list, but you can't the next, head in this mian.c except in dplist.c

int main( int argc, char *argv[] )
{
	FILE* fpmap=fopen("room_sensor.map","r");
	FILE* fpdata=fopen("sensor_data","rb");
	 datamgr_parse_sensor_files(fpmap,fpdata);
	int totalsensor= datamgr_get_total_sensors();
	room_id_t roomid=0;
	sensor_ts_t timelast=0;
	sensor_id_t sensorid=0;
	for(int x=0;x<totalsensor;x++){
		sensorid=datamgr_get_id_index(x);
		roomid=datamgr_get_room_id(sensorid);
		timelast= datamgr_get_last_modified(sensorid);
		printf("Senor %"PRIu16 "of room%"PRIu16"is modified at %ld \n",sensorid,roomid,timelast);
	}
	 fclose(fpmap);
	 fclose(fpdata);
	 datamgr_free();
	return 0;
}

// print out all the information of sensors
void printallsensor(){
// 	dplist_node_t* the_sensor_node=NULL;
// 	if(sensor_list==NULL){
// 		return;
// 	}
// 	else{
// 	assert(sensor_list!=NULL);
// 	the_sensor_node=sensor_list->head;
// 	while(the_sensor_node->next!=NULL){
// 		sen_element_print(the_sensor_node->element);
// 		the_sensor_node=the_sensor_node->next;
// 	}
// }
}