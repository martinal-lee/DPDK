/**
* @File name: LinkLayerDisassemble.h
* @Author: Martinal
* @Version: -
* @Description: disssemble link layer data package（ieee 802.3）
* */


#ifndef DPDKLEARNING_LINKLAYERDISASSEMBLE_H
#define DPDKLEARNING_LINKLAYERDISASSEMBLE_H


#include <rte_ethdev.h>
#include "Header.h"


/**
 * @brief 以太网帧解析
 * @param recv_mbuf_ptr           从内存池取出的内容作为传入的指针
 * @return
 *     -<em>frame header</em>
 */
static
struct ethernetHeader LinkDisassemble(unsigned char* recv_mbuf_ptr){
    struct rte_ether_hdr* ether = (struct rte_ether_hdr*)recv_mbuf_ptr;
    struct ethernetHeader ether_result;
    memset(&ether_result,0,sizeof (ether_result));

    memcpy(ether_result.srcMAC,&ether->s_addr,6);
    memcpy(ether_result.dstMAC,&ether->d_addr,6);

    ether_result.frameType = htons(ether->ether_type);

    return ether_result;
}


#endif //DPDKLEARNING_LINKLAYERDISASSEMBLE_H
