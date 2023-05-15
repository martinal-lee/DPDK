/**
* @File name: NetworkAssemble.h
* @Author: Martinal
* @Version: -
* @Description: assemble network layer data （ipv4）
* */

#ifndef DPDKLEARNING_NETWORKASSEMBLE_H
#define DPDKLEARNING_NETWORKASSEMBLE_H

#include <rte_mbuf.h>
#include <rte_dev.h>
#include <rte_ethdev.h>
#include <rte_config.h>

/**
 * @brief arp数据封装
 * @param msg               dpdk内存池，用于写数据
 * @param src_mac           源mac
 * @param dst_mac           目的mac
 * @param src_ip            源ip
 * @param dst_ip            目的IP
 * @param arpType           数据标识：1 arp请求 2 arp响应
 *
 * @return
 *     -<em>None</em>
 */
static void
ARPAssemble(unsigned char* msg,unsigned char* src_mac,unsigned char* dst_mac,unsigned int src_ip,unsigned int dst_ip,unsigned short arpType){

    struct rte_arp_hdr* arphdr = (struct  rte_arp_hdr*)(msg+sizeof(struct rte_ether_hdr));

    arphdr->arp_hardware = htons(0x0001);
    arphdr->arp_protocol = htons(0x0800);
    arphdr->arp_hlen = 6;
    arphdr->arp_plen = 4;
    arphdr->arp_opcode = htons(arpType);

    rte_memcpy(arphdr->arp_data.arp_sha.addr_bytes,src_mac,RTE_ETHER_ADDR_LEN);
    rte_memcpy(arphdr->arp_data.arp_tha.addr_bytes,dst_mac,RTE_ETHER_ADDR_LEN);

    arphdr->arp_data.arp_sip = htonl(src_ip);
    arphdr->arp_data.arp_tip = htonl(dst_ip);


}

/**
 * @brief ipv4数据包封装
 * @param msg               dpdk内存池，用于写数据
 * @param src_ip            源ip
 * @param dst_ip            目的IP
 * @param protocol_type     上层层协议标识
 * @param total_len         ip包总长度
 * @param data              ip负载
 * @param data_flag         数据标识：1 直接组装ip包 0 不直接组装ip包
 *
 * @return
 *     -<em>struct rte_ipv4_hdr*</em> 用于上层校验和
 */
static struct rte_ipv4_hdr*
IPv4Assemble(unsigned char* msg,unsigned int src_ip,unsigned int dst_ip,unsigned short protocol_type,unsigned short total_len,unsigned char* data,unsigned int data_flag){

    struct rte_ipv4_hdr* iphdr = (struct rte_ipv4_hdr*)(msg+sizeof(struct rte_ether_hdr));
    iphdr->version_ihl = 0x45;
    iphdr->type_of_service = 0;
    iphdr->total_length = htons(total_len);
    iphdr->packet_id = 0;
    iphdr->fragment_offset = 0;
    iphdr->time_to_live = 64;
    iphdr->next_proto_id = protocol_type;
    iphdr->src_addr = htonl(src_ip);
    iphdr->dst_addr = htonl(dst_ip);

    iphdr->hdr_checksum = 0;//先赋0，再计算
    iphdr->hdr_checksum = rte_ipv4_cksum(iphdr);

    struct rte_ipv4_hdr* transportChecksumPtr = iphdr;
    if (data_flag == 1){
        unsigned char* data_ptr =(unsigned char*)(iphdr+1);
        //固定长度首部
        rte_memcpy(data_ptr,data,total_len-20);
    }else{
        //do nothing
    }

    return transportChecksumPtr;
}
/**
 * @brief icmp校验和计算
 * @param msg               dpdk内存池，用于写数据
 * @param iphdr             IP头，用于校验和
 * @param type              icmp类型
 * @param code              icmp操作码
 * @param id                icmp id
 * @param seq               icmp序列号
 * @param data              icmp数据
 * @param data_len          icmp数据长度
 *
 * @return
 *     -<em>None</em>
 */
static unsigned short ICMPCksum(unsigned short *addr, unsigned short count){
    register long cksum = 0;
    while(count >1){
        cksum+=*(unsigned short*)addr++;
        count -=2;

    }
    if(count>0){
        cksum+= *(unsigned  char *)addr;
    }

    while(cksum>>16){
        cksum = (cksum&0xffff) +(cksum>>16);
    }


    return ~cksum;
}
/**
 * @brief icmp数据包封装
 * @param msg               dpdk内存池，用于写数据
 * @param type              icmp类型
 * @param code              icmp操作码
 * @param id                icmp id
 * @param seq               icmp序列号
 * @param data              icmp数据
 * @param data_len          icmp数据长度
 *
 * @return
 *     -<em>None</em>
 */
static void
ICMPAssemble(unsigned char* msg,unsigned char type,unsigned char code,unsigned short id,unsigned short seq,unsigned char* data,unsigned int data_len){

    struct rte_icmp_hdr* icmphdr = (struct rte_icmp_hdr*)(msg+sizeof(struct rte_ipv4_hdr)+sizeof (struct rte_ether_hdr));
    icmphdr->icmp_cksum = 0;
    icmphdr->icmp_seq_nb = htons(seq);
    icmphdr->icmp_ident = htons(id);
    icmphdr->icmp_type = type;
    icmphdr->icmp_code = code;

    unsigned char* icmp_data = msg+sizeof(struct rte_ipv4_hdr)+sizeof (struct rte_ether_hdr)+sizeof(struct rte_icmp_hdr);
    rte_memcpy(icmp_data,data,data_len);
    icmphdr->icmp_cksum = ICMPCksum((unsigned short *)icmphdr,sizeof(struct rte_icmp_hdr)+data_len);


}


#endif //DPDKLEARNING_NETWORKASSEMBLE_H
