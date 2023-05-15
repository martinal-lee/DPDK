/**
* @File name: NetworkLayerDisassemble.h
* @Author: Martinal
* @Version: -
* @Description: disssembleNetwork layer data package
* */


#ifndef DPDKLEARNING_NETWORKLAYERDISASSEMBLE_H
#define DPDKLEARNING_NETWORKLAYERDISASSEMBLE_H



#include <rte_ethdev.h>
#include "Header.h"


/**
 * @brief arp数据包解析
 * @param recv_mbuf_ptr           从内存池取出的内容作为传入的指针
 * @return
 *     -<em>arp header</em>
 */
static
struct ARPHeader ARPDisassemble(unsigned char* recv_mbuf_ptr){
    struct rte_arp_hdr* arphdr = (struct rte_arp_hdr*)recv_mbuf_ptr;
    struct ARPHeader arphdr_result;
    memset(&arphdr_result,0,sizeof (arphdr_result));

    arphdr_result.hardwareType = ntohs(arphdr->arp_hardware);
    arphdr_result.protoType = ntohs(arphdr->arp_protocol);
    arphdr_result.hardAddrLen = arphdr->arp_hlen;
    arphdr_result.protoAddrLen = arphdr->arp_plen;
    arphdr_result.operation = ntohs(arphdr->arp_opcode);
    rte_memcpy(arphdr_result.srcMAC,arphdr->arp_data.arp_sha.addr_bytes,RTE_ETHER_ADDR_LEN);
    rte_memcpy(arphdr_result.dstMAC,arphdr->arp_data.arp_tha.addr_bytes,RTE_ETHER_ADDR_LEN);
    arphdr_result.srcIPAddr = ntohl(arphdr->arp_data.arp_sip);
    arphdr_result.dstIPAddr = ntohl(arphdr->arp_data.arp_tip);

    return arphdr_result;
}

/**
 * @brief ipv4数据包解析
 * @param recv_mbuf_ptr           从内存池取出的内容作为传入的指针
 * @return
 *     -<em>ipv4 header</em>
 */
static
struct IPv4Header IPv4Disassemble(unsigned char* recv_mbuf_ptr){
    struct rte_ipv4_hdr* iphdr = (struct rte_ipv4_hdr*)recv_mbuf_ptr;
    struct IPv4Header iphdr_result;
    memset(&iphdr_result,0,sizeof (iphdr_result));

    iphdr_result.verHLen = iphdr->version_ihl;
    iphdr_result.tos = iphdr->type_of_service;
    iphdr_result.totalLen = htons(iphdr->total_length);
    iphdr_result.id = htons(iphdr->packet_id);
    iphdr_result.ttl = iphdr->time_to_live;
    iphdr_result.protocol = iphdr->next_proto_id;
    iphdr_result.checksum = htons(iphdr->hdr_checksum);
    iphdr_result.srcIP = htonl(iphdr->src_addr);
    iphdr_result.dstIP = htonl(iphdr->dst_addr);

    return iphdr_result;
}

/**
 * @brief icmp数据包解析
 * @param recv_mbuf_ptr           从内存池取出的内容作为传入的指针
 * @return
 *     -<em>icmp header</em>
 */
static
struct ICMPHeader ICMPDisassemble(unsigned char* recv_mbuf_ptr){
    struct rte_icmp_hdr* icmphdr = (struct rte_icmp_hdr*)recv_mbuf_ptr;
    struct ICMPHeader icmphdr_result;
    memset(&icmphdr_result,0,sizeof (icmphdr_result));

    icmphdr_result.type = icmphdr->icmp_type;
    icmphdr_result.code = icmphdr->icmp_code;
    icmphdr_result.crc = ntohs(icmphdr->icmp_cksum);
    icmphdr_result.id = ntohs(icmphdr->icmp_ident);
    icmphdr_result.seq = ntohs(icmphdr->icmp_seq_nb);

    return icmphdr_result;
}

#endif //DPDKLEARNING_NETWORKLAYERDISASSEMBLE_H
