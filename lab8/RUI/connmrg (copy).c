#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include "config.h"
#include "dplist.h"
#include "tcpsock.h"
#include "connmrg.h"

#ifndef TIMEOUT
  #error TIMEOUT not set
#endif
void *element_copy(void * element);  // Duplicate 'element'; If needed allocated new memory for the duplicated element.
void element_free(void ** element); // If needed, free memory allocated to element
int element_compare(void * x, void * y);// Compare two element elements; returns -1 if x<y, 0 if x==y, or 1 if x>y 
dplist_t * socket_list=NULL;
struct socket_sensor{
	tcpsock_t socket;
	time_t	  ts;
};

/*This methods hold the core fuctionality of your connmgr. It starts listening on the given port and 
* when a sensor node connects, it writes the data to a sensor_data_recv file. This file must have the 
* same formet as  the sensor data file in assignment 6 and 7
*/
void connmgr_listen(int port_number){

  tcpsock_t *new_socket;
  int max_client=20;
  tcpsock_t *client_socket[20];// the array of client pointers 
  sensor_data_t data;
  int bytes,result,sd,activity=0, max_sd=0;
  fd_set readfds;
  
  FILE * fp_bin;

 

   



  








  
  
     
   
  
  //for all the socket, the time should be checked and free those sockets silent for too long
  if(sd>0){
    //check the time ts here
    //delete it if timeout
  }
}//for every socket in socket_client[max_client] 

//here check for the master socket, free it if the time is out


}//for the endless loop	




/*This method shoud be called to clean up the connmgr, and to free all used memory.
* After this no new connection will be accepted
*/
void connmgr_free(){

}


/*This method is called when TIMEOUT isn't defined at compile time
*/
// void print_help(void)
// {
//   printf("Use this program with 1 command line options: \n");
//   printf("\t%-15s : TCP server port number\n", "\'server port\'");
// }

 // Duplicate 'element'; If needed allocated new memory for the duplicated element.
void * element_copy(void * element){
	socket_sensor_t oldsocket=*(socket_sensor_t*)(element);
	socket_sensor_t *newsocket=(socket_sensor_t *)malloc(sizeof(socket_sensor_t));
	newsocket->socket=oldsocket.socket;
	newsocket->ts=oldsocket.ts;
	return newsocket;
} 
// If needed, free memory allocated to element
void element_free(void ** element){
	free((socket_sensor_t* )*element);
}
// Compare two element elements; returns -1 if x<y, 0 if x==y, or 1 if x>y 
int element_compare(void * x, void * y){
	if()
	return 0;
}


