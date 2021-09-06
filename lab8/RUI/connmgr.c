#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include "config.h"
#include "dplist.h"
#include "tcpsock.h"

#include "connmgr.h"

#ifndef TIMEOUT
  #error TIMEOUT not set
#endif
typedef struct socket_sensor{
	tcpsock_t* socket;
	time_t	  ts;
} socket_sensor_t;
//typedef struct socket_sensor socket_sensor_t;
void *element_copy(void * element);  // Duplicate 'element'; If needed allocated new memory for the duplicated element.
void element_free(void ** element); // If needed, free memory allocated to element
int element_compare(void * x, void * y);// Compare two element elements; returns -1 if x<y, 0 if x==y, or 1 if x>y 

dplist_t * socket_list=NULL;


/*This methods hold the core fuctionality of your connmgr. It starts listening on the given port and 
* when a sensor node connects, it writes the data to a sensor_data_recv file. This file must have the 
* same formet as  the sensor data file in assignment 6 and 7
*/
void connmgr_listen(int port_number){
	tcpsock_t *master_socket;
	tcpsock_t *new_socket;
	tcpsock_t *read_socket;
  	int master_sd=-1;
  	time_t master_ts=0;
	sensor_data_t data;
  	int bytes,result,sd,activity=0, max_sd=0;
  	int client_size=0;
  	int waiting_for_client=0;
  	fd_set readfds;
  	struct timeval timeout1; 
	FILE * fp_bin;
//create the dplist
	socket_list=dpl_create(element_copy,element_free,element_compare);
	printf("Socket_list is created!\n");
//open the file for wrting
 	fp_bin = fopen("sensor_data_recv", "wb");
  	if ((fp_bin)==NULL) { 
     	printf("Couldn't create sensor_data\n"); 
    	exit(EXIT_FAILURE);}
//create a new master socket   
 if (tcp_passive_open(&master_socket, port_number)!=TCP_NO_ERROR) exit(EXIT_FAILURE);
printf("master_socket is created, now waiting for connections!\n");
timeout1.tv_sec=TIMEOUT;
timeout1.tv_usec=0;
master_ts=time(NULL);

while(1){
  master_sd=socket_get_sd(master_socket);
	//clear the socket set
  FD_ZERO(&readfds);
	// add the master socket into readfds
  FD_SET(master_sd,&readfds);
  max_sd=master_sd;

  client_size=dpl_size(socket_list);
  for(int i=0;i<client_size;i++){
  	sd=socket_get_sd(dpl_get_tcpsock(socket_list,i));
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
	master_ts=time(NULL);
  //there is new client
  if(tcp_wait_for_connection(master_socket,&new_socket)!=TCP_NO_ERROR) {exit(EXIT_FAILURE);}
printf("New connection, socket fd is %d, port is %d \n",socket_get_sd(new_socket),socket_get_port(new_socket));
// then add the new socket into the client array
result=add_new_client_to_dpl(socket_list,new_socket);
}// there is new connection

//Else it is a new data sending form the older socket
for(int i=0;i<client_size;i++){
	int myindex=i;
	sd=socket_get_sd(dpl_get_tcpsock(socket_list,i));
	if(FD_ISSET(sd,&readfds)){
		read_socket=dpl_get_tcpsock(socket_list,myindex);
		if(read_socket==NULL){ printf("Fail to get the read_socket!\n");
			exit(EXIT_FAILURE);}
    //there is new data coming from this socket
    // read sensor ID
			updatetime(socket_list, myindex);
	  	bytes = sizeof(data.id);
      	result = tcp_receive(read_socket,(void *)&data.id,&bytes);
    // read temperature
      	bytes = sizeof(data.value);
     	result = tcp_receive(read_socket,(void *)&data.value,&bytes);
    // read timestamp
      	bytes = sizeof(data.ts);
      	result = tcp_receive(read_socket, (void *)&data.ts,&bytes);
        	if ((result==TCP_NO_ERROR) && bytes){
      //The data is received successfully  	
        	printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value, (long int)data.ts);
      //do something with received data
      		fwrite(&data.id, sizeof(data.id), 1, fp_bin);
      		fwrite(&data.value, sizeof(data.value), 1, fp_bin);
      		fwrite(&data.ts, sizeof(time_t), 1, fp_bin);     
      }
        else if (result==TCP_CONNECTION_CLOSED){
        //the connection was closed before the data is received
      printf("Peer has closed connection\n");
      if(remove_sensor_node(socket_list, myindex)){
      	printf("Sensor socket %d is remove successfully!\n",myindex);
      }
      }else{
      printf("Error occured on connection to peer\n");
      remove_sensor_node(socket_list, myindex);
    }

}//for this socket has something to read

}// for the socket has something to do, the if statement ends here

//after all of this, begin to delete the socket over time
 socket_list=checkallclients(socket_list);
 client_size=dpl_size(socket_list);
if(client_size==0&&waiting_for_client==0){
	waiting_for_client=1;
	master_ts=time(NULL);
}else if(client_size>0){
	waiting_for_client=0;
}

 time_t time_now=time(NULL);
 if(client_size==0&&waiting_for_client==1&&time_now-master_ts>TIMEOUT){
 		if(tcp_close(&master_socket)!=0){
 			printf("Fail to close the master_socket!\n");
 			exit(EXIT_FAILURE);
 		}
 		printf("Master socket has been clodes now!\n");
 		if(fclose(fp_bin)==0){"Succeed in closing the FILE pointer!";}
 		break;
 }



}//endless while loop
printf("Congratulations! Everything is done now!\n");
}

/*This method shoud be called to clean up the connmgr, and to free all used memory.
* After this no new connection will be accepted
*/
void connmgr_free(){
	dpl_free(&socket_list,1);
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
	socket_sensor_t* newsocket=(socket_sensor_t*)malloc(sizeof(socket_sensor_t));
	newsocket->socket= tcp_sock_copy((tcpsock_t*)element);
	newsocket->ts=((socket_sensor_t*)element)->ts;
	return newsocket;
	
} 
// If needed, free memory allocated to element
void element_free(void ** element){
	tcp_sock_free (((socket_sensor_t*)(*element))->socket);
	free((socket_sensor_t* )*element);
}
// Compare two element elements; returns 1 if x==y, 0 if x!=y
int element_compare(void * x, void * y){
	if(tcp_sock_compare(x,y)){
		return 1;
	}
	else{
	return 0;}
}

tcpsock_t* get_socket(socket_sensor_t *s){
	if(s==NULL){
		printf("NULL pointer in get_socket!\n");
		exit(EXIT_FAILURE);
	}
	return (s->socket);
}

tcpsock_t * dpl_get_tcpsock(dplist_t * list,int index){
	socket_sensor_t* s=(socket_sensor_t*)dpl_get_element_at_index(list,index);
	if(s==NULL){
		printf("NULL pointer in dpl_get_tcpsock!\n");
		exit(EXIT_FAILURE);
	}
	return s->socket;
}

time_t dpl_get_ts(dplist_t * list,int index){
	socket_sensor_t* s=(socket_sensor_t*)dpl_get_element_at_index(list,index);
	return s->ts;
}

int add_new_client_to_dpl(dplist_t *list,tcpsock_t * sock){
	if(list==NULL||sock==NULL){
		printf("Insert client failed!\n");
		exit(EXIT_FAILURE);}
	socket_sensor_t* client=(socket_sensor_t * )malloc(sizeof(socket_sensor_t));
	client->socket=sock;
	client->ts=time(NULL);
	int size=dpl_size(list);
	list=dpl_insert_at_index(list,client,size-1,0);
	printf("Succeed inserting client %d into sensor_list\n",size-1);
	return 1;
}
void updatetime(dplist_t *list, int index){
	socket_sensor_t* s=(socket_sensor_t*)dpl_get_element_at_index(list,index);
	if(s==NULL){
		printf("NULL pointer in updatetime!\n");
		exit(EXIT_FAILURE);
	}
	s->ts=time(NULL);
}
//delete everything of a dplist_node in the dplist
dplist_t * remove_sensor_node(dplist_t *list, int index){
	printf("Now remove the node at %d\n", index);
	socket_sensor_t* s=(socket_sensor_t*)dpl_get_element_at_index(list,index);
	 if (tcp_close(&(s->socket))!=TCP_NO_ERROR) {exit(EXIT_FAILURE);}
     printf("Client server %d is shutting down\n",index);
     free(s);
     list=dpl_remove_at_index(list,index,0);
     printf("Sensor socket %d remove finished!\n",index);
     return list;
}

dplist_t * checkallclients(dplist_t *list){
	time_t timenow=time(NULL);
	time_t client_time=0;
	int remove_result;
	int client_size=dpl_size(list);
	for(int x=0;x<client_size;x++){
		client_time=dpl_get_ts(list,x);
		if(timenow-client_time>TIMEOUT){
			printf("Sensor %d has expired the timeout\n",x );
			//this sensor has exceeds longer than timeout, delete it
			list=remove_sensor_node(list,x);
			if(remove_result){
				printf("Sensor %d has been removed from dplist\n", x);
			}else{
				exit(EXIT_FAILURE);
			}
		}
	}
	return list;
}
