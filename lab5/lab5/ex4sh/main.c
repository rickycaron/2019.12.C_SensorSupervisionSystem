#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"
typedef int element_t;

dplist_t * list = NULL;

void dpl_print( dplist_t * list );
void * element_copy(void * element);  // Duplicate 'element'; If needed allocated new memory for the duplicated element.
void element_free(void ** element); // If needed, free memory allocated to element
int element_compare(void * x, void * y);// Compare two element elements; returns -1 if x<y, 0 if x==y, or 1 if x>y 
    

int main(void)
{
  int * a,*b;
  int x;
  int index;
  a = malloc( sizeof(int) );
  assert(a != NULL);

  
  list = dpl_create(element_copy,element_free,element_compare);

  *a = 1;
  dpl_insert_at_index(list,a,-1,true);

  printf("we get here\n");
  //printf("we got here\n");// a way to debug
  *a = 2;
  dpl_insert_at_index(list,a,-1,false);

  *a = 3;
  dpl_insert_at_index(list,a,-1,false);

  b = malloc( sizeof(int) );
  assert(b != NULL);

  *b = 4;
  dpl_insert_at_index(list,b,10,false);  

  *b = 5;
  dpl_insert_at_index(list,b,10,false); 
  
  dpl_print(list);
  
  x = 3;
  index = dpl_get_index_of_element(list, &x);
  if (index == -1)
  {
    printf("element %d is not found\n", x);
  }
  else
  {
    printf("element %d found at index %d\n", x, index);
  }

  // remove elements from the list
  list = dpl_remove_at_index(list, -1,false);
  list = dpl_remove_at_index(list, 10,false);
  list = dpl_remove_at_index(list, -1,false);
  printf("we got here\n");// a way to debug
  dpl_free( &list,false);
  free(a); 
  free(b);
 
  
  // check Valgrind: after freeing the entire list, there is still a memory leak!!!
  
  return 0;
}


void dpl_print( dplist_t * list )
{
  element_t element;
  int i, length;

  length = dpl_size(list);
  for ( i = 0; i < length; i++)    
  {
    element = *((element_t*)dpl_get_element_at_index( list, i ));
    printf("index element %d = %d\n", i, element);
  }
}

// Duplicate 'element'; If needed allocated new memory for the duplicated element.
void * element_copy(void * element){
  void* copiedele;
  copiedele=malloc(sizeof(element_t));
  *((element_t*)copiedele)= *((element_t*)element);
  return copiedele;
} 


// If needed, free memory allocated to element
void element_free(void ** element){
  free (*((element_t**)element));

} 
// Compare two element elements; returns -1 if x<y, 0 if x==y, or 1 if x>y 

int element_compare(void * x, void * y){
  element_t result=0;
  int compareresult=0;
  result=*((element_t*)x)-*((element_t*)y);
  if(result==0){
    compareresult=0;
  }
  else if(result<0){
    compareresult=-1;
  }
  else if(result>0){
    compareresult=1;
  }
  return compareresult;

} 


