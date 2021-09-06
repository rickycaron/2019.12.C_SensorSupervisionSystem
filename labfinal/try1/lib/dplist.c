#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list 

#ifdef DEBUG
	#define DEBUG_PRINTF(...) 									         \
		do {											         \
			fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	 \
			fprintf(stderr,__VA_ARGS__);								 \
			fflush(stderr);                                                                          \
                } while(0)
#else
	#define DEBUG_PRINTF(...) (void)0
#endif


#define DPLIST_ERR_HANDLER(condition,err_code)\
	do {						            \
            if ((condition)) DEBUG_PRINTF(#condition " failed\n");    \
            assert(!(condition));                                    \
        } while(0)

        
/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
  dplist_node_t * prev, * next;
  void * element;
};

struct dplist {
  dplist_node_t * head;
  void * (*element_copy)(void * src_element);			  
  void (*element_free)(void ** element);
  int (*element_compare)(void * x, void * y);
};


dplist_t * dpl_create (// callback functions
			  void * (*element_copy)(void * src_element),
			  void (*element_free)(void ** element),
			  int (*element_compare)(void * x, void * y)
			  )
{
  dplist_t * list;
  list = malloc(sizeof(struct dplist));
  DPLIST_ERR_HANDLER(list==NULL,DPLIST_MEMORY_ERROR);
  list->head = NULL;  
  list->element_copy = element_copy;
  list->element_free = element_free;
  list->element_compare = element_compare; 
  return list;
}

// Every list node of the list needs to be deleted (free memory)
// If free_element == true : call element_free() on the element of the list node to remove
// If free_element == false : don't call element_free() on the element of the list node to remove
// The list itself also needs to be deleted (free all memory)
// '*list' must be set to NULL.
// Extra error handling: use assert() to check if '*list' is not NULL at the start of the function.  
void dpl_free(dplist_t ** list, bool free_element)
{
  if((*list)->head==NULL){
    //even when the list is empty, you still need to free the dplist_t
    free(*list);
    *list=NULL;
    return;
  }
  assert((*list)->head!=NULL);
  dplist_node_t *list_node;
  dplist_node_t *tobefree;
  list_node=(*list)->head;
    if(free_element==true){

      while(list_node!=NULL){
        tobefree=list_node;
        list_node=list_node->next;
        if(tobefree->element!=NULL){
        (*list)->element_free(&(tobefree->element));
        }
        free(tobefree);
      }

    }else{
        while(list_node!=NULL){
          tobefree=list_node;
          list_node=list_node->next;
          free(tobefree);
      }
      }
      free(*list);
      *list=NULL;
}

// Inserts a new list node containing an 'element' in the list at position 'index' and returns a pointer to the new list.
// If insert_copy == true : use element_copy() to make a copy of 'element' and use the copy in the new list node
// If insert_copy == false : insert 'element' in the new list node without taking a copy of 'element' with element_copy() 
// Remark: the first list node has index 0.
// If 'index' is 0 or negative, the list node is inserted at the start of 'list'. 
// If 'index' is bigger than the number of elements in the list, the list node is inserted at the end of thelist.
dplist_t * dpl_insert_at_index(dplist_t * list, void * element, int index, bool insert_copy)
{
   DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
  dplist_node_t *ref_at_index, *list_node;
  list_node=malloc(sizeof(dplist_node_t));
  list_node->prev=NULL;
  list_node->next=NULL;
  if(insert_copy==true){  
    list_node->element=list->element_copy(element);
}
    else{
      list_node->element=element;
    }
    // the node has been created, now it should insert it into the list
    if(list->head==NULL){// the list is empty
      list->head=list_node;
      list_node->prev=NULL;
      list_node->next=NULL;
    }
    else if(index<=0){
      list_node->prev=NULL;
      list_node->next=list->head;
      list->head->prev=list_node;
      list->head=list_node;
    }
    else {
      ref_at_index=dpl_get_reference_at_index(list,index);
      if(index<dpl_size(list)){

        ref_at_index->prev->next=list_node;
        list_node->prev=ref_at_index->prev;
        list_node->next=ref_at_index;
        ref_at_index->prev=list_node;
      }
        else{
          assert(ref_at_index->next==NULL);
          ref_at_index->next=list_node;
          list_node->prev=ref_at_index;
          list_node->next=NULL;
        }
    }  
return list;
}
// Removes the list node at index 'index' from the list. 
// If free_element == true : call element_free() on the element of the list node to remove
// If free_element == false : don't call element_free() on the element of the list node to remove
// The list node itself should always be freed
// If 'index' is 0 or negative, the first list node is removed. 
// If 'index' is bigger than the number of elements in the list, the last list node is removed.
// If the list is empty, return the unmodifed list
dplist_t * dpl_remove_at_index( dplist_t * list, int index, bool free_element)
{
  if(list == NULL || list->head == NULL){return list;}
    dplist_node_t * todelete = dpl_get_reference_at_index(list, index);  // a pointer to the node to be deleted
    if(todelete == NULL){return list;}

    dplist_node_t* dummy1 = todelete->prev;
    dplist_node_t* dummy2= todelete->next;

    if(dummy1==NULL){
        // this is node 0
        if(dummy2!=NULL){
            dummy2->prev = NULL;
            list->head =dummy2;
        }else{
            list->head = NULL;
        }
    }
    else{//dummy1!=NULL, it is not at the begining
        dummy1->next = dummy2;
        if(dummy2!=NULL){// it is in the list
            dummy2->prev = dummy1;
        }
        else{//dummy2==NULL  
        }
    }
    
    if(free_element){
        list->element_free((void*)&(todelete->element));
        //todelete->element=NULL;
        // free(todelete);      // also free memory for the node
        // todelete=NULL;
        }
        free(todelete);      // also free memory for the node
        todelete=NULL;
    
    return list;
}
// Returns the number of elements in the list.

int dpl_size( dplist_t * list )
{
    int count=0;
    dplist_node_t *dummy=list->head;
    if(dummy==NULL){return 0;}
    count++;
    while(dummy->next!=NULL){
      count++;
      dummy=dummy->next;
    }
    return count;
}

// Returns the list element contained in the list node with index 'index' in the list.
// Remark: return is not returning a copy of the element with index 'index', i.e. 'element_copy()' is not used. 
// If 'index' is 0 or negative, the element of the first list node is returned. 
// If 'index' is bigger than the number of elements in the list, the element of the last list node is returned.
// If the list is empty, (void *)0 is returned.
void * dpl_get_element_at_index( dplist_t * list, int index )
{
  dplist_node_t * list_node;
  if(list->head==NULL){return NULL;}
  else{
    list_node=dpl_get_reference_at_index(list,index);
    return list_node->element;
  }
}
// Returns an index to the first list node in the list containing 'element'.
// Use 'element_compare()' to search 'element' in the list
// A match is found when 'element_compare()' returns 0
// If 'element' is not found in the list, -1 is returned.
int dpl_get_index_of_element( dplist_t * list, void * element )
{
   int count=0;
   dplist_node_t* list_node;
   if(list->head==NULL){return 0;}
   list_node=list->head;
   while(!((list->element_compare(list_node->element,element))==0||list_node->next==NULL)){
    //use a xor gate
    count++;
    list_node=list_node->next;
   }
   if(list->element_compare(list_node->element,element)==0){
    return count;// we find it
   }
   else{
    return -1;// didn't find it
   }

   
}

// Returns a reference to the list node with index 'index' in the list. 
// If 'index' is 0 or negative, a reference to the first list node is returned. 
// If 'index' is bigger than the number of list nodes in the list, a reference to the last list node is returned. 
// If the list is empty, NULL is returned.
dplist_node_t * dpl_get_reference_at_index( dplist_t * list, int index )
{
  if(list==NULL||list->head==NULL){return NULL;}
 dplist_node_t *list_node;
 list_node=list->head;
 int count=0;
 if(index<=0){return list->head;}
 else{
  int size=dpl_size(list);
  if(index<=size-1){
    while(count<index){
      count++;
      list_node=list_node->next;
    }
    return list_node;
  }else{
      list_node=list->head;
      while(list_node->next!=NULL){
        list_node=list_node->next;
      }
return list_node;
    }
 } 
}
 // Returns the element contained in the list node with reference 'reference' in the list. 
// If the list is empty, NULL is returned. 
// If 'reference' is NULL, the element of the last element is returned.
// If 'reference' is not an existing reference in the list, NULL is returned.
void * dpl_get_element_at_reference( dplist_t * list, dplist_node_t * reference )
{
    dplist_node_t *list_node=NULL;
    if(list->head==NULL){return NULL;}
    else if(reference==NULL){
      int lastind=dpl_size(list)-1;
      return dpl_get_reference_at_index(list,lastind)->element;
    }

    list_node=list->head;
    while( !((list->element_compare(list_node->element,reference->element))==0||list_node->next==NULL) ){
      list_node=list_node->next;
    }

   if(list->element_compare(list_node->element,reference->element)==0){
    return list_node->element;
   }
   else{
    return NULL;
   }
}



// HERE STARTS THE EXTRA SET OF OPERATORS //

// ---- list navigation operators ----//
  
dplist_node_t * dpl_get_first_reference( dplist_t * list )
{
    // add your code here
}

dplist_node_t * dpl_get_last_reference( dplist_t * list )
{
    // add your code here
}

dplist_node_t * dpl_get_next_reference( dplist_t * list, dplist_node_t * reference )
{
    // add your code here
}

dplist_node_t * dpl_get_previous_reference( dplist_t * list, dplist_node_t * reference )
{
    // add your code here
}

// ---- search & find operators ----//  
  
// void * dpl_get_element_at_reference( dplist_t * list, dplist_node_t * reference )
// {
//     // add your code here
// }

dplist_node_t * dpl_get_reference_of_element( dplist_t * list, void * element )
{
    // add your code here
}

int dpl_get_index_of_reference( dplist_t * list, dplist_node_t * reference )
{
    // add your code here
}
  
// ---- extra insert & remove operators ----//

dplist_t * dpl_insert_at_reference( dplist_t * list, void * element, dplist_node_t * reference, bool insert_copy )
{
    // add your code here
}

dplist_t * dpl_insert_sorted( dplist_t * list, void * element, bool insert_copy )
{
    // add your code here
}

dplist_t * dpl_remove_at_reference( dplist_t * list, dplist_node_t * reference, bool free_element )
{
    // add your code here
}

dplist_t * dpl_remove_element( dplist_t * list, void * element, bool free_element )
{
    // add your code here
}
  
// ---- you can add your extra operators here ----//
