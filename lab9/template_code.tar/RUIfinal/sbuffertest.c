#define _GNU_SOURCE 

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "errmacros.h"

//#include "connmgr.h"
#include "sbuffer.h"
#ifdef DEBUG
  #define DEBUG_PRINTF(condition,...)                 \
    do {                        \
       if((condition))                    \
       {                        \
      fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__); \
      fprintf(stderr,__VA_ARGS__);                \
       }                        \
    } while(0)            
#else
  #define DEBUG_PRINTF(...) (void)0
#endif

#define MAIN_ERR_HANDLER(condition,...)  \
  do {            \
    if ((condition))      \
    {         \
      DEBUG_PRINTF(1,"error condition \"" #condition "\" is true\n"); \
      __VA_ARGS__;        \
    }         \
  } while(0)
sbuffer_t* shared_buffer=NULL;  

pthread_mutex_t sensor_data_mutex; 

void* thread_function_connmgr(void *arg)
{
  // pthread_mutex_lock(&sensor_data_mutex);
  int res;
  sensor_data_t data1;
  data1.id=0;
  data1.value=2;
  data1.ts=time(NULL);
res=sbuffer_insert(shared_buffer, &data1);
sbuffer_print(&shared_buffer);
printf("1st print!\n");
  sensor_data_t data2;
  data2.id=1;
  data2.value=3;
  data2.ts=time(NULL);
res=sbuffer_insert(shared_buffer, &data2);
sbuffer_print(&shared_buffer);
 printf("2st print!\n");
  sensor_data_t data3;
  data3.id=2;
  data3.value=5;
  data3.ts=time(NULL);
res=sbuffer_insert(shared_buffer, &data3);
sbuffer_print(&shared_buffer);
printf("3st print!\n");
    sensor_data_t data4;
  data4.id=3;
  data4.value=4;
  data4.ts=time(NULL);
  res=sbuffer_insert(shared_buffer, &data4);
  sbuffer_print(&shared_buffer);
  printf("4st print!\n");
 printf("connmgr goes out\n");
 // pthread_mutex_unlock(&sensor_data_mutex);
   return NULL;
}
//this function fetch sensor data,
void* thread_function_datamgr(void *arg)
{

  int res;
      sensor_data_t  datagot;
      do{
         res=sbuffer_remove_in_datamgr(shared_buffer,&datagot);
       }while(res==1);
   perror("datamgr read some data!");
do{
    if(res==0||res==3){
      printf("We got here in datamgr!\n");
      sbuffer_print(&shared_buffer);
    }
     res=sbuffer_remove_in_datamgr(shared_buffer,&datagot);
}while(res!=1&&res!=-1);
  printf("now datamgr goes out\n");
  return NULL;
} 
// void* thread_function_datamgr_old_version(void *arg)
// {
//   // sleep(1);
//   // pthread_mutex_lock(&sensor_data_mutex);
//   int res;
//       sensor_data_t  datagot;
//       while(sbuffer_remove_in_datamgr(shared_buffer,&datagot)==1){}
//    //perror("datamgr read some data!");
// //sbuffer_print(&shared_buffer);
//  res=sbuffer_remove_in_datamgr(shared_buffer,&datagot);
//  // pthread_mutex_unlock(&sensor_data_mutex);

//   while(res!=1&&res!=-1){
//     // pthread_mutex_lock(&sensor_data_mutex);
//     if(res==0||res==3){
//       //printf("We got here in datamgr!\n");
//       //use this value
//       //sbuffer_print(&shared_buffer);
//     }
//     res=sbuffer_remove_in_datamgr(shared_buffer,&datagot);
//     // pthread_mutex_unlock(&sensor_data_mutex);
//   }
//   //printf("now datamgr goes out\n");
//   return NULL;
// }

//
void* thread_function_databasemgr(void *arg)
{
  int res;
      sensor_data_t  datagot;
      do{
         res=sbuffer_remove_in_databasemgr(shared_buffer,&datagot);
       }while(res==1);
   perror("databasemgr read some data!");
do{
    if(res==0||res==3){
      printf("We got here in databasemgr!\n");
      sbuffer_print(&shared_buffer);
    }
     res=sbuffer_remove_in_databasemgr(shared_buffer,&datagot);
}while(res!=1&&res!=-1);
  printf("now databasemgr goes out\n");
  return NULL;
}




int main(int argc, char *argv[])
{
  // pid_t child_pid;//this is for the log process creaion
  int result;//an interger for error handling
  
  // result=mkfifo(FIFO_NAME,0666);
  // CHECK_MKFIFO(result);
  // printf("FIFO creaion succeeds!\n");

  // printf("Start to fork!\n");
  // child_pid=fork();
  // SYSCALL_ERROR(child_pid);
  // if(child_pid==0){
  //   //this is the log process
  //   char* program="./log";
  //   char* arg_list[]={program,NULL};
  //   execvp(program,arg_list);
  //   //it should never come here 
  //   perror("\nAn error occured in execvp!\n");
  //   exit(EXIT_FAILURE);
  // }
  // else
  // { //below is for the main process
   //  int server_port;
   //  FILE *fp;//this is for the FIFO communication
   //  char *send_buf; //this is for the sending of log information
   //  fp = fopen(FIFO_NAME, "w");
   //  FILE_OPEN_ERROR(fp); 
   //  printf("main process synchronizing with log process ok\n");
   //  //get the server port number from the argument
   //  if (argc != 2){
   //    printf("Use this program with 1 command line options: \n");
   //    printf("\t%-15s : TCP server port number\n", "\'server port\'");
   //    exit(EXIT_SUCCESS);
   //  }else{
   //    server_port = atoi(argv[1]);
   //  } 
   //  printf("The server port number is %d\n",server_port);
    int presult;
    pthread_t thread_connmgr;
    pthread_t thread_datamgr;
    pthread_t thread_databasemgr;
   //  void *thread_result;
   //  // send_buf="Hello world!";
   //  // send_to_log(send_buf, fp); 

   // // create the server socket 
   if(sbuffer_init(&shared_buffer)!=0)
   {
      printf("Couldn't initialize the shared buffer!\n"); 
      exit(EXIT_FAILURE);
    }    
 
    result=pthread_mutex_init(&sensor_data_mutex,NULL);
    MAIN_ERR_HANDLER(result!=0);

    presult=pthread_create(&thread_connmgr, NULL,thread_function_connmgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);

    presult=pthread_create(&thread_datamgr, NULL,thread_function_datamgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);

    presult=pthread_create(&thread_databasemgr, NULL,thread_function_databasemgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);

    
    presult=pthread_join(thread_connmgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);

    presult=pthread_join(thread_datamgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);    

    presult=pthread_join(thread_databasemgr,NULL);
    MAIN_ERR_HANDLER(presult!=0);
   

   if(sbuffer_free(&shared_buffer)!=0)
   {
      printf("Couldn't free the shared buffer!\n"); 
      exit(EXIT_FAILURE);
   }   
     sbuffer_print(&shared_buffer);
     printf("We got here!\n");
    // result=fclose(fp);
    // FILE_CLOSE_ERROR(result);
    result=pthread_mutex_destroy(&sensor_data_mutex);
    MAIN_ERR_HANDLER(result!=0);
    
    exit(EXIT_SUCCESS);
  }

// }  
// 
// 
// 
// int send_to_log(char * send_buf, FILE *fp){
//   if(fputs(send_buf,fp)==EOF)
//   {
//     fprintf(stderr, "Error writing data to fifo\n");
//     exit(EXIT_FAILURE);
//   }
//   FFLUSH_ERROR(fflush(fp));
//   printf("Message from main: %s\n", send_buf);
//   return 0;
// }








// int main(int argc, char const *argv[])
// {
//   int server_port;
//    // create the server socket 
//   if (argc != 2){
//     printf("Use this program with 1 command line options: \n");
//     printf("\t%-15s : TCP server port number\n", "\'server port\'");
//     exit(EXIT_SUCCESS);
//   }else{
//     server_port = atoi(argv[1]);
//   }
//    connmgr_listen(server_port);
//    connmgr_free();
// // intialize all the client sd with 0
//   return 0;
// }