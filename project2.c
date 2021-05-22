
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define NULL  __DARWIN_NULL
#define __DARWIN_NULL ((void *)0)
#define MAXCOMMENTATORS 100
 /****************************************************************************** 
  pthread_sleep takes an integer number of seconds to pause the current thread 
  original by Yingwu Zhu
  updated by Muhammed Nufail Farooqi
  *****************************************************************************/

pthread_t moderator;
pthread_t commentator[MAXCOMMENTATORS];
pthread_mutex_t mutex;
pthread_cond_t conditionvar;
struct timespec timetoexpire;

int pthread_sleep (int seconds)
{
   /*pthread_mutex_t mutex;
   pthread_cond_t conditionvar;
   struct timespec timetoexpire;*/
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
int main(int argc, char *argv[]){

  //Commentator Count
  int N;

  if( argc == 2 ) {
      printf("The argument supplied is %s\n", argv[1]);
      sscanf(argv[1], "%d", &N);
  }
  else{
    printf("Error: Missing count of commentators!\n");
    return 1;
  }

  int t =0;
  while(1) {
    pthread_create(&moderator, NULL, moderate, NULL);
    for (t=0;t<N;t++){
      pthread_create(&commentator[t], NULL, commentate, NULL);
    }

    pthread_join(moderator, NULL);
    for (t=0;t<N;t++){
      pthread_join(commentator[t], NULL);
    }
  }

  return 0;
} 