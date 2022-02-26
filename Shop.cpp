
#include "Shop.h"

void Shop::init() 
{
   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);
   pthread_cond_init(&cond_customer_served_, NULL);
   pthread_cond_init(&cond_barber_paid_, NULL);
   pthread_cond_init(&cond_barber_sleeping_, NULL);
}

string Shop::int2string(int i) 
{
   stringstream out;
   out << i;
   return out.str( );
}

void Shop::printC(int person, string message)
{
   cout << ((person != barber) ? "customer[" : "barber  [" ) << person << "]: " << message << endl;
}
void Shop::printB(int person, string message) {
   cout << ((person != barber) ? "barber[" : "barber  [" ) << person << "]: " << message << endl;
}

int Shop::get_cust_drops() const
{
    return cust_drops_;
}

bool Shop::visitShop(int id) 
{
   pthread_mutex_lock(&mutex_);
   
   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_) 
   {
      printC( id,"leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return false;
   }
   
   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service
   if (customer_in_chair_ != 0 || !waiting_chairs_.empty()) 
   {
      waiting_chairs_.push(id);
      printC(id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
      waiting_chairs_.pop();
   }

   printC(id, "moves to a service chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   customer_in_chair_ = id;
   in_service_ = true;

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&cond_barber_sleeping_);

   pthread_mutex_unlock(&mutex_); 
   return true;
}

void Shop::leaveShop(int id) 
{
   pthread_mutex_lock( &mutex_ );

   // Wait for service to be completed
   printC(id, "wait for the hair-cut to be done");
   while (in_service_ == true)
   {
      pthread_cond_wait(&cond_customer_served_, &mutex_);
   }
   
   // Pay the barber and signal barber appropriately
   money_paid_ = true;
   pthread_cond_signal(&cond_barber_paid_);
   printC(id, "says good-bye to the barber." );
   pthread_mutex_unlock(&mutex_);
}

void Shop::helloCustomer(int id) 
{
   pthread_mutex_lock(&mutex_);
   
   // If no customers than barber can sleep
   if (waiting_chairs_.empty() && customer_in_chair_ == 0 ) 
   {
      printB(id, "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }

   if (customer_in_chair_ == 0)               // check if the customer, sit down.
   {
       pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }

   printB(id, "starts a hair-cut service for " + int2string( customer_in_chair_ ) );
   pthread_mutex_unlock( &mutex_ );
}

void Shop::byeCustomer(int id) 
{
  pthread_mutex_lock(&mutex_);

  // Hair Cut-Service is done so signal customer and wait for payment
  in_service_ = false;
  printB(id, "says he's done with a hair-cut service for " + int2string(customer_in_chair_));
  money_paid_ = false;
  pthread_cond_signal(&cond_customer_served_);
  while (money_paid_ == false)
  {
      pthread_cond_wait(&cond_barber_paid_, &mutex_);
  }

  //Signal to customer to get next one
  customer_in_chair_ = 0;
  printB(id, "calls in another customer");
  pthread_cond_signal( &cond_customers_waiting_ );

  pthread_mutex_unlock( &mutex_ );  // unlock
}
