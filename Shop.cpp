
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
   cout << "customer[" << person << "]: " << message << endl;
}
void Shop::printB(int person, string message) {
   cout << "barber[" << person << "]: " << message << endl;
}

int Shop::get_cust_drops() const
{
    return cust_drops_;
}

bool Shop::visitShop(int customer_id) 
{
   pthread_mutex_lock(&mutex_);
   
   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_) 
   {
      printC(customer_id,"leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return false;
   }
   
   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service
   int chair = findBarber(); //returns -1 if no open chairs
   //   if (chair  == -1 || !waiting_chairs_.empty()) 
   if (chair  == -1 || !waiting_chairs_.empty()) 
   {
      waiting_chairs_.push(customer_id);
      printC(customer_id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
      chair = findBarber();
      waiting_chairs_.pop();
   }
   //chair = findBarber();
   printC(customer_id, "moves to service chair [" + int2string(chair) + "]. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   customer_in_chair_[chair] = customer_id;
   in_service_ = true;

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&cond_barber_sleeping_);

   pthread_mutex_unlock(&mutex_); 
   return true;
}

void Shop::leaveShop(int customer_id) 
{
   pthread_mutex_lock( &mutex_ );

   int barber_id = askBarberID(customer_id);
   // Wait for service to be completed
   printC(customer_id, "wait for barber [" + int2string(barber_id) + "] to be done with haircut");
   while (in_service_ == true)
   {
      pthread_cond_wait(&cond_customer_served_, &mutex_);
   }
   
   // Pay the barber and signal barber appropriately
   money_paid_ = true;
   pthread_cond_signal(&cond_barber_paid_);
   printC(customer_id, "says good-bye to barber [" + int2string(barber_id) + "]" );
   pthread_mutex_unlock(&mutex_);
}

void Shop::helloCustomer(int barber_id) 
{
   pthread_mutex_lock(&mutex_);
   
   // If no customers than barber can sleep
   if (waiting_chairs_.empty() && customer_in_chair_[barber_id] == 0 ) 
   {
      printB(barber_id, "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }

   if (customer_in_chair_[barber_id] == 0)               // check if the customer, sit down.
   {
       pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }

   printB(barber_id, "starts a hair-cut service for [" + int2string( customer_in_chair_[barber_id] ) + "]" );
   pthread_mutex_unlock( &mutex_ );
}

void Shop::byeCustomer(int barber_id)
{
  pthread_mutex_lock(&mutex_);

  // Hair Cut-Service is done so signal customer and wait for payment
  in_service_ = false;
  printB(barber_id, "says he's done with a hair-cut service for [" + int2string(customer_in_chair_[barber_id]) + "]");
  money_paid_ = false;
  pthread_cond_signal(&cond_customer_served_);
  while (money_paid_ == false)
  {
      pthread_cond_wait(&cond_barber_paid_, &mutex_);
  }

  //Signal to customer to get next one
  customer_in_chair_[barber_id] = 0;
  printB(barber_id, "calls in another customer");
  pthread_cond_signal( &cond_customers_waiting_ );

  pthread_mutex_unlock( &mutex_ );  // unlock
}

//customer looks for a barber chair to sit down in. Returns an open chair index or -1 if no open chairs
int Shop::findBarber() {
   for (int i = 0; i < barbers; i++) {
      if (customer_in_chair_[i] == 0) {
         return i;
      }
   }
   return -1;
}
int Shop::askBarberID(int customerID) {
   for (int i = 0; i < barbers; i++) {
      if (customer_in_chair_[i] == customerID) {
         return i;//return index (barber's id)
      }
   }
   return -1; //Never gets used unless bug
}