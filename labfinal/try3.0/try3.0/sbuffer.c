#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include "sbuffer.h"
static pthread_mutex_t sbuffer_lock;
static FILE * fp_in_sbuffer=NULL;
/*
 * All data that can be stored in the sbuffer should be encapsulated in a
 * structure, this structure can then also hold extra info needed for your implementation
 */
struct sbuffer_data {
    sensor_data_t data;
    int datamgr_toread;
    int databasemgr_toread;
};

typedef struct sbuffer_node {
  struct sbuffer_node * next;
  sbuffer_data_t element;
} sbuffer_node_t;

struct sbuffer {
  sbuffer_node_t * head;
  sbuffer_node_t * tail;
};  


int sbuffer_init(sbuffer_t ** buffer)
{
  pthread_mutex_init(&sbuffer_lock,NULL);
  pthread_mutex_lock(&sbuffer_lock);
  //here initialzi the fp
  fp_in_sbuffer = fopen(FIFO_NAME, "w");
  FILE_OPEN_ERROR(fp_in_sbuffer); 
  printf("fp_in_sbuffer is ready to write!\n");

  *buffer = malloc(sizeof(sbuffer_t));
  if (*buffer == NULL)
    {
      pthread_mutex_unlock(&sbuffer_lock);
      pthread_mutex_destroy(&sbuffer_lock);
       return SBUFFER_FAILURE;
    }
  (*buffer)->head = NULL;
  (*buffer)->tail = NULL;
  pthread_mutex_unlock(&sbuffer_lock);
  return SBUFFER_SUCCESS; 
}


int sbuffer_free(sbuffer_t ** buffer)
{  pthread_mutex_lock(&sbuffer_lock);
   //here close the fp_in_sbuffer
   int result=fclose(fp_in_sbuffer);
   FILE_CLOSE_ERROR(result);
   fp_in_sbuffer=NULL;

  if ((buffer==NULL) || (*buffer==NULL)) 
  { pthread_mutex_unlock(&sbuffer_lock);
    return SBUFFER_FAILURE;
  } 
 
  while ( (*buffer)->head )
  {
    sbuffer_node_t * dummy;
    dummy = (*buffer)->head;
    (*buffer)->head = (*buffer)->head->next;
    free(dummy);
  }
  free(*buffer);
  *buffer = NULL;
  pthread_mutex_unlock(&sbuffer_lock);
  pthread_mutex_destroy(&sbuffer_lock);
  return SBUFFER_SUCCESS;   
}


int sbuffer_insert(sbuffer_t * buffer, sensor_data_t * data)
{ pthread_mutex_lock(&sbuffer_lock);
  sbuffer_node_t * dummy;
  if (buffer == NULL){
    pthread_mutex_unlock(&sbuffer_lock);
    return SBUFFER_FAILURE;}
  dummy = malloc(sizeof(sbuffer_node_t));
  if (dummy == NULL){ 
     pthread_mutex_unlock(&sbuffer_lock);
     return SBUFFER_FAILURE;}
  dummy->element.data = *data;
  dummy->next = NULL;
  dummy->element.datamgr_toread=1;
  dummy->element.databasemgr_toread=1;
  if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL
  {
    buffer->head = buffer->tail = dummy;
  } 
  else // buffer not empty
  {
    buffer->tail->next = dummy;
    buffer->tail = buffer->tail->next; 
  }
  pthread_mutex_unlock(&sbuffer_lock);
  return SBUFFER_SUCCESS;
}


int sbuffer_remove_in_datamgr(sbuffer_t * buffer,sensor_data_t * data)
{ 
  pthread_mutex_lock(&sbuffer_lock);
  sbuffer_node_t * dummy;
  if (buffer == NULL){ 
      pthread_mutex_unlock(&sbuffer_lock);
      return SBUFFER_FAILURE;}//-1   
  if (buffer->head == NULL) { 
      pthread_mutex_unlock(&sbuffer_lock);
      return SBUFFER_NO_DATA; }//1
  int hasread=0;//if it has been read, it is 2
  dummy = buffer->head;
  if(dummy->element.datamgr_toread==1)
  {
    sensor_data_t thedata=buffer->head->element.data;
    //printf("We got here!\n");
    //here is very important, in your main program you have to create a sensor_data_t variable can you do this!
  *data = thedata;

  dummy->element.datamgr_toread=0;
  hasread=0;
  }else{
    hasread=2;
  }
  //check both value 0 and then remove it
  if(dummy->element.datamgr_toread==0&&dummy->element.databasemgr_toread==0)
  {
    if (buffer->head == buffer->tail) // buffer has only one node
    {
      buffer->head = buffer->tail = NULL; 
    }
    else  // buffer has many nodes empty
    {
      buffer->head = buffer->head->next;
    }
    free(dummy);  
      pthread_mutex_unlock(&sbuffer_lock);
    return (SBUFFER_SUCCESS+hasread);
    //0=0+0: removed,first time to read
    //2=0+2: removed, has already been read before
  }
  else
  {
     pthread_mutex_unlock(&sbuffer_lock);
    return (SBUFFER_DO_NOT_REMOVE_NOW+hasread);
    //3+0:do not remove, first time to read
    //3+2:do not remove, has already been read before
  }

}

int sbuffer_remove_in_databasemgr(sbuffer_t * buffer,sensor_data_t * data)
{ 
  pthread_mutex_lock(&sbuffer_lock);
  sbuffer_node_t * dummy;
  if (buffer == NULL) {
      pthread_mutex_unlock(&sbuffer_lock);
      return SBUFFER_FAILURE;}//-1   
  if (buffer->head == NULL) { 
      pthread_mutex_unlock(&sbuffer_lock);
      return SBUFFER_NO_DATA; }//1
  int hasread=0;//if it has been read, it is 2
  dummy = buffer->head;
  if(dummy->element.datamgr_toread==1){
     pthread_mutex_unlock(&sbuffer_lock);
     return SBUFFER_DATAMGR_HAS_NOT_READ;
  }
  if(dummy->element.databasemgr_toread==1)
  {
    sensor_data_t thedata=buffer->head->element.data;
    //printf("We got here!\n");
    //here is very important, in your main program you have to create a sensor_data_t variable can you do this!
  *data = thedata;
  dummy->element.databasemgr_toread=0;
  hasread=0;
  }else{
    hasread=2;
  }
  //check both value 0 and then remove it
  if(dummy->element.datamgr_toread==0&&dummy->element.databasemgr_toread==0)
  {
    if (buffer->head == buffer->tail) // buffer has only one node
    {
      buffer->head = buffer->tail = NULL; 
    }
    else  // buffer has many nodes empty
    {
      buffer->head = buffer->head->next;
    }
    free(dummy);  
      pthread_mutex_unlock(&sbuffer_lock);
    return (SBUFFER_SUCCESS+hasread);
    //0=0+0: removed,first time to read
    //2=0+2: removed, has already been read before
  }
  else
  {
     pthread_mutex_unlock(&sbuffer_lock);
    return (SBUFFER_DO_NOT_REMOVE_NOW+hasread);
    //3+0:do not remove, first time to read
    //3+2:do not remove, has already been read before
  }
 
}

void sbuffer_print(sbuffer_t ** buffer)
{
  pthread_mutex_lock(&sbuffer_lock);
  sbuffer_node_t * dummy;
  if ((buffer==NULL) || (*buffer==NULL)) 
  {
      pthread_mutex_unlock(&sbuffer_lock);
    return ;
  } 
  dummy = (*buffer)->head;
  while ( dummy!=NULL )
  {
    sensor_data_t thedata=dummy->element.data;
    int read1=dummy->element.datamgr_toread;
    int read2=dummy->element.databasemgr_toread;
    fprintf(stdout,"Sensor %"PRIu16" at %ld is %g C, datamgr %d, databasemgr %d \n",thedata.id,thedata.ts,thedata.value,read1,read2);   
    // printf("Sensor %"PRIu16" at %ld is %g C\n",thedata.id,thedata.ts,thedata.value);
    dummy =dummy->next;
  }
    fprintf(stdout, "*************************\n");
    fflush(stdout);
  pthread_mutex_unlock(&sbuffer_lock);  
}

void send_to_log(char * send_buf){
  pthread_mutex_lock(&sbuffer_lock);
  if(fp_in_sbuffer==NULL){
    printf("fp_in_sbuffer hasn't been intialized\n");
    pthread_mutex_unlock(&sbuffer_lock);
    return ;
  }
  if(fputs(send_buf,fp_in_sbuffer)==EOF)
  {
    fprintf(stderr, "Error writing data to fifo\n");
    exit(EXIT_FAILURE);
  }
  //fputs("\n",fp_in_sbuffer);
  FFLUSH_ERROR(fflush(fp_in_sbuffer));
  printf("Message send from sbuffer: %s\n", send_buf);
  pthread_mutex_unlock(&sbuffer_lock);
  return ;
}
