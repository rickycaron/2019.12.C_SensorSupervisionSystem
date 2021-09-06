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
tcpsock_t *master_socket;
  int master_sd=-1;
  tcpsock_t *new_socket;
  int max_client=20;
  tcpsock_t *client_socket[20];// the array of client pointers 
  sensor_data_t data;
  int bytes,result,sd,activity=0, max_sd=0;
  fd_set readfds;
  struct timeval timeout1; 
  FILE * fp_bin;

  fp_bin = fopen("sensor_data_recv", "wb");
  if ((fp_bin)==NULL) { 
     printf("Couldn't create sensor_data\n"); 
     exit(EXIT_FAILURE);}

   for(int i=0;i<max_client;i++){
    socket_set_sd(client_socket[i],-1);
   }
//create a new master socket   
 if (tcp_passive_open(&master_socket,server_port)!=TCP_NO_ERROR) exit(EXIT_FAILURE);
printf("master_socket is created, now waiting for connections!\n");

timeout1.tv_sec=TIMEOUT;
timeout1.tv_usec=0;
while(1){
  master_sd=socket_get_sd(master_socket);
//clear the socket set
  FD_ZERO(&readfds);
// add the master socket into readfds
  FD_SET(master_sd,&readfds);
  max_sd=master_sd;

//add the client socket to set
  for(int i=0;i<max_client;i++){
    //get the client sd
    sd=socket_get_sd(client_socket[i]);
    // if it is valid, add it to the readfds
    if(sd>0){FD_SET(sd,&readfds);}
    // update the max_client 
    if(sd>max_sd){max_sd=sd;}
  }

activity=select(max_sd+1,&readfds,NULL,NULL,&timeout1);

if((activity<0)&&(errno!=EINTR)){
  printf("Select() failed!Error!\n");
}

//If there is anything happened to the master socket, there is new client
result=FD_ISSET(master_sd,&readfds);
if(result){
  //there is new client
  if(tcp_wait_for_connection(master_socket,&new_socket)!=TCP_NO_ERROR) {exit(EXIT_FAILURE);}
printf("New connection, socket fd is %d, port is %d \n",socket_get_sd(new_socket),socket_get_port(new_socket));
// then add the new socket into the client array
for(int i=0;i<max_client;i++){
  if(socket_get_sd(client_socket[i])==0){
    client_socket[i]=new_socket;
    printf("Add this new socket in the client_array %d\n",i );
    break;
  }
}
}

//Else it is a new data sending form the older socket
for(int i=0;i<max_client;i++){
  sd=socket_get_sd(client_socket[i]);
  if(FD_ISSET(sd,&readfds)){
    //there is new data coming from this socket
    // read sensor ID
      bytes = sizeof(data.id);
      result = tcp_receive(client_socket[i],(void *)&data.id,&bytes);
      // read temperature
      bytes = sizeof(data.value);
      result = tcp_receive(client_socket[i],(void *)&data.value,&bytes);
      // read timestamp
      bytes = sizeof(data.ts);
      result = tcp_receive(client_socket[i], (void *)&data.ts,&bytes);
      if ((result==TCP_NO_ERROR) && bytes){
      //The data is received successfully
        printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value, (long int)data.ts);
      //do something with received data
      fwrite(&data.id, sizeof(data.id), 1, fp_bin);
      fwrite(&data.value, sizeof(data.value), 1, fp_bin);
      fwrite(&data.ts, sizeof(time_t), 1, fp_bin);      

      //refresh the time here  

      }
      else if (result==TCP_CONNECTION_CLOSED){
        //the connection was closed before the data is received
      printf("Peer has closed connection\n");
      if (tcp_close(&client_socket[i])!=TCP_NO_ERROR) {exit(EXIT_FAILURE);}
      printf("Client server %d is shutting down\n",i);
      }
      else{
      printf("Error occured on connection to peer\n");
      tcp_close(&client_socket[i]);
    }
   
  }// for the socket has somethinf to do, the if statement ends here

  //for all the socket, the time should be checked and free those sockets silent for too long
  if(sd>0){
    //check the time ts here
    //delete it if timeout
  }
}//for every socket in socket_client[max_client] 

//here check for the master socket, free it if the time is out


}//for the endless loop	


}

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


