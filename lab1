#include <stdio.h>
#include <pthread.h>
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int ready = 0;
int num = 0;

void *provider(void *ptr){
    while(1){
        sleep(1);
        pthread_mutex_lock(&lock);
        if (ready == 1){
            pthread_mutex_unlock(&lock);
            continue;
        }
        ready = 1;
        num += 1;
        printf("provided event %d\n", num);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }
}

void *consumer(void *ptr){
    while(1){
        pthread_mutex_lock(&lock);
        while (ready == 0){
            pthread_cond_wait(&cond,&lock);
            printf("awoken\n");
        }
        ready = 0;
        printf("consumed event %d\n", num);
        pthread_mutex_unlock(&lock);
    }
}

main(){
    pthread_t prov, cons;
    pthread_create(&prov, NULL, provider, NULL);
    pthread_create(&cons, NULL, consumer, NULL);
    pthread_join(prov, NULL);
    pthread_join(cons, NULL);
    
    return 0;
}
