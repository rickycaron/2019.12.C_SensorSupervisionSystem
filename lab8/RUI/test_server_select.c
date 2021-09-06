#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include "config.h"
#include "tcpsock.h"
#include "tcpsock.c"
//#include "dplist.h"

#ifndef TIMEOUT
  #error TIMEOUT not set
#endif
//#define PORT 5678
//#define MAX_CONN 3  // state the max. number of connections the server will handle before exiting

// below is the print function when the port isn't inputed correctly
void print_help(void);
//dplist_t* socket_list=NULL;
/* Implements a IO multiplexing test server with select( )(can handle many connections at the same time)
 */
int main(int argc, char *argv[]){
  tcpsock_t *master_socket;
  int master_sd=0;
  tcpsock_t *new_socket;
  tcpsock_t *client_socket[20];// the array of client pointers 
  int max_client=20;
  int max_sd=0;
  int sd;
  sensor_data_t data;
  int bytes,result;
  fd_set readfds;
  struct timeval timeout1;
  int activity=0;
  int server_port;
  FILE * fp_bin;
  fp_bin = fopen("sensor_data_recv", "wb");
  if ((fp_bin)==NULL) { 
     printf("Couldn't create sensor_data\n"); 
     exit(EXIT_FAILURE);}


  // create the server socket 
  if (argc != 2){
    print_help();
    exit(EXIT_SUCCESS);
  }else{
    server_port = atoi(argv[1]);
  }
   printf("Test server is started\n");
// intialize all the client sd with 0
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
  return 0;
}



void print_help(void)
{
  printf("Use this program with 1 command line options: \n");
//  printf("\t%-15s : a unique sensor node ID\n", "\'ID\'");
//  printf("\t%-15s : node sleep time (in sec) between two measurements\n","\'sleep time\'");
//  printf("\t%-15s : TCP server IP address\n", "\'server IP\'");
  printf("\t%-15s : TCP server port number\n", "\'server port\'");
}



