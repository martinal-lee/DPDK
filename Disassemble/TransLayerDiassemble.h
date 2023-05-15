/**
* @File name: TransLayerDisassemble.h
* @Author: Martinal
* @Version: -
* @Description: disssemble Transport layer data
* */


#ifndef DPDKLEARNING_TRANSLAYERDISASSEMBLE_H
#define DPDKLEARNING_TRANSLAYERDISASSEMBLE_H



#include <rte_ethdev.h>
#include "Header.h"
#include "NetworkDisassemble.h"


/**
 * @brief udp数据包解析
 * @param recv_mbuf_ptr           从内存池取出的内容作为传入的指针
 * @return
 *     -<em>udp header</em>
 */
static
struct UDPHeader TransUDPDisassemble(unsigned char* recv_mbuf_ptr){
    struct rte_udp_hdr* udphdr = (struct rte_udp_hdr*)recv_mbuf_ptr;
    struct UDPHeader udphdr_result;
    memset(&udphdr_result,0,sizeof (udphdr_result));

    udphdr_result.srcPort = htons(udphdr->src_port);
    udphdr_result.dstPort = htons(udphdr->dst_port);
    udphdr_result.len = htons(udphdr->dgram_len);
    udphdr_result.checkSum = htons(udphdr->dgram_cksum);

    return udphdr_result;
}


#endif //DPDKLEARNING_TRANSLAYERDISASSEMBLE_H
