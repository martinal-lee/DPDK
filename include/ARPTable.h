/**
* @File name: ARPTable.h
* @Author: Martinal
* @Version: -
* @Description: arp table
* */

#ifndef DPDKLEARNING_ARPTABLE_H
#define DPDKLEARNING_ARPTABLE_H

#pragma pack(1)

#include <rte_malloc.h>
#include <rte_ethdev.h>

#define ARP_ENTRY_STATUS_DYNAMIC 0     /**动态arp*/
#define ARP_ENTRY_STATUS_STATIC 1      /**静态arp*/


//arp表项目
struct arp_entry{
    unsigned int ip; //ip地址
    unsigned char mac[RTE_ETHER_ADDR_LEN]; //mac地址
    unsigned char status;   // 0 静态 1 动态

    struct  arp_entry* prev;
    struct  arp_entry* next;
};

//arp表
struct arp_table{
    struct arp_entry* entry; //arp表项
    int count; //arp表中条目数
};


//单例模式arp表，c中使用static实现
static struct arp_table* arpt = NULL;

static struct arp_table* arp_table_instance(){
    if (arpt == NULL){
        arpt = rte_malloc("arp_table",sizeof (struct arp_table),0);
        if (arpt == NULL){
            rte_exit(EXIT_FAILURE,"ARP申请内存失败！\n");
        }
        memset(arpt,0,sizeof (struct arp_table));
    }
    return arpt;
}
//查找arp表IP对应的mac地址
unsigned char*
arp_find_dst_mac_addr(unsigned int ip){
    struct arp_entry* iter;
    struct arp_table* arpTable = arp_table_instance();


    for (iter = arpTable->entry;iter!=NULL;iter = iter->next){
        if (ip == iter->ip){
            return iter->mac;
        }
        return NULL;
    }

}
#pragma pack()
#endif //DPDKLEARNING_ARPTABLE_H
