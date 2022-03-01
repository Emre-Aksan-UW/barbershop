#ifndef SHOP_ORG_H_
#define SHOP_ORG_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
// Emre Aksan
// P4 Synchronization
// Shop h

//Shop class snychronizes barber and customer threads,
// has the methods of customer and barber threads.

#include <vector>
using namespace std;

#define kDefaultNumChairs 3
#define kDefaultnumBarbers 1

class Shop
{
public:
   Shop(int num_chairs, int num_barbers) : max_waiting_cust_((num_chairs > 0 ) ? num_chairs : kDefaultNumChairs), barbers(num_barbers), cust_drops_(0)
   {
      init();
   };
   Shop() : max_waiting_cust_(kDefaultNumChairs), barbers(kDefaultnumBarbers), cust_drops_(0)
   { 
      init(); 
   };

   bool visitShop(int id);   // return true only when a customer got a service
   void leaveShop(int id);
   void helloCustomer(int barber_id);
   void byeCustomer(int barber_id);
   int get_cust_drops() const;
   int findBarber();
   int askBarberID(int customerID);

 private:
   const int max_waiting_cust_;              // the max number of threads that can wait
   const int barbers;                 //number of barbers
   vector<int> customer_in_chair_ = vector<int>(barbers,0); //vector of barbers chairs, index id of barber, int is customer id. id 0 means empty chair.
   vector<bool> in_service_ = vector<bool>(barbers, false);            
   vector<bool> money_paid_ = vector<bool>(barbers, false);
   queue<int> waiting_chairs_;  // includes the ids of all waiting threads
   int cust_drops_;

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;
   pthread_cond_t  cond_customers_waiting_;
   vector <pthread_cond_t>  cond_customer_served_ = vector <pthread_cond_t>(barbers);
   vector <pthread_cond_t> cond_barber_paid_ = vector <pthread_cond_t>(barbers);
   vector <pthread_cond_t> cond_barber_sleeping_ = vector <pthread_cond_t>(barbers);

   void init();
   string int2string(int i);
   void printC(int person, string message);
   void printB(int person, string message);
};
#endif
