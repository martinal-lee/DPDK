/**
* @File name: TransportLayerAssemble.h
* @Author: Martinal
* @Version: -
* @Description: assemble transport layer data package（udp、tcp etc.）
* */

#ifndef DPDKLEARNING_TRANSLAYERASSEMBLE_H
#define DPDKLEARNING_TRANSLAYERASSEMBLE_H

#include <rte_mbuf.h>
#include <rte_dev.h>
#include <rte_ethdev.h>
#include <rte_config.h>

/**
 * @brief udp数据包封装
 * @param msg               dpdk内存池，用于写数据
 * @param src_port          源端口
 * @param dst_port          目的端口
 * @param data              udp负载
 * @param iphdr             用于计算校验和
 * @param total_len         udp包总长度
 * @param data_flag         数据标识：1 直接组装udp包 0 不直接组装udp包
 *
 * @return
 *     -<em>None</em>
 */
static void
UDPAssemble(unsigned char* msg,unsigned short dst_port,unsigned short src_port,unsigned char* data,struct rte_ipv4_hdr *iphdr,unsigned short total_len,unsigned int data_flag){


    struct rte_udp_hdr* udphdr = (struct rte_udp_hdr*)(msg+sizeof(struct rte_ipv4_hdr)+sizeof (struct rte_ether_hdr));
    udphdr->src_port = htons(src_port);
    udphdr->dst_port = htons(dst_port);
    udphdr->dgram_len = htons(total_len);

    if (data_flag == 1){
        unsigned char* udp_data= (unsigned char*)(udphdr+1);
        rte_memcpy(udp_data,data,total_len - 8);
    }else{
        //do nothing
    }

    udphdr->dgram_cksum = 0;
    udphdr->dgram_cksum = rte_ipv4_udptcp_cksum(iphdr,udphdr);
}

//static void TCPAssemble(unsigned char* msg,unsigned char* dst_mac,unsigned int src_ip,unsigned int dst_ip,unsigned short src_port,
//                              unsigned short dst_port,unsigned short total_len);


#endif //DPDKLEARNING_TRANSLAYERASSEMBLE_H
