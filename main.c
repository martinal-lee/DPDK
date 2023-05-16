
#include <rte_mbuf.h>
#include <rte_dev.h>
#include <rte_ethdev.h>
#include <rte_config.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "SendFile.h"
#include "LinkLayerDisassemble.h"
#include "NetworkDisassemble.h"
#include "TransLayerDiassemble.h"
#include "Header.h"
#include "ARPTable.h"
#include "ARPTimer.h"
#include "ConstValue.h"
#include <rte_timer.h>
#include <rte_ring.h>
#include "UDPAndTCPAPI.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
static struct rx_tx_queue* handle_queue = NULL;
static struct rx_tx_queue* handle_queue_instance(void){
    if (handle_queue == NULL){
        handle_queue = rte_malloc("rx tx queue",sizeof (struct rx_tx_queue),0);
        memset(handle_queue,0,sizeof (struct rx_tx_queue));
    }
    return handle_queue;
}

static int UDPServer(void* arg){
/**需要实现udp的socket, bind, recvfrom, sendto 以及close api*/
    int connfd = socket_bypass(AF_INET, SOCK_DGRAM, 0);
    if (connfd == -1) {
        printf("sockfd failed\n");
        return -1;
    }

    struct sockaddr_in localaddr, clientaddr; // struct sockaddr
    memset(&localaddr, 0, sizeof(struct sockaddr_in));

    localaddr.sin_port = (8889);
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = inet_addr("68.87.129.223"); // 0.0.0.0


    bind_bypass(connfd, (struct sockaddr*)&localaddr, sizeof(localaddr));

    char buffer[1024] = {0};
    socklen_t addrlen = sizeof(clientaddr);
    while (1) {

        if (recv_from_bypass(connfd, buffer, 1024, 0,
                      (struct sockaddr*)&clientaddr, &addrlen) < 0) {

            continue;

        } else {

            printf("recv from %s:%d, data:%s\n", inet_ntoa(clientaddr.sin_addr),
                   clientaddr.sin_port, buffer);
            sendto_bypass(connfd, buffer, strlen(buffer), 0,
                    (struct sockaddr*)&clientaddr, sizeof(clientaddr));
        }

    }

    close_bypass(connfd);

}



#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
static int PktProcessMain(void* arg){
    struct rte_mempool* mbuf_pool = (struct rte_mempool*)arg;
    struct rx_tx_queue* ring_buffer = handle_queue_instance();
    struct arp_table* const_arp_table = arp_table_instance();

    while(1){
        struct rte_mbuf* process_mbuf[BURST_BUF_SIZE];
        //多消费者线程安全，接收队列出队
        unsigned int num_recvd = rte_ring_mc_dequeue_burst(ring_buffer->rx_queue,(void**)process_mbuf,BURST_BUF_SIZE,NULL);
        for (unsigned int i = 0; i<num_recvd;i++){
            //接收出以太网帧
            unsigned char *recv_msg=  rte_pktmbuf_mtod(process_mbuf[i], unsigned char*);
            struct ethernetHeader eth_hdr =  LinkDisassemble(recv_msg);
            recv_msg = recv_msg + sizeof(struct ethernetHeader);

            if (eth_hdr.frameType ==RTE_ETHER_TYPE_ARP){
                struct  ARPHeader arphdr = ARPDisassemble(recv_msg);
                if (arphdr.dstIPAddr == localHostIP){//不判断则为arp投毒

                    if (arphdr.operation == RTE_ARP_OP_REQUEST){
                        struct in_addr addr_src, addr_dst;
                        addr_src.s_addr = ntohl(arphdr.srcIPAddr);
                        addr_dst.s_addr = ntohl(arphdr.dstIPAddr);
//
//                        printf("ARP DATA: src ip: %s dst ip: %s src mac: %s dst mac: %s \n",
//                               inet_ntoa(addr_src), inet_ntoa(addr_dst), (arphdr.srcMAC), (arphdr.dstMAC));
                        struct rte_mbuf *send_data_mbuf = ARPSend(mbuf_pool, gDpdkPortID, (unsigned char *) &arphdr.srcMAC, (unsigned char *) &arphdr.srcMAC,
                                                           arphdr.dstIPAddr, arphdr.srcIPAddr,RTE_ARP_OP_REPLY);
                        rte_ring_mp_enqueue_burst(ring_buffer->tx_queue,(void**)send_data_mbuf,1,NULL);
                    }
                    else if (arphdr.operation == RTE_ARP_OP_REPLY){
                        //初始化arp表的静态实例
                        unsigned char* mac = arp_find_dst_mac_addr(arphdr.srcIPAddr);
                        if (mac==NULL){
                            //没有该ip对应的mac地址，需要插入arp表
                            struct arp_entry* arpEntry = rte_malloc("arp entry",sizeof(struct arp_entry),0);
                            if(arpEntry !=NULL){
                                memset(arpEntry,0,sizeof (struct arp_entry));

                                arpEntry->ip = arphdr.srcIPAddr;
                                memcpy(arpEntry->mac,arphdr.srcMAC, (RTE_ETHER_ADDR_LEN));
                                arpEntry->status = ARP_ENTRY_STATUS_DYNAMIC;

                                ADD_ENTRY(const_arp_table->entry,arpEntry);
                                const_arp_table->count++;
                            }
                        }
                    }else{
                        //do nothing
                    }
                }
            }
            if (eth_hdr.frameType == RTE_ETHER_TYPE_IPV4) {
                //去除帧头(偏移量函数)
                struct IPv4Header iphdr = IPv4Disassemble(recv_msg);
                recv_msg = recv_msg + sizeof(struct IPv4Header);

                if (iphdr.protocol == IPPROTO_ICMP){
                    struct ICMPHeader icmphdr = ICMPDisassemble(recv_msg);
                    if (icmphdr.type == RTE_IP_ICMP_ECHO_REQUEST){
                        unsigned char* data = NULL;
                        data = recv_msg+sizeof (struct ICMPHeader);

                        unsigned int data_len = iphdr.totalLen - sizeof (struct ICMPHeader) - sizeof (struct IPv4Header);
                        struct rte_mbuf *send_data_mbuf = ICMPSend(mbuf_pool, gDpdkPortID,eth_hdr.srcMAC,iphdr.dstIP,iphdr.srcIP,icmphdr,0,0,data,data_len);

                        rte_ring_mp_enqueue_burst(ring_buffer->tx_queue,(void**)send_data_mbuf,1,NULL);
                    }
                }
                if (iphdr.protocol == IPPROTO_UDP) {
                    UDPRecvProcess(iphdr, recv_msg);
                }
            }
            //释放接收缓冲区
            rte_pktmbuf_free(process_mbuf[i]);
        }

        //UDP发送
        UDPSendProcess(mbuf_pool,ring_buffer);
    }

    return 0;
}
#pragma clang diagnostic pop

//static int AssembleMain(void* arg){
//    struct rte_mempool* mbuf_pool = (struct rte_mempool*)arg;
//
//
//    return 0;
//}

static const struct rte_eth_conf port_conf_default={
        .rxmode={.max_rx_pkt_len = RTE_ETHER_MAX_LEN}
};

static void init_port(struct rte_mempool *mbuf_pool){
    //判断端口可用性
    uint16_t nb_sys_ports = rte_eth_dev_count_avail();
    if (nb_sys_ports==0){
        rte_exit(EXIT_FAILURE,"无可用端口！\n");
    }
    //获取默认网卡信息(原生)
    struct rte_eth_dev_info dev_info;
    rte_eth_dev_info_get(gDpdkPortID,&dev_info);

    const uint16_t num_rx_queues = 1;
    const uint16_t num_tx_queues = 1;
    struct rte_eth_conf port_conf = port_conf_default;

    //配置接管网卡信息,nb_rx_desc表示队列节点（包）数量
    rte_eth_dev_configure(gDpdkPortID,num_rx_queues,num_tx_queues,&port_conf);
    if(rte_eth_rx_queue_setup(gDpdkPortID,0,512, rte_eth_dev_socket_id(gDpdkPortID)
            ,NULL,mbuf_pool)<0){
        rte_exit(EXIT_FAILURE,"接收队列启动失败！\n");
    }

    struct rte_eth_txconf tx_conf = dev_info.default_txconf;
    //接收多大发送多大
    tx_conf.offloads = port_conf.rxmode.offloads;
    if(rte_eth_tx_queue_setup(gDpdkPortID,0,512, rte_eth_dev_socket_id(gDpdkPortID)
            ,&tx_conf)<0){
        rte_exit(EXIT_FAILURE,"发送队列启动失败！\n");
    }
    //启动设备
    if (rte_eth_dev_start(gDpdkPortID)<0){
        rte_exit(EXIT_FAILURE,"无法启动设备！\n");
    }
    //混杂模式
    rte_eth_promiscuous_enable(gDpdkPortID);
}

int main(int argc ,char* argv[]) {
    if (rte_eal_init(argc,argv)<0){
        rte_exit(EXIT_FAILURE,"eal 初始化失败！\n");
    }

    //所有的dpdk数据均从这里取得
    struct rte_mempool *mbuf_pool
            = rte_pktmbuf_pool_create("mbuf pool",NUM_MBUF,0,0,RTE_MBUF_DEFAULT_BUF_SIZE,(int)rte_socket_id());
    if (mbuf_pool == NULL){
        rte_exit(EXIT_FAILURE,"数据缓冲区初始化失败！\n");
    }
    init_port(mbuf_pool);

    /*arp定时器初始化*/
    rte_timer_subsystem_init();
    struct rte_timer arp_timer1,arp_timer2;
    rte_timer_init(&arp_timer1);
    rte_timer_init(&arp_timer2);

    unsigned int hz = rte_get_timer_hz();//时钟时间频率
    unsigned int lcore_id = rte_lcore_id();
    //struct arp_table_control* table_control = malloc(sizeof (struct arp_table_control));
    //PERIODICAL 循环重置定时器 arp_request_timer_cb()是回调函数 mbuf_pool发送数据缓冲池
    unsigned int message = rte_timer_reset(&arp_timer1,hz,PERIODICAL,lcore_id,arp_request_timer_cb,mbuf_pool);
    struct arp_table* const_arp_table = arp_table_instance();

    rte_timer_reset(&arp_timer2,hz,PERIODICAL,lcore_id,arp_table_show_timer_cb,const_arp_table);
    if (message == 0){
        printf("ARP TIMER SUCCESS START!!!\n");
    }else{
        printf("ARP TIMER START FAILURE!!!\n");
    }

    //初始化接收发送环形缓冲区
    struct rx_tx_queue *ring_buffer = handle_queue_instance();
    if (ring_buffer == NULL){
        rte_exit(EXIT_FAILURE,"接收/发送环形缓冲区初始化失败！\n");
    }
    if (ring_buffer->rx_queue == NULL){
        ring_buffer->rx_queue = rte_ring_create("receive ring buffer",RING_BUFFER_SIZE,(int)rte_socket_id(),RING_F_SP_ENQ | RING_F_SC_DEQ);
    }
    if (ring_buffer->tx_queue == NULL){
        ring_buffer->tx_queue = rte_ring_create("send ring buffer",RING_BUFFER_SIZE,(int)rte_socket_id(),RING_F_SP_ENQ | RING_F_SC_DEQ);
    }
    //接收线程
    unsigned int next_lcore_recv = rte_get_next_lcore(lcore_id,1,0);
    rte_eal_remote_launch(PktProcessMain,mbuf_pool, next_lcore_recv);/**启用多线程，1回调函数 2回调函数参数 3启用的线程数*/

//    //发送线程
//    next_lcore_recv = rte_get_next_lcore(next_lcore_recv,1,0);
//    rte_eal_remote_launch(AssembleMain,mbuf_pool, next_lcore_recv);/**启用多线程，1回调函数 2回调函数参数 3启用的线程数*/

    //udp 处理线程
    next_lcore_recv = rte_get_next_lcore(next_lcore_recv,1,0);
    rte_eal_remote_launch(UDPServer,mbuf_pool, next_lcore_recv);/**启用多线程，1回调函数 2回调函数参数 3启用的线程数*/

    while(1){
        struct rte_mbuf *rx_mbuf[BURST_BUF_SIZE];
        //这里的mbuf在内存池中
        unsigned short num_recvd = rte_eth_rx_burst(gDpdkPortID,0,rx_mbuf,BURST_BUF_SIZE);
        if (num_recvd>BURST_BUF_SIZE){
            rte_exit(EXIT_FAILURE,"网卡接收缓冲区溢出！\n");
        } else if(num_recvd>0){
            /**单生产者入队 1 环形缓冲区 2 网卡缓冲区的mbuf 3 入队个数 4 null*/
            rte_ring_sp_enqueue_burst(ring_buffer->rx_queue, (void**)rx_mbuf, num_recvd, NULL);
        }

        struct rte_mbuf *tx_mbuf[BURST_BUF_SIZE];
        /**单消费者出队 1 环形缓冲区 2 网卡缓冲区的mbuf 3 入队个数(全部出队) 4 null*/
        unsigned int num_send = rte_ring_sc_dequeue_burst(ring_buffer->tx_queue, (void**)tx_mbuf, BURST_BUF_SIZE, NULL);
        if (num_send>0){
            rte_eth_tx_burst(gDpdkPortID, 0, tx_mbuf, num_send);
            //释放缓冲区
            for (unsigned int i =0; i<num_send;i++){
                rte_pktmbuf_free(tx_mbuf[i]);
            }
        }

        //超时发送arp请求
        static unsigned long long prev_tsc = 0, cur_tsc;
        unsigned long long diff_tsc;

        cur_tsc = rte_rdtsc();
        diff_tsc = cur_tsc - prev_tsc;
        if (diff_tsc > TIMER_RESOLUTION_CYCLES) {
            rte_timer_manage();
            prev_tsc = cur_tsc;
        }

    }
}

#pragma clang diagnostic pop