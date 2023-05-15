/**
* @File name: LinkLayerAssemble.c
* @Author: Martinal
* @Version: -
* @Description: assemble link layer data package（ieee 802.3）
* */


#ifndef DPDKLEARNING_LINKLAYERASSEMBLE_H
#define DPDKLEARNING_LINKLAYERASSEMBLE_H

#include <rte_mbuf.h>
#include <rte_dev.h>
#include <rte_ethdev.h>
#include <rte_config.h>

/**
 * @brief 以太网帧封装
 * @param msg           dpdk内存池，用于写数据
 * @param dst_mac       目的mac
 * @param src_mac       源mac
 * @param protocol_type   网络层协议标识
 * @param data          以太网帧负载
 * @param data_len     负载长度，不直接组包时为0
 * @param data_flag     数据标识：1 直接组装mac包 0 不直接组装mac包
 *
 * @return
 *     -<em>None</em>
 */
static void
LinkAssemble(unsigned char* msg,unsigned char* src_mac,unsigned char* dst_mac, unsigned short protocol_type,unsigned char* data,unsigned int data_len,unsigned int data_flag){
    struct rte_ether_hdr* ethhdr = (struct rte_ether_hdr*)msg;
    rte_memcpy(ethhdr->s_addr.addr_bytes,src_mac,RTE_ETHER_ADDR_LEN);
    rte_memcpy(ethhdr->d_addr.addr_bytes,dst_mac,RTE_ETHER_ADDR_LEN);

    ethhdr->ether_type = htons(protocol_type);

    if (data_flag == 1){
        unsigned char* data_ptr = (unsigned char*)(ethhdr+1);
        rte_memcpy(data_ptr,data,data_len);
    }else{
        //do nothing
    }
}


#endif //DPDKLEARNING_LINKLAYERASSEMBLE_H
