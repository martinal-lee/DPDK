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

/**
 * @brief TCP数据包解析
 * @param recv_mbuf_ptr           从内存池取出的内容作为传入的指针
 * @return
 *     -<em>tcp header</em>
 */
static
struct TCPHeader TransTCPDisassemble(unsigned char* recv_mbuf_ptr){
    struct rte_tcp_hdr* tcphdr = (struct rte_tcp_hdr*)recv_mbuf_ptr;
    struct TCPHeader tcphdr_result;
    memset(&tcphdr_result,0,sizeof (tcphdr_result));

    tcphdr_result.srcPort = ntohs(tcphdr->src_port);
    tcphdr_result.dstPort = ntohs(tcphdr->dst_port);
    tcphdr_result.seqNo  = ntohl(tcphdr->sent_seq);
    tcphdr_result.ackNo = ntohl(tcphdr->recv_ack);
    tcphdr_result.headerLen = tcphdr->data_off;

    tcphdr_result.flags = tcphdr->tcp_flags;
    tcphdr_result.window = ntohs(tcphdr->rx_win);
    tcphdr_result.checksum = ntohs(tcphdr->cksum);
    tcphdr_result.urgentPointer = ntohs(tcphdr->tcp_urp);

    return tcphdr_result;
}

#endif //DPDKLEARNING_TRANSLAYERDISASSEMBLE_H
