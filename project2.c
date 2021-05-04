
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define NULL  __DARWIN_NULL
#define __DARWIN_NULL ((void *)0)
 /****************************************************************************** 
  pthread_sleep takes an integer number of seconds to pause the current thread 
  original by Yingwu Zhu
  updated by Muhammed Nufail Farooqi
  *****************************************************************************/
int pthread_sleep (int seconds)
{
   pthread_mutex_t mutex;
   pthread_cond_t conditionvar;
   pthread_t moderator;
   pthread_t commentator;
   struct timespec timetoexpire;
   if(pthread_mutex_init(&mutex,NULL))
    {
      return -1;
    }
   if(pthread_cond_init(&conditionvar,NULL))
    {
      return -1;
    }
   struct timeval tp;
   //When to expire is an absolute time, so get the current time and add //it to our delay time
   gettimeofday(&tp, NULL);
   timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;

   pthread_mutex_lock (&mutex);
   int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
   pthread_mutex_unlock (&mutex);
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&conditionvar);

   //Upon successful completion, a value of zero shall be returned
   return res;

}

void moderate() {
    printf("%s\n", "Let the discussion begin");
}
void commentate() {
    printf("%s\n", "Commentator #0 generates answer, position in queue: 0");
} 
int main(){
    while(1) {

        int pthread_create(moderator, NULL, void * (* moderate)(void *), NULL);
        int pthread_create(commentator, NULL, void * (* commentate)(void *), NULL);
        pthread_join(moderator, NULL);
        pthread_join(commentator, NULL);

    }
} 