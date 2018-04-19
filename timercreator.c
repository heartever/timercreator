// gcc timercreator.c -o timer -lrt

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>

#include<sys/socket.h>  
#include<sys/un.h>  
#define UNIX_DOMAIN "/home/ww31/userspacedelay/timercreator/UNIX.domain" // must be an existing directory

void SignHandler(int iSignNo);
void testTimerSign();
void logTime(const char *);
timer_t timer;

uint64_t rdtsc() {
    uint64_t ret = 0;
    __asm__ __volatile__(
                         "rdtscp"
                         : "=a" (ret)
                         :
                         : // "%rax"
                         );
    return ret;
}

const int NUMBER = 100;
uint64_t signaltime[100];
uint64_t begintime[100];
int count = 0;
#define W_LEN ((1ULL) << (22)) // maximum = 22 on my platform
char buf[W_LEN] = {0};

int fd;

int main() {
    int i;
    
    printf("TSC start: %lu \n", rdtsc());
    fd = open("output", O_WRONLY);
    if (fd == -1) {
        return EXIT_FAILURE;
    }
    testTimerSign();
    
   /* code related to the socket system call*/
    int connect_fd;  
    int ret;  
    char send_buff[W_LEN] = {0};  

    static struct sockaddr_un srv_addr;  
    // creat unix socket  
    connect_fd = socket(PF_UNIX, SOCK_STREAM, 0);  
    if(connect_fd < 0){  
      perror("cannot creat socket");  
      return -1;  
    }  
    srv_addr.sun_family=  AF_UNIX;  
    strcpy(srv_addr.sun_path, UNIX_DOMAIN);  
    //connect server  
    ret = connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));  
    if (ret < 0){  
      perror("cannot connect server");  
      close(connect_fd);  
      return -1;  
    }  
    memset(send_buff, 0, W_LEN);  
    strcpy(send_buff, "message from client");  
    //send info server  
    write(connect_fd, send_buff, sizeof(send_buff));  
    close(connect_fd);  
    /* code related to the socket system call end*/  
    
    for(i = 1; i < count; i++) {
        printf("Timestamp %d : %lu ~ %lu (delay: %lu) \n",
               i,
               signaltime[i-1],
               signaltime[i],
               signaltime[i] - signaltime[i-1]);
    }
    printf("TSC end: %lu \n", rdtsc());
    return 0;
}

void SignHandler(int iSignNo) {
    // logTime("sig recv");
 //   printf("TSC when entering sig handler: %lu, count: %d \n", rdtsc(), count);
    if(count < NUMBER)
    {
      signaltime[count++] = rdtsc();
    }
    if (iSignNo == SIGUSR1) {
        printf("Captured signal: SIGUSR1\n");
    } else if (iSignNo == SIGALRM) {
//        printf("Captured signal: SIGALRM\n");
    } else {
        printf("Captured signal: %d \n",iSignNo);
    }
}

void testTimerSign() {
    struct sigevent evp;
    struct itimerspec ts;
    
    int ret;
    evp.sigev_value.sival_ptr = &timer;
    evp.sigev_notify = SIGEV_SIGNAL;
    evp.sigev_signo = SIGALRM;
    signal(evp.sigev_signo, SignHandler);
    ret = timer_create(CLOCK_REALTIME, &evp, &timer); // use high resolution timer
    
    if(ret) {
        perror("timer_create");
    }

    uint64_t ts_set_begin, ts_set_end;
    ts_set_begin = rdtsc();
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 100000;              // set the interval of expirations
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 100000;                  // set the time of initial expiration
    ret = timer_settime(timer, 0, &ts, NULL);  // arm the timer
    // logTime("timer set");
    ts_set_end = rdtsc();
    printf("TSC after setting timer %lu \n", ts_set_end);

    if(ret) {
        perror("timer_settime");
    }
    // printf("delay: %lu \n", ts_set_end - ts_set_begin);
}

void logTime(const char * s) {
    struct timespec ts;
    timespec_get(& ts, TIME_UTC);
    printf("time @ %s: %lu.%ld \n", s, ts.tv_sec, ts.tv_nsec);
}
