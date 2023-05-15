/**
* @File name: PublicTools.h
* @Author: Martinal
* @Version: -
* @Description: 公共的函数工具
* */
#ifndef DPDKLEARNING_PUBLICTOOLS_H
#define DPDKLEARNING_PUBLICTOOLS_H

#include <rte_ethdev.h>
//头插法双链表
#define ADD_ENTRY(arp_table_first_entry, arp_entry) do{ \
     (arp_entry)->next = arp_table_first_entry;                 \
     (arp_entry)->prev = NULL;                                 \
     if((arp_table_first_entry)!=NULL){                        \
        (arp_table_first_entry)->prev = arp_entry;                                          \
     }                                                       \
     (arp_table_first_entry) = arp_entry;                     \
                                                           \
}while(0);

#define REMOVE_ENTRY(arp_table_first_entry,arp_entry) do{ \
    if ((arp_table_first_entry) == NULL) break;                   \
    if ((arp_entry)->prev != NULL) (arp_entry)->prev->next = (arp_entry)->next; \
    if ((arp_entry)->next != NULL) (arp_entry)->next->prev = (arp_entry)->prev; \
    if ((arp_entry) == (arp_table_first_entry)) (arp_table_first_entry)->next = (arp_entry)->next; \
    (arp_entry)->prev = (arp_entry)->next = NULL;                   \
    }                                                         \
}while(0);



void
print_ethaddr(const char *decription, const struct rte_ether_addr *eth_addr)
{
    char buf[RTE_ETHER_ADDR_FMT_SIZE];
    rte_ether_format_addr(buf, RTE_ETHER_ADDR_FMT_SIZE, eth_addr);
    printf("%s%s ", decription, buf);
}


#endif //DPDKLEARNING_PUBLICTOOLS_H
