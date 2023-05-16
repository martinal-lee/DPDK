/**
* @File name: PublicTools.h
* @Author: Martinal
* @Version: -
* @Description: 公共的函数工具
* */
#ifndef DPDKLEARNING_PUBLICTOOLS_H
#define DPDKLEARNING_PUBLICTOOLS_H

#include <rte_ethdev.h>




void
print_ethaddr(const char *decription, const struct rte_ether_addr *eth_addr)
{
    char buf[RTE_ETHER_ADDR_FMT_SIZE];
    rte_ether_format_addr(buf, RTE_ETHER_ADDR_FMT_SIZE, eth_addr);
    printf("%s%s ", decription, buf);
}


#endif //DPDKLEARNING_PUBLICTOOLS_H
