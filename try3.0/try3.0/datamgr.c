     #include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib/dplist.h"
#include "datamgr.h"

dplist_t* sensor_list=NULL;

//call back function
void * sen_element_copy(void * src_element){
	//without creating a new sensor_datamgr_t variable, the compiler doesn't allow dereference of the pointer
	sensor_datamgr_t oldele=*(sensor_datamgr_t*)(src_element);
	// use = direclty to copy a structure
	sensor_datamgr_t* newnode=(sensor_datamgr_t*)malloc(sizeof(sensor_datamgr_t));
	newnode->id=oldele.id;
	newnode->value=oldele.value;
	newnode->ts=oldele.ts;
	newnode->rmid=oldele.rmid;
	newnode->lavase=oldele.lavase;
	newnode->sesize=oldele.sesize;
	return newnode;
}		  
void sen_element_free(void ** element){
	free(((sensor_datamgr_t*)*element)->lavase);
	free((sensor_datamgr_t*)*element);
}
// if the values are all the same, return 2;
// id and rmid are the same, return 1
// id and rmid not the same, return 0
int sen_element_compare(void * x, void * y){
	int comresult=0;
	sensor_datamgr_t xh=*(sensor_datamgr_t*)x;
	sensor_datamgr_t yh=*(sensor_datamgr_t*)y;
	if(xh.id==yh.id){
		comresult=1;
		if(xh.value==yh.value&&xh.ts==yh.ts){comresult=2;}
		}
	else{comresult=0;}
	return comresult;
}	
//print the content of a sensor
void sen_element_print(void* element){
	sensor_datamgr_t eleprin=*(sensor_datamgr_t*)element;
	sensor_id_t theid=eleprin.id;
	room_id_t	thermid=eleprin.rmid;
	double		therunningavg=eleprin.value;
	time_t		thelastmod=eleprin.ts;
	printf("Sensor %" PRIu16 " in room %"PRIu16 " has the running avg %lf is last modified %ld \n",theid,thermid,therunningavg,thelastmod);
	
}

//find the pointer to the sensor_datamgr_t by the id
//we can't use the variable in dplist.c, we can only use the function 
sensor_datamgr_t* findsensordatat(sensor_id_t fid){
	int x=0;
	sensor_datamgr_t* sensorptr=NULL;
	int found=0;
	while(x<NUM_SENSORS &&found==0){
		sensorptr=(sensor_datamgr_t*)dpl_get_element_at_index(sensor_list,x);
		if(sensorptr->id==fid){
			found=1;
			}
		x++;
	}
	if(sensorptr->id==fid){
		//perror("id is found in function!");  //this is for debugging
		return sensorptr;

	}
	else{
		return NULL;
	}
	// if(found==0){return NULL;}// didn't find the id, return NULL
	// else{return sensorptr;}
}

// get the sensor id from the snsor list by the index
sensor_id_t datamgr_get_id_index(int index){
	sensor_datamgr_t* thesensor=dpl_get_element_at_index(sensor_list,index);
	return (thesensor->id);
}
/*
 *  This method holds the core functionality of your datamgr. It takes in 2 file pointers to the sensor files and parses them. 
 *  When the method finishes all data should be in the internal pointer list and all log messages should be printed to stderr.
 */
void datamgr_parse_sensor_files(FILE * fp_sensor_map, FILE * fp_sensor_data){
	assert(fp_sensor_map!=NULL&&fp_sensor_data!=NULL);
	sensor_list=dpl_create(&sen_element_copy,&sen_element_free,&sen_element_compare);

	sensor_id_t idv=0;// these are only variables for manipulating the value in this functions
	room_id_t	rmidv=0;
	//sensor_value_t valuev=0;
	sensor_ts_t tsv=0;
	sensor_value_t tmpv=0;

	rewind(fp_sensor_map);
	//int i=0;//used for printing

	while(fscanf(fp_sensor_map,"%hd %hd\n",&rmidv,&idv)!=EOF){
		sensor_datamgr_t* newsensor=(sensor_datamgr_t*)malloc(sizeof(sensor_datamgr_t));
		newsensor->id=idv;
		newsensor->rmid=rmidv;
		newsensor->value=0;
		newsensor->ts=0;// first set it to zero and then initialize it 
		newsensor->lavase=(sensor_value_t*)malloc(RUN_AVG_LENGTH*sizeof(sensor_value_t));
		newsensor->sesize=0;
		dpl_insert_at_index(sensor_list,newsensor,10,false);//insert it at the end of the list
		//printf("%d\n",i++ );
		sen_element_print(newsensor);
	}


	assert(sensor_list!=NULL);
	rewind(fp_sensor_data);
	sensor_datamgr_t* thedata=NULL;
	while(feof(fp_sensor_data)==0){
			fread(&idv,sizeof(sensor_id_t),1,fp_sensor_data);
			fread(&tmpv,sizeof(sensor_value_t),1,fp_sensor_data);
			fread(&tsv,sizeof(sensor_ts_t),1,fp_sensor_data);// read a line value of one sensor
			thedata=findsensordatat(idv);
			//printf("The id is %d \n",idv);
			if(thedata==NULL){perror("Invalid id number!");}
			else{
				assert(thedata->id==idv);
					// now we can start to maniulate the sensor value
				thedata->ts=tsv;//first assign the ts
				//next deal with lavarse, sesize and value
				if(thedata->sesize<RUN_AVG_LENGTH){// the lavase is still not full, don't calculate and just add the value here
					*((thedata->lavase)+(thedata->sesize))=tmpv;
					(thedata->sesize)++;
					thedata->value=datamgr_get_avg(idv);// here the value should be 0
					//perror("insertnewok");
					//sen_element_print(thedata);
				}
				else{// the lavase is full now, delete the first one and add the new value at the end
					for(int j=0;j<(RUN_AVG_LENGTH-1);j++){
						*((thedata->lavase)+j)=*((thedata->lavase)+j+1);
					}
					*((thedata->lavase)+RUN_AVG_LENGTH-1)=tmpv;
                       thedata->value=datamgr_get_avg(idv);
                      // perror("insertoldok");
                       //sen_element_print(thedata);
				}
				if(thedata->sesize>=RUN_AVG_LENGTH){
					if(thedata->value<SET_MIN_TEMP){
					fprintf(stderr, "Room %"PRIu16 " is too cold!\n",thedata->rmid );
				 printf("Room %"PRIu16 " measured by sensor %"PRIu16" at %ld is too cold! %g C\n",thedata->rmid,thedata->id,thedata->ts,thedata->value);
				// printf("Room is too cold!\n");
				}	

					if(thedata->value>SET_MAX_TEMP){
					fprintf(stderr, "Room %"PRIu16 " is too hot!\n",thedata->rmid );
				 printf("Room %"PRIu16 "measured by sensor %"PRIu16"at %ld is too hot! %g C\n",thedata->rmid,thedata->id,thedata->ts,thedata->value);
				// printf("Room is too hot!\n");
				}
}}}
}

/*
 * Reads continiously all data from the shared buffer data structure, parse the room_id's
 * and calculate the running avarage for all sensor ids
 * When *buffer becomes NULL the method finishes. This method will NOT automatically free all used memory
 */
void datamgr_parse_sensor_data(FILE * fp_sensor_map, sbuffer_t ** buffer){
	sensor_id_t idv=0;// these are only variables for manipulating the value in this functions
	room_id_t	rmidv=0;
	sensor_ts_t tsv=0;
	sensor_value_t tmpv=0;
if(sensor_list==NULL){
	assert(fp_sensor_map!=NULL);
	sensor_list=dpl_create(&sen_element_copy,&sen_element_free,&sen_element_compare);
	rewind(fp_sensor_map);
	while(fscanf(fp_sensor_map,"%hd %hd\n",&rmidv,&idv)!=EOF){
		sensor_datamgr_t* newsensor=(sensor_datamgr_t*)malloc(sizeof(sensor_datamgr_t));
		newsensor->id=idv;
		newsensor->rmid=rmidv;
		newsensor->value=0;
		newsensor->ts=0;// first set it to zero and then initialize it 
		newsensor->lavase=(sensor_value_t*)malloc(RUN_AVG_LENGTH*sizeof(sensor_value_t));
		newsensor->sesize=0;
		int size=dpl_size(sensor_list);
		dpl_insert_at_index(sensor_list,newsensor,size+1,false);//insert it at the end of the list
		sen_element_print(newsensor);
	}	
}
		assert(sensor_list!=NULL);
		sensor_datamgr_t* thedata=NULL;
int res;
sensor_data_t  data_got;
res=sbuffer_remove_in_datamgr(*buffer,&data_got);
while(res!=1&&res!=-1)
{
    if(res==0||res==3)
    {
    printf("datamgr read some data!");
    //Can you make sure the data_got is valid?// yes I can now!!!
    	idv=data_got.id;
    	tmpv=data_got.value;
    	tsv=data_got.ts;
    	thedata=findsensordatat(idv);
		//printf("The id is %d \n",idv);
			if(thedata==NULL){
				perror("Invalid id number!");
					time_t timer=time(NULL);
					char print_time[11];//at leatst 11
					sprintf(print_time,"%ld",timer);
					char idstring[5];
					sprintf(idstring,"%u", idv);
					char * message=(char *)malloc(70*sizeof(char));
					strcpy(message, print_time);
					strcat(message," Received sensor data with invalid sensor node ID");
					strcat(message, idstring);
					strcat(message, "!\n");
  					send_to_log(message);
  					free(message);
			}
			else{
				assert(thedata->id==idv);
					// now we can start to maniulate the sensor value
				thedata->ts=tsv;//first assign the ts
				//next deal with lavarse, sesize and value
				if(thedata->sesize<RUN_AVG_LENGTH){// the lavase is still not full, don't calculate and just add the value here
					*((thedata->lavase)+(thedata->sesize))=tmpv;
					(thedata->sesize)++;
					thedata->value=datamgr_get_avg(idv);// here the value should be 0
					//perror("insertnewok");
					//sen_element_print(thedata);
				}
				else{// the lavase is full now, delete the first one and add the new value at the end
					for(int j=0;j<(RUN_AVG_LENGTH-1);j++){
						*((thedata->lavase)+j)=*((thedata->lavase)+j+1);
					}
					*((thedata->lavase)+RUN_AVG_LENGTH-1)=tmpv;
                       thedata->value=datamgr_get_avg(idv);
                      // perror("insertoldok");
                       //sen_element_print(thedata);
				}
				if(thedata->sesize>=RUN_AVG_LENGTH){
					time_t timer=time(NULL);
					char print_time[11];//at leatst 11
					sprintf(print_time,"%ld",timer);

					char idstring[5];
					sprintf(idstring,"%u", thedata->id);

					char runningavg[10];
					sprintf(runningavg,"%f",thedata->value);
					if(thedata->value<SET_MIN_TEMP){
					//fprintf(stderr, "Room %"PRIu16 " is too cold!\n",thedata->rmid );
				 printf("Room %"PRIu16 " measured by sensor %"PRIu16" at %ld is too cold! %g C\n",thedata->rmid,thedata->id,thedata->ts,thedata->value);
					char * message=(char *)malloc(120*sizeof(char));
					strcpy(message, print_time);
					strcat(message," The sensor node with sensor id ");
					strcat(message, idstring);
					strcat(message, " reports it's too cold (running avg temperature =");
					strcat(message, runningavg);
					strcat(message, ")!\n");
  					send_to_log(message);
  					free(message);
				}	
					if(thedata->value>SET_MAX_TEMP){
					//fprintf(stderr, "Room %"PRIu16 " is too hot!\n",thedata->rmid );
				 printf("Room %"PRIu16 "measured by sensor %"PRIu16"at %ld is too hot! %g C\n",thedata->rmid,thedata->id,thedata->ts,thedata->value);
					char * message=(char *)malloc(120*sizeof(char));
					strcpy(message, print_time);
					strcat(message," The sensor node with sensor id ");
					strcat(message, idstring);
					strcat(message, " reports it's too hot (running avg temperature =");
					strcat(message, runningavg);
					strcat(message, ")!\n");
  					send_to_log(message);
  					free(message);			

				}}}
	sbuffer_print(buffer);
	}
	res=sbuffer_remove_in_datamgr(*buffer,&data_got);
}//the while loop for wrting data into shared buffer


}//endof the datamgr_parse_sensor_data ()


/*
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors 
 * will not return a valid result
 */
void datamgr_free(){
	 dpl_free(&sensor_list,true);
	 sensor_list=NULL;
}
    
/*   
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid 
 */
uint16_t datamgr_get_room_id(sensor_id_t sensor_id){
	sensor_datamgr_t* sensortofind=findsensordatat(sensor_id);
	if(sensortofind!=NULL){
		return (sensortofind->rmid);
	}
	else{
		ERROR_HANDLER(1,"NULL pointer to find room id!");
		return -1;
	}
}


/*
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid 
 */
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id){
	sensor_datamgr_t* sensor_data=findsensordatat(sensor_id);
	if(sensor_data==NULL){
		ERROR_HANDLER(1,"NULL pointer in calculating avg!");
		return 0;
	}
	if(sensor_data->sesize<RUN_AVG_LENGTH){return 0;}
	else{
		sensor_value_t sum=0;
		sensor_value_t avg=0;
		for(int x=0;x<RUN_AVG_LENGTH;x++){
			sum=sum+ *((sensor_data->lavase)+x);
		}
		avg=sum/RUN_AVG_LENGTH;
		return avg;
	}
}


/*
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid 
 */
time_t datamgr_get_last_modified(sensor_id_t sensor_id){
	sensor_datamgr_t* sensortofind=findsensordatat(sensor_id);
	if(sensortofind!=NULL){
		return (sensortofind->ts);
	}
	else{
		ERROR_HANDLER(1,"NULL pointer to find room id!");
		return -1;
	}
}


/*
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 */
int datamgr_get_total_sensors(){
	int totalsensor = dpl_size(sensor_list);
	return totalsensor;
}

