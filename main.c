/*
  By- Krtin Jain
 */

#include <stdio.h>

#include "msg.h"
#include "xtimer.h"
#include "udp_rx.h"
#include "net/ipv6/addr.h"
#include "net/gnrc.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/udp.h"
#include "periph/adc.h"
#include "mutex.h"
#include "random.h"
 
#define ENABLE_DEBUG (1)
#include "debug.h"

#define MAIN_QUEUE_SIZE     (8)
#define TX_POWER            (7)
#define UDP_SEND_PORT_STR   "8808"  //TODO
#define UDP_RX_PORT          8850      //TODO

/* Only line 5 can be used because only PA pins are able to be used as ADC as
   per the CC2538's user guide (the current RIOT code is incorrect!) */


#define ADC_LINE_SELECT     (5)   //do not modify
#define ADC_RES             ADC_RES_12BIT

#define RELAY_NODE_ADDR     "fe80::212::4b00::18e0::ba41" //TODO

#define SENSE_INTERVAL_SEC  (2)

static msg_t main_msg_queue[MAIN_QUEUE_SIZE];

/**
 * @brief Sends a UDP packet
 *
 * @param[in] addr_str      Destination IPv6 address as a string.
 * @param[in] port_str      Destination port as a string.
 * @param[in] data          Pointer to data. Must be a null terminated string 
 *                          but will not send the null character.
 */

static void udp_send(char *addr_str, char *port_str, char *data)
{
    
    int iface;
    uint16_t loc; //loc of port
    ipv6_addr_t address; 

    //get interface
    iface = ipv6_addr_split_iface(addr_str);
    if (iface < 0)
    {
    	if(gnrc_netif_numof() == 1)
    	{
        	iface = gnrc_netif_iter(NULL)->pid;
    	}
    }
    
    //read in dest. addr 
    //same as below- try using a varibale here
    bool temp = 
    	(ipv6_addr_from_str(&address, addr_str) == NULL);
    if (temp) 
    {
        puts("Error:unable to parse destination address");
        return;
    }

    // read in port loc
    uint16_t buf = atoi(port_str);
    if (buf == 0) 
    {
        puts("Error:unable to parse destination port");
        return;
    }
    loc = buf;

    gnrc_pktsnip_t *pload; //to store payoad
    unsigned pload_size; //payload size
    gnrc_pktsnip_t *udPee, *iPee; //UDP and IP 
    
    // allocate pload
   	pload = gnrc_pktbuf_add(NULL, data, strlen(data), GNRC_NETTYPE_UNDEF);
    if (pload == NULL) 
    {
        puts("Error:unable to copy data to packet buffer");
        return;
    }
    pload_size = (unsigned)pload->size;
    
    //allo header and set port
    udPee = gnrc_udp_hdr_build(pload, loc, loc);
    if (udPee == NULL) 
    {
        puts("Error:unable to allocate UDP header");
        gnrc_pktbuf_release(pload);
        return;
    }
    
    //ipv6 head
    iPee = gnrc_ipv6_hdr_build(udPee, NULL, &address);
    if (iPee == NULL) 
    {
        puts("Error:unable to allocate IPv6 header");
        gnrc_pktbuf_release(udPee);
        return;
    }
    
	//netif header
    if (iface > 0) 
    {
        gnrc_pktsnip_t *temp = gnrc_netif_hdr_build(NULL, 0, NULL, 0);
        kernel_pid_t holder = (kernel_pid_t)iface; 
        ( (gnrc_netif_hdr_t*)temp->data ) -> if_pid = holder;
     	LL_PREPEND(iPee, temp);
    }
    
    //send it bruv
    //try using a varibale for bool i'm getting a weird error
    bool temp2 = gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, iPee);
    if (!temp2) 
    {
        puts("Error: unable to locate UDP thread");
        gnrc_pktbuf_release(iPee);
        return;
    }

}


int main(void)
{
    int samp = 0;

    msg_init_queue(main_msg_queue, MAIN_QUEUE_SIZE);

    mutex_t myoot;
    mutex_init(&myoot);
  
    puts("Welcome to Lab11 ! ");

    gnrc_netif_t *netif = NULL;
    int16_t tPower = TX_POWER;

    ///////////////begin while loop here: @kj

    while ((netif = gnrc_netif_iter(netif))) {
        
        ipv6_addr_t ipv6_addrs[GNRC_NETIF_IPV6_ADDRS_NUMOF];
        
        int ans = gnrc_netapi_get(netif->pid, NETOPT_IPV6_ADDR, 0, ipv6_addrs, sizeof(ipv6_addrs));
        if (ans < 0) {continue;}
        
        unsigned k = (unsigned)(ans / sizeof(ipv6_addr_t))
        unsigned j = 0;

        while (j < k){
            char ipv6_addr[IPV6_ADDR_MAX_STR_LEN];
            ipv6_addr_to_str(ipv6_addr, &ipv6_addrs[j], IPV6_ADDR_MAX_STR_LEN);
            
            //string stringy = "My address is %s\n";
            printf("My address is %s\n", ipv6_addr);

            /* For EE 250L, we only use devices with one netif, se it's safe to set the power here */
            gnrc_netapi_set(netif->pid, NETOPT_TX_POWER, 0, &tPower, sizeof(tPower));
            gnrc_netapi_get(netif->pid, NETOPT_TX_POWER, 0, &tPower, sizeof(tPower));
            
            //string stringx = "Power level is now %d\n";
            printf("Power level is now %d\n", tPower);
            j++;
        }
    }
    
    udp_rx_args_t argument;
    argument.udp_port = UDP_RX_PORT;
    argument.mute = &myoot;
   

    udp_rx_init(&argument); 
    
#ifdef OPENMOTE_BUILD    

    if (adc_init(ADC_LINE(ADC_LINE_SELECT)) >= 0) 
    {
        printf("Successfully initialized ADC_LINE(%u)\n", ADC_LINE_SELECT);
    }
    else 
    {
        printf("Initialization of ADC_LINE(%u) failed\n", ADC_LINE_SELECT);
        return 1;
    }

#endif
    
    while(true)
    {
        //TODO: handle mutex, format payload, send udp pkt, set sample interval
        xtimer_sleep(SENSE_INTERVAL_SEC);
        mutex_lock(&myoot);

#ifdef OPENMOTE_BUILD

        int temp3 = adc_sample(ADC_LINE(ADC_LINE_SELECT), ADC_RES);
        if (temp3 >= 0) 
        {
          printf("ADC_LINE(%u): %i\n", ADC_LINE_SELECT, samp);
        }
        
        else 
        {
          printf("ADC_LINE(%u): Error with line and resolution selection\n", ADC_LINE_SELECT);
        }

#else
        samp = 1024;
        printf("fake ADC value for native: %d\n", 1024);  //@vn: you can pass number directly

#endif
        char arrbar[50];
        
        //@kj -using samp here so you need the value
        snprintf(arrbar, 50, "rpi-krtinjain/light: %d", samp); //
        //string stringz = "String sent: %s\n";
        printf("String sent: %s\n", arrbar);

        udp_send(RELAY_NODE_ADDR, UDP_SEND_PORT_STR, arrbar);

        mutex_unlock(&myoot);
    }

    // never reached burv

    DEBUG("ERROR!\n");
    
    return 1; 
}
