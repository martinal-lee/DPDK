/**
* @File name: UDPAPI.h
* @Author: Martinal
* @Version: -
* @Description: udp的接口函数
* */

#ifndef DPDKLEARNING_UDPAPI_H
#define DPDKLEARNING_UDPAPI_H
#include <rte_ethdev.h>
// 本地地址的socket句柄信息
struct localhost{
    int fd;

    unsigned int localIP;
    unsigned char localMAC[RTE_ETHER_ADDR_LEN];
    unsigned short localPort;

    unsigned char protocol;

    struct rte_ring* recv_buffer;
    struct rte_ring* send_buffer;

    struct localhost* prev;
    struct localhost* next;

    //条件变量通知是否有数据，若无则等待，实现阻塞同步
    pthread_cond_t cond;//条件变量
    pthread_mutex_t mutex;//互斥变量
};

//单例
static struct localhost* lhost_list = NULL;

/**
 * @brief udp socketfd 查找
 * @param sockfd           待查找的唯一标识符
 * @return
 *     -<em>localhost结构体</em>
 */




#endif //DPDKLEARNING_UDPAPI_H
