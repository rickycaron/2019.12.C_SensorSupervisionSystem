#include <stdio.h>
#include <stdlib.h>
#include "connmgr.h"
#include "snesor_db.h"
int main(int argc, char const *argv[])
{
  int server_port;
   // create the server socket 
  if (argc != 2){
    printf("Use this program with 1 command line options: \n");
    printf("\t%-15s : TCP server port number\n", "\'server port\'");
    exit(EXIT_SUCCESS);
  }else{
    server_port = atoi(argv[1]);
  }
   connmgr_listen(server_port);
   connmgr_free();
// intialize all the client sd with 0
  return 0;
}