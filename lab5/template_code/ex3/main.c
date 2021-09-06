#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

dplist_t * list = NULL;

void dpl_print( dplist_t * list );

int main(void)
{
  int * a;
  int x;
  int index;
  a = malloc( sizeof(int) );
  assert(a != NULL);
  
  list = dpl_create();

  *a = 1;
  dpl_insert_at_index(list,a,-1);

  *a = 2;
  dpl_insert_at_index(list,a,-1);

  *a = 3;
  dpl_insert_at_index(list,a,-1);

  a = malloc( sizeof(int) );
  assert(a != NULL);

  *a = 4;
  dpl_insert_at_index(list,a,10);  

  *a = 5;
  dpl_insert_at_index(list,a,10); 
  
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
  list = dpl_remove_at_index(list, -1);
  list = dpl_remove_at_index(list, 10);
  list = dpl_remove_at_index(list, -1);
  
  dpl_free( &list );
  free(a); 
  
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
    element = dpl_get_element_at_index( list, i );
    printf("index element %d = %d\n", i, element);
  }
}


