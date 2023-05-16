/**
* @File name: ARPTimer.h
* @Author: Martinal
* @Version: -
* @Description: 定时发送arp request以及定时显示arp表
* */

#ifndef DPDKLEARNING_ARPTIMER_H
#define DPDKLEARNING_ARPTIMER_H
#include <rte_timer.h>
#include "SendFile.h"
#include "ConstValue.h"
#include "ARPTable.h"
#include "PublicTools.h"
//头插法双链表
#define ADD_ENTRY(arp_table_first_entry, arp_entry) do{ \
     (arp_entry)->next = arp_table_first_entry;                 \
     (arp_entry)->prev = NULL;                                 \
     if((arp_table_first_entry)!=NULL){                        \
        (arp_table_first_entry)->prev = arp_entry;                                          \
     }                                                       \
     (arp_table_first_entry) = arp_entry;                     \
                                                           \
}while(0);

#define REMOVE_ENTRY(arp_table_first_entry,arp_entry) do{ \
    if ((arp_table_first_entry) == NULL) break;                   \
    if ((arp_entry)->prev != NULL) (arp_entry)->prev->next = (arp_entry)->next; \
    if ((arp_entry)->next != NULL) (arp_entry)->next->prev = (arp_entry)->prev; \
    if ((arp_entry) == (arp_table_first_entry)) (arp_table_first_entry)->next = (arp_entry)->next; \
    (arp_entry)->prev = (arp_entry)->next = NULL;                   \
}while(0);

#define TIMER_RESOLUTION_CYCLES 26880000000ULL // 10s
//#define TIMER_RESOLUTION_CYCLES 161280000000ULL // 60s

/**
 * @brief arp timer回调函数，定时发送arp请求数据包
 * @param timer 定时器
 * @param arg   发送缓冲池，用于申请发送内存
 * @return
 *     -<em>None </em>
 */
void
arp_request_timer_cb(__attribute__((unused)) struct rte_timer *timer,void *arg){
    struct rte_mempool* mbuf_pool = (struct rte_mempool*)arg;
    //struct arp_table_control* table_control = (struct arp_table_control*)arg;

    unsigned int i = 0;
    for (i = 1;i <= 254;i ++) {
        //uint32_t dstip = (localHostIP & 0xFFFFFF00) | (0xFFFFFF00 & (i << 24));
        uint32_t dstip = ((localHostIP & 0xFFFFFF00) | i);
        struct in_addr addr;
        addr.s_addr = dstip;
        //printf("arp ---> src: %s \n", inet_ntoa(addr));

        struct rte_mbuf *arp_send_data_mbuf = NULL;
        uint8_t *dstmac = arp_find_dst_mac_addr(ntohl(dstip));

        if (dstmac == NULL) {
            arp_send_data_mbuf = ARPSend(mbuf_pool, gDpdkPortID,broadcast_mac,default_mac, localHostIP, dstip, RTE_ARP_OP_REQUEST);

        } else {
            continue;
            //arp_send_data_mbuf = ARPSend(mbuf_pool, gDpdkPortID,dstmac,dstmac, localHostIP, dstip, RTE_ARP_OP_REQUEST);
        }

        //没有走发送队列，直接送到网卡去了
        rte_eth_tx_burst(gDpdkPortID, 0, &arp_send_data_mbuf, 1);
        rte_pktmbuf_free(arp_send_data_mbuf);
    }

}

/**
 * @brief 打印arp表的回调函数
 * @param timer 定时器
 * @param arg   接收arp表
 * @return
 *     -<em>None </em>
 */
void
arp_table_show_timer_cb(__attribute__((unused)) struct rte_timer *timer,void *arg){
    struct arp_table* const_arp_table = (struct arp_table* )arg;
    //打印arp表
    struct arp_entry *iter;
    printf("------------------arp table:----------------- \n");
    for (iter = const_arp_table->entry; iter != NULL; iter = iter->next) {
        struct in_addr addr;
        addr.s_addr = ntohl(iter->ip);

        printf("ip: %s ", inet_ntoa(addr));
        print_ethaddr(" mac: ", (struct rte_ether_addr *) iter->mac);
        printf("status: %d \n", iter->status);

    }
    printf("------------------arp table:----------------- \n");


}

/** @deprecated*/
/**
 * @brief arp timer初始化
 * @param mbuf_pool 发送缓冲池，用于申请发送内存
 * @return
 *     -<em>None </em>
 */
void
ArpTimerInit(struct rte_mempool *mbuf_pool){
    rte_timer_subsystem_init();
    struct rte_timer arp_timer;
    rte_timer_init(&arp_timer);

    unsigned int hz = rte_get_timer_hz();//时钟时间频率
    unsigned int lcore_id = rte_lcore_id();
    //PERIODICAL 循环重置定时器 arp_request_timer_cb()是回调函数 mbuf_pool发送数据缓冲池
    unsigned int message = rte_timer_reset(&arp_timer,hz,PERIODICAL,lcore_id,arp_request_timer_cb,mbuf_pool);
    if (message == 0){
        printf("ARP TIMER SUCCESS START!!!\n");
    }else{
        printf("ARP TIMER START FAILURE!!!\n");
    }


}



#endif //DPDKLEARNING_ARPTIMER_H
