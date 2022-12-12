#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

bool IsEmpty(BENSCHILLIBOWL *bcb);
bool IsFull(BENSCHILLIBOWL *bcb);
void AddOrderToBack(Order **orders, Order *order);
Order *RemoveOrderFromFront(Order **orders);

MenuItem BENSCHILLIBOWLMenu[] = {
    "BensChilli",       "BensHalfSmoke",  "BensHotDog", "BensChilliCheeseFries",
    "BensShake",        "BensHotCakes",   "BensCake",   "BensHamburger",
    "BensVeggieBurger", "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
  int random_idx_choice = random() % BENSCHILLIBOWLMenuLength;
  return BENSCHILLIBOWLMenu[random_idx_choice];
}

/* Allocate memory for the Restaurant, then create the mutex and condition
 * variables needed to instantiate the Restaurant */
BENSCHILLIBOWL *OpenRestaurant(int max_size, int expected_num_orders) {
  BENSCHILLIBOWL *bcb = malloc(sizeof(BENSCHILLIBOWL));
  bcb->orders = NULL; 
  bcb->current_size = 0;
  bcb->max_size = max_size;
  bcb->next_order_number = 0;
  bcb->orders_handled = 0;
  bcb->expected_num_orders = expected_num_orders;
  pthread_mutex_init(&(bcb->mutex), NULL);
  pthread_cond_init(&(bcb->can_add_orders), NULL);
  pthread_cond_init(&(bcb->can_get_orders), NULL);
  printf("Restaurant is open!\n");
  return bcb;
}

/* check that the number of orders received is equal to the number handled
 * (ie.fullfilled). Remember to deallocate your resources */
void CloseRestaurant(BENSCHILLIBOWL *bcb) {
  pthread_mutex_destroy(&(bcb->mutex));
  pthread_cond_destroy(&(bcb->can_add_orders));
  pthread_cond_destroy(&(bcb->can_get_orders));
  free(bcb);
  printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
// will be using the pthread_mutex_lock/unlock functions for acquire/release,
// pthread_cond_signal() for cond.signal(), pthread_cond_wait() for wait
int AddOrder(BENSCHILLIBOWL *bcb, Order *order) {
  pthread_mutex_lock(&(bcb->mutex)); 
  while (IsFull(bcb)) {
    pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
  }
  order->order_number =
      bcb->next_order_number; 
  AddOrderToBack(&(bcb->orders), order); 
  bcb->current_size += 1;                
  bcb->next_order_number += 1;           
  pthread_cond_signal(&(bcb->can_get_orders));
  pthread_mutex_unlock(&(bcb->mutex)); 
  return bcb->next_order_number;       

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL *bcb) {
  pthread_mutex_lock(&(bcb->mutex)); 
  while (IsEmpty(bcb)) {
    pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
  }

  Order *order =
      RemoveOrderFromFront(&(bcb->orders)); 

  bcb->orders_handled += 1; 
  bcb->current_size -= 1;   
  pthread_cond_signal(&(bcb->can_add_orders));
  pthread_mutex_unlock(&(bcb->mutex)); 
  return order;
}

// Optional helper functions (you can implement if you think they would be
// useful)
bool IsEmpty(BENSCHILLIBOWL *bcb) {
  if (bcb->current_size == 0) {
    return true;
  }
  return false;
}

bool IsFull(BENSCHILLIBOWL *bcb) {
  if (bcb->current_size == bcb->max_size) {
    return true;
  }
  return false;
}

/* this method adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (*orders != NULL) {
    Order *curr_order = *orders;
    while (curr_order->next) {
      curr_order = curr_order->next;
    }
    curr_order->next = order;
    order->next = NULL;
  } else {
    *orders = order;
  }
}

/* this method removes from the front of the queue */
Order *RemoveOrderFromFront(Order **orders) {
  if (*orders != NULL) {
    Order *to_go = *orders;
    *orders = (*orders)->next;
    return to_go;
  }
  return NULL;
}