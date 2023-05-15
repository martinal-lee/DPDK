/**
* @File name: ConstValue.h
* @Author: Martinal
* @Version: -
* @Description: 定义运行时需要的常量参数
* */

#ifndef DPDKLEARNING_CONSTVALUE_H
#define DPDKLEARNING_CONSTVALUE_H

static unsigned char broadcast_mac[RTE_ETHER_ADDR_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff};
static unsigned char default_mac[RTE_ETHER_ADDR_LEN] = {0x0,0x0,0x0,0x0,0x0,0x0};

#define MAKE_IPV4_ADDR(a, b, c, d) ((a) + ((b)<<8) + ((c)<<16) + ((d)<<24))
static unsigned int localHostIP = MAKE_IPV4_ADDR(68, 87, 129, 223);

#define NUM_MBUF (4096-1)
#define BURST_BUF_SIZE 32

static unsigned short gDpdkPortID = 0;

#define RING_BUFFER_SIZE 1024
struct rx_tx_queue{
    struct rte_ring *rx_queue;/**接收环形缓冲区*/
    struct rte_ring *tx_queue;/**发送环形缓冲区*/
};
//arp回调函数所必须的参数
struct arp_table_control{
    struct rte_mempool* mbuf_pool;
    struct rx_tx_queue* ring_buffer;
};


#endif //DPDKLEARNING_CONSTVALUE_H
