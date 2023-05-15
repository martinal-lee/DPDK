/**
* @File name: SendFile.h
* @Author: Martinal
* @Version: -
* @Description:send package from all layers
* */

#ifndef DPDKLEARNING_SENDFILE_H
#define DPDKLEARNING_SENDFILE_H
#include <rte_mbuf.h>
#include <rte_dev.h>
#include <rte_ethdev.h>
#include <rte_config.h>

#include <arpa/inet.h>
#include "TransLayerAssemble.h"
#include "LinkLayerAssemble.h"
#include "NetworkAssemble.h"
#include "Header.h"

/**
 * @brief UDP数据报发送
 * @param mbuf_pool             dpdk内存池，用于申请内存块mbuf
 * @param dev_port              指定发送设备号
 * @param data                  udp数据
 * @param udp_data_length       udp数据长度
 * @param dst_mac               目的MAC
 * @param src_ip                源ip
 * @param dst_ip                目的ip
 * @param src_port              源端口
 * @param dst_port              目的端口
 *
 * @return
 *     -<em>rte_mbuf* </em>
 */
static struct
rte_mbuf* UDPSend(struct rte_mempool *mbuf_pool,u_int16_t dev_port,uint8_t *data,uint16_t udp_data_length,uint8_t *dst_mac,uint32_t src_ip,uint32_t dst_ip,uint16_t src_port,
                            uint16_t dst_port){
    //整个帧长度
    unsigned total_len = udp_data_length + 14+20+8;
    if (total_len<60) total_len = 60;
    //mbuf从哪儿开始，不用设置大小
    struct rte_mbuf *mbuf = rte_pktmbuf_alloc(mbuf_pool);
    if (!mbuf) {
        rte_exit(EXIT_FAILURE, "申请发送mbuf失败！\n");
    }
    mbuf->pkt_len = total_len;
    mbuf->data_len = total_len;
    //mbuf指针的位置,强转为unsigned char*
    uint8_t *msg = rte_pktmbuf_mtod(mbuf, uint8_t*);

    unsigned char src_mac[6];
    rte_eth_macaddr_get(dev_port,(struct rte_ether_addr*)src_mac);
    LinkAssemble(msg,src_mac,dst_mac,RTE_ETHER_TYPE_IPV4,NULL,0,0);
    struct rte_ipv4_hdr* transportChecksumPtr = IPv4Assemble(msg,src_ip,dst_ip,IPPROTO_UDP,total_len-14,NULL,0);
    UDPAssemble(msg,dst_port,src_port,data,transportChecksumPtr,udp_data_length+8,1);


    return mbuf;
}

/**
 * @brief arp数据响应包
 * @param mbuf_pool             dpdk内存池，用于申请内存块mbuf
 * @param dev_port              指定发送设备号
 * @param link_layer_dst_mac    链路层的mac
 * @param arp_dst_mac           arp包中的目的MAC
 * @param src_ip                源ip
 * @param dst_ip                目的ip
 * @param arpType               arp的类型，1为请求报文 2为点对点响应报文
 *
 * @return
 *     -<em>rte_mbuf* </em>
 */
static struct
rte_mbuf* ARPSend(struct rte_mempool *mbuf_pool,unsigned short dev_port,unsigned char *link_layer_dst_mac,
                  unsigned char *arp_dst_mac,unsigned int src_ip,unsigned int dst_ip,unsigned short arpType){
    //整个帧长度
    unsigned total_len = 42;
    if (total_len<60) total_len = 60;
    //mbuf从哪儿开始，不用设置大小
    struct rte_mbuf *mbuf = rte_pktmbuf_alloc(mbuf_pool);
    if (!mbuf) {
        rte_exit(EXIT_FAILURE, "申请发送mbuf失败！\n");
    }
    mbuf->pkt_len = total_len;
    mbuf->data_len = total_len;
    //mbuf指针的位置,强转为unsigned char*
    uint8_t *msg = rte_pktmbuf_mtod(mbuf, uint8_t*);

    unsigned char src_mac[6];
    rte_eth_macaddr_get(dev_port,(struct rte_ether_addr*)src_mac);
    LinkAssemble(msg,src_mac,link_layer_dst_mac,RTE_ETHER_TYPE_ARP,NULL,0,0);
    ARPAssemble(msg,src_mac,arp_dst_mac,src_ip,dst_ip,arpType);

    return mbuf;
}

/**
 * @brief icmp数据发送
 * @param mbuf_pool             dpdk内存池，用于申请内存块mbuf
 * @param dev_port              指定发送设备号
 * @param dst_mac                目的MAC
 * @param src_ip                源IP
 * @param dst_ip               目的IP
 * @param icmphdr                icmp解析头，用于匹配回显报文中的字段
 * @param type                报文的类型
 * @param code                  报文的操作码
 * @param data              报文数据
 * @param data_len              报文数据字段长度
 *
 * @return
 *     -<em>rte_mbuf* </em>
 */
static struct
rte_mbuf* ICMPSend(struct rte_mempool *mbuf_pool,unsigned short dev_port,unsigned char *dst_mac,unsigned int src_ip,unsigned int dst_ip,
        struct ICMPHeader icmphdr,unsigned char type,unsigned char code,unsigned char* data,unsigned int data_len){
    //整个帧长度
    unsigned total_len = 14+20+sizeof(struct ICMPHeader)+data_len;
    if (total_len<60) total_len = 60;
    //mbuf从哪儿开始，不用设置大小
    struct rte_mbuf *mbuf = rte_pktmbuf_alloc(mbuf_pool);
    if (!mbuf) {
        rte_exit(EXIT_FAILURE, "申请发送mbuf失败！\n");
    }
    mbuf->pkt_len = total_len;
    mbuf->data_len = total_len;
    //mbuf指针的位置,强转为unsigned char*
    uint8_t *msg = rte_pktmbuf_mtod(mbuf, uint8_t*);

    unsigned char src_mac[6];
    rte_eth_macaddr_get(dev_port,(struct rte_ether_addr*)src_mac);
    LinkAssemble(msg,src_mac,dst_mac,RTE_ETHER_TYPE_IPV4,NULL,0,0);
    struct rte_ipv4_hdr* transportChecksumPtr = IPv4Assemble(msg,src_ip,dst_ip,IPPROTO_ICMP,total_len-14,NULL,0);
    ICMPAssemble(msg,type,code,icmphdr.id,icmphdr.seq,data,data_len);
    return mbuf;
}

#endif //DPDKLEARNING_SENDFILE_H
