#include "datamgr.h"

dplist_t* sensor_list=NULL;

//call back function
void * sen_element_copy(void * src_element){
	//without creating a new sensor_data_t variable, the compiler doesn't allow dereference of the pointer
	sensor_data_t oldele=*(sensor_data_t*)(src_element);
	// use = direclty to copy a structure
	sensor_data_t* newnode=(sensor_data_t*)malloc(sizeof(sensor_data_t));
	newnode->id=oldele.id;
	newnode->value=oldele.value;
	newnode->ts=oldele.ts;
	newnode->rmid=oldele.rmid;
	return newnode;
}		  
void sen_element_free(void ** element){
	free((sensor_data_t*)*element);
}
// if the values are all the same, return 2;
// id and rmid are the same, return 1
// id and rmid not the same, return 0
int sen_element_compare(void * x, void * y){
	int comresult=0;
	sensor_data_t xh=*(sensor_data_t*)x;
	sensor_data_t yh=*(sensor_data_t*)y;
	if(xh.id==yh.id&&xh.rmid==yh.rmid){
		comresult=1;
		if(xh.value==yh.value&&xh.ts==yh.ts){comresult=2;}
		}
	else{comresult=0;}
	return comresult;
}	
//print the content of a sensor
void sen_element_print(void* element){
	sensor_data_t eleprin=*(sensor_data_t*)element;
	sensor_id_t theid=eleprin.id;
	room_id_t	thermid=eleprin.rmid;
	double		therunningavg=eleprin.value;
	time_t		thelastmod=eleprin.ts;
	printf("Senor%" PRIu16 "in room%"PRIu16 "has the running avg %lf is last modified %ld \n",theid,thermid,therunningavg,thelastmod);
	
}


/*
 *  This method holds the core functionality of your datamgr. It takes in 2 file pointers to the sensor files and parses them. 
 *  When the method finishes all data should be in the internal pointer list and all log messages should be printed to stderr.
 */
void datamgr_parse_sensor_files(FILE * fp_sensor_map, FILE * fp_sensor_data){
	assert(fp_sensor_map!=NULL&&fp_sensor_data!=NULL);
	sensor_list=dpl_create(&sen_element_copy,&sen_element_free,&sen_element_compare);

	sensor_id_t senid=0;
	room_id_t	roomid=0;
	double tmp=0;
	sensor_ts_t tm=0;

	rewind(fp_sensor_map);
	//int i=0;//used for printing

	while(fscanf(fp_sensor_map,"%hd %hd\n",&senid,&roomid)!=EOF){
		sensor_data_t* newsensor=(sensor_data_t*)malloc(sizeof(sensor_data_t));
		newsensor->id=senid;
		newsensor->rmid=roomid;
		newsensor->ts=0;// first set it to zero and then initialize it 
		newsensor->value=0;
		dpl_insert_at_index(sensor_list,newsensor,10,true);//insert it at the end of the list
		//printf("%d\n",i++ );
		sen_element_print(newsensor);
	}

	



 




}


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

