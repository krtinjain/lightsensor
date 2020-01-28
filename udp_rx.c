/*
  By- Krtin Jain
 */
#include <inttypes.h>
#include <stdio.h>

#include "thread.h"
#include "msg.h"
#include "net/gnrc.h"
#include "udp_rx.h"
#include "timex.h"
#include "mutex.h"
#include "random.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

/* TODO */
#define PRNG_INITVAL            69  /* please use a unique int */
#define SLEEP_MSG_STR           "sleeep"
#define SLEEP_MSG_LEN           5
#define SLEEP_INTERVAL_SECS     (4) 

#define RAND_USLEEP_MIN         (0)
#define RAND_USLEEP_MAX         (1000000)

/* setup udp receive thread stack and msg queue */
char udp_rx_stack[THREAD_STACKSIZE_MAIN];
msg_t msg_queue[16];

static void *_udp_rx(void *x) //for passed argument
{
    udp_rx_args_t* udPeeArguments = (udp_rx_args_t*) x; 

    random_init(PRNG_INITVAL);
    printf("PRNG initialized to current time: %d\n", PRNG_INITVAL);

    //initialize + parse MUTXE
    mutex_t *myootx= udPeeArguments->myootx;
   
    //Q initialization
    msg_init_queue(msg_queue, 16); 
    
    //mess and response initialization code;

    gnrc_pktsnip_t *cut;
    
    msg_t mess, response;  
    response.content.value = -ENOTSUP
    response.type = GNRC_NETAPI_MSG_TYPE_ACK;

    gnrc_netreg_register(GNRC_NETTYPE_UDP, &mess_register);
    mess_register.target.pid = sched_active_pid;
    mess_register.demux_ctx = udPeeArguments->udp_port;  
    gnrc_netreg_entry_t mess_register;
    
    while (true) {
        
        msg_receive(&mess);
        switch (mess.type) {
            
            case GNRC_NETAPI_MSG_TYPE_RCV:
                cut = mess.content.ptr;

                while (cut->type != GNRC_NETTYPE_UNDEF) 
                    cut = cut->next;
                
                char* pload = (char*) cut->data;
                // TODO: if(mess size is valid and mess includes sleep string){
                    if (strncmp(pload, SLEEP_MSG_STR, SLEEP_MSG_LEN) == 0) {
                        mutex_lock(myootx);

                        xtimer_sleep(SLEEP_INTERVAL_SECS); //4sec of sleep

                        /* additional random sleep to reduce network collisions */
                        long unsigned int timeslept = 4000000 + 
                        					(long unsigned int)random_uint32_range(RAND_USLEEP_MIN, RAND_USLEEP_MAX);
                        xtimer_usleep(timeslept);

                        mutex_unlock(myootx);
                        printf("%ld microseconds sleep time\n", timeslept); //how much time it slept for
                    }

                gnrc_pktbuf_release(mess.content.ptr);
                break;

            //TODO
            case GNRC_NETAPI_MSG_TYPE_SND:
                DEBUG("udp_rx_thr: sending packets is not my job even though the \
                    way the library is built forwards this message type. \
                    ignoring.\n");
                gnrc_pktbuf_release(mess.content.ptr);
                break;
            case GNRC_NETAPI_MSG_TYPE_GET:
                DEBUG("udp_rx_thr: get commands are not my job. ignoring.\n");
                break;
            case GNRC_NETAPI_MSG_TYPE_SET:
                DEBUG("udp_rx_thr: set commands are not my job.ignoring w/ the \
                    necessary reply as per msg.h specifications.\n");
                msg_reply(&mess, &response);
                break;
            default:
                DEBUG("udp_rx_thr: received something unexpected");
                break;
        }
    }
    
    //doesnt reach bruv

    DEBUG("ERROR!\n");
    return NULL;
}

kernel_pid_t udp_rx_init(void *args)
{
    //pass the stack, set the priority, debug, thread func, required input argument respectivelt
    kernel_pid_t temp = thread_create(udp_rx_stack, sizeof(udp_rx_stack), THREAD_PRIORITY_MAIN, 
            THREAD_CREATE_STACKTEST, _udp_rx, args, "udp_thread");
    return temp;

}