/**
* @File name: UDPAndTCPAPI.h
* @Author: Martinal
* @Version: -
* @Description: udp的接口函数
* */


#ifndef DPDKLEARNING_UDPANDTCPAPI_H
#define DPDKLEARNING_UDPANDTCPAPI_H
#include <rte_ethdev.h>
#include <rte_malloc.h>

//头插法双链表
#define ADD_UDP_SOCKET_ENTRY(arp_table_first_entry, arp_entry) do{ \
     (arp_entry)->next = arp_table_first_entry;                 \
     (arp_entry)->prev = NULL;                                 \
     if((arp_table_first_entry)!=NULL){                        \
        (arp_table_first_entry)->prev = arp_entry;                                          \
     }                                                       \
     (arp_table_first_entry) = arp_entry;                     \
                                                           \
}while(0);

#define REMOVE_UDP_SOCKET_ENTRY(arp_table_first_entry,arp_entry) do{ \
    if ((arp_table_first_entry) == NULL) break;                   \
    if ((arp_entry)->prev != NULL) (arp_entry)->prev->next = (arp_entry)->next; \
    if ((arp_entry)->next != NULL) (arp_entry)->next->prev = (arp_entry)->prev; \
    if ((arp_entry) == (arp_table_first_entry)) (arp_table_first_entry)->next = (arp_entry)->next; \
    (arp_entry)->prev = (arp_entry)->next = NULL;                   \
}while(0);

// 本地地址的socket句柄信息
struct localhost{
    int fd;

    unsigned int localIP;
    unsigned char localMAC[RTE_ETHER_ADDR_LEN];
    unsigned short localPort;

    unsigned char protocol;

    struct rte_ring* recv_buffer;
    struct rte_ring* send_buffer;

    //暂时不需要红黑树
    struct localhost* prev;
    struct localhost* next;

    //条件变量通知是否有数据，若无则等待，实现阻塞同步
    pthread_cond_t cond;//条件变量
    pthread_mutex_t mutex;//互斥变量
};
//arp表
struct localhost_table{
    struct localhost* entry; //arp表项
    int count; //arp表中条目数
};

// 在协议栈线程结束时放入传输层接收队列struct rte_ring* recv_buffer;的结构体
struct TransportRecvBufSingleData{
    unsigned int srcIP;
    unsigned int dstIP;
    unsigned short srcPort;
    unsigned short dstPort;

    unsigned int protocol;

    unsigned char* data;
    unsigned short transport_payload_length;//传输层payload长度
    //为什么不是直接传输udp/tcp数据包上去？猜想原因是因为高层在发送时不知道发给谁？？
};

//单例
static struct localhost_table* host_list = NULL;
struct localhost_table* get_host_list_instance(){
    if (host_list == NULL){
        host_list = rte_malloc("socket fd",sizeof (struct localhost_table),0);
        if (host_list == NULL){
            rte_exit(EXIT_FAILURE,"socket fd 链表创建失败！\n");
        }
        memset(host_list,0,sizeof (struct localhost_table));
    }
    return host_list;
}

/**
 * @brief socketfd find     查找
 * @param sockfd            待查找的唯一标识符
 * @return
 *     -<em>localhost结构体</em>
 */
struct localhost* getHostInfoFromFD(int fd){
    struct localhost* host;
    for (host = host_list->entry;host != NULL; host = host->next){
        if (fd == host->fd){
            return host;
        }
    }
    return NULL;
}


/**
 * @brief socket find   查找(通过端口、ip以及协议标识)
 * @param ip            待查找的socket ip
 * @param port          待查找的socket port
 * @param proto         待查找的socket protocol
 *
 * @return
 *     -<em>localhost结构体</em>
 */
struct localhost* getHostInfoFromPortAndIP(unsigned int ip,unsigned short port ,unsigned char proto){
    struct localhost* host;

    for (host = host_list->entry;host != NULL; host = host->next){
        if (ip == host->localIP && port == host->localPort && proto == host->protocol){
            return host;
        }
    }
    return NULL;
}

/**
 * @brief 从bitmap中获取随机值
 * @param None 待查找的唯一标识符
 * @return
 *     -<em>fd value</em>
 */
static int get_fd_from_bitmap(){
    //应该是从bitmap中获取fd的随机值,0 1 2系统占用
    int fd = 3;
    return fd;
}

/**
 * @brief socket api        tcp and udp
 * @param type           套接字的类型
 * @return
 *     -<em>localhost结构体</em>
 */
int socket_bypass(__attribute__((unused)) int domain,int type,__attribute__((unused)) int protocol){
    /**简单实现，不需要协议以及工作方式*/
    //分配可用id
    int fd = get_fd_from_bitmap();

    struct localhost_table*  localhost_list = get_host_list_instance();
    struct localhost* host = rte_malloc("localhost",sizeof (struct localhost),0);
    memset(host,0,sizeof (struct localhost));

    host->fd = fd;
    if (type == SOCK_DGRAM){
        host->protocol = IPPROTO_UDP;
    } else if (type == SOCK_STREAM){
        host->protocol = IPPROTO_TCP;
    }
    host->recv_buffer = rte_ring_create("recv buffer transport layer",RING_BUFFER_SIZE,rte_socket_id(),RING_F_SP_ENQ | RING_F_SC_DEQ);
    if (host->recv_buffer == NULL){
        rte_free(host->recv_buffer);
        return -2;//环形缓冲区申请失败
    }
    host->send_buffer = rte_ring_create("send buffer transport layer",RING_BUFFER_SIZE,rte_socket_id(),RING_F_SP_ENQ | RING_F_SC_DEQ);
    if (host->send_buffer == NULL){
        rte_ring_free(host->send_buffer);
        rte_free(host->send_buffer);
        return -2;//环形缓冲区申请失败
    }

    //初始化该套接字的条件变量
    pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
    memcpy(&host->cond,&blank_cond,sizeof(pthread_cond_t));
    memcpy(&host->mutex,&blank_mutex,sizeof(pthread_mutex_t));

    //加入链表
    ADD_UDP_SOCKET_ENTRY(localhost_list->entry,host);
    return fd;

}

/**
* @brief bind socket 绑定套接字
* @param fd           待查找的唯一标识符
* @return
*     -<em>成功or失败</em>
*/
int bind_bypass(int fd,const struct sockaddr* addr,socklen_t addrLen){
    /**把对应的ip，port绑定到已分配的fd上去，实现唯一的标识*/
    struct localhost* host = getHostInfoFromFD(fd);
    if (host == NULL){
        return -1;//bind failure
    }

    const struct sockaddr_in *addr_in = (const struct sockaddr_in*)addr;
    host->localPort = addr_in->sin_port;
    memcpy(&host->localIP,&addr_in->sin_addr.s_addr,sizeof(unsigned int ));
    memcpy(host->localMAC,localhost_mac,RTE_ETHER_ADDR_LEN);


    return 0;
}

/**
 * @brief receive data api
 * @param sockfd           待查找的唯一标识符
 * @param buf
 * @param len
 * @param src_addr
 *
 * @return
 *     -<em>成功or失败</em>
 */
unsigned long recv_from_bypass(int sockfd, void *buf, unsigned long len, __attribute__((unused))  int flags,
                     struct sockaddr *src_addr, __attribute__((unused))  socklen_t *addrlen){
    struct localhost* host_socket = getHostInfoFromFD(sockfd);
    if(host_socket == NULL){
        return -1;
    }

    struct TransportRecvBufSingleData* t_data =NULL;
    unsigned char* data_ptr = NULL;

    struct sockaddr_in* addr_in = (struct sockaddr_in*)src_addr;

    int npacket = -1;//出队的数据包数量
    pthread_mutex_lock(&host_socket->mutex);
    /**
     * 1 这里是使用取地址的t_data还是使用直接传递指针名的t_data???-->确定使用（void**）&t_data，否则会段错误
     * 2 这里的void**理解为第二个*与&互逆操作，t_data是指针指向的地址，void*不用管，可以理解为指向该地址的指针，由系统负责
     * 3 数组名不用取地址是因为指针数组的名字就是第一个指针的地址，（*数组名）就是数组第一个指针，与直接引用数组名一致，故没有影响
     */
    while((npacket = rte_ring_mc_dequeue(host_socket->recv_buffer,(void**)&t_data))<0){
        //同步条件变量：队列中没有值则这这里阻塞等待
        pthread_cond_wait(&host_socket->cond,&host_socket->mutex);
    }
    pthread_mutex_unlock(&host_socket->mutex);

    addr_in->sin_port = t_data->srcPort;
    addr_in->sin_addr.s_addr = t_data->srcIP;
    //仅仅用于输出，否则上面的地址会和整个程序逻辑不一致
    struct in_addr print_addr;
    memset(&print_addr,0,sizeof(struct in_addr));
    print_addr.s_addr = ntohl(t_data->srcIP);
    //printf("UDP DATA RECV FROM ---> IP: %s,PORT: %d \n", inet_ntoa(print_addr),addr_in->sin_port);

    //设置的接收大小小于实际接收到的数据（这里的数据指的是传输层负载载荷！）
    unsigned char* ptr = NULL;
    if (len < t_data->transport_payload_length){
        rte_memcpy(buf,t_data->data,len);
        ptr = rte_malloc("larger than recv len",t_data->transport_payload_length-len,0);
        rte_memcpy(ptr,t_data->data+len,t_data->transport_payload_length-len);

        //超长的数据重新放入接收队列（重新放入接收队列应该是在队尾，这样子若原本还有数据
        // 则不会影响数据的顺序吗？）
        t_data->transport_payload_length -= len;
        rte_free(t_data->data);
        t_data->data = rte_malloc("remalloc data space",sizeof (t_data->transport_payload_length),0);
        rte_memcpy(t_data->data,ptr,t_data->transport_payload_length);

        rte_ring_mp_enqueue(host_socket->recv_buffer,t_data);

        return len;
    } else{
        rte_memcpy(buf,t_data->data,t_data->transport_payload_length);
        len = t_data->transport_payload_length;
        rte_free(t_data->data);
        rte_free(t_data);

        return len;
    }
}

/**
 * @brief send data
 * @param sockfd           待查找的唯一标识符
 * @param buf
 * @param len
 * @param flags
 * @param dest_addr
 *
 * @return
 *     -<em>成功or失败</em>
 */
unsigned long sendto_bypass(int sockfd, const void *buf, unsigned long len, __attribute__((unused))  int flags,
                  const struct sockaddr *dest_addr, __attribute__((unused))  socklen_t addrlen){
    struct localhost* host_socket = getHostInfoFromFD(sockfd);
    if(host_socket == NULL){
        return -1;
    }

    struct TransportRecvBufSingleData* t_data = rte_malloc("udp send data",sizeof (struct TransportRecvBufSingleData),0);
    if (t_data == NULL) return -1;


    const struct sockaddr_in *daddr = (const struct sockaddr_in *)dest_addr;
    t_data->dstIP = daddr->sin_addr.s_addr;
    t_data->dstPort = daddr->sin_port;
    t_data->srcIP = host_socket->localIP;
    t_data->srcPort = host_socket->localPort;
    t_data->transport_payload_length = len;

    t_data->data = rte_malloc("udp send buf",len,0);
    if(t_data->data == NULL) {
        rte_free(t_data);
        return -1;
    }
    rte_memcpy(t_data->data,buf,len);
    //不需要阻塞同步，否则会和主线程形成死锁
    rte_ring_mp_enqueue(host_socket->send_buffer,t_data);

    //print_addr仅仅用于输出
    struct in_addr print_addr;
    memset(&print_addr,0,sizeof (struct in_addr));
    print_addr.s_addr = htonl(daddr->sin_addr.s_addr);
    //printf("UDP DATA Send To ---> IP: %s PORT: %d \n", inet_ntoa(print_addr), t_data->dstPort);
    return len;
}

/**
 * @brief socket close
 * @param sockfd           socket唯一标识符
 * @return
 *     -<em>成功or失败</em>
 */
int close_bypass(int sockfd){
    struct localhost* host = getHostInfoFromFD(sockfd);
    if (host == NULL) return -1;
    struct localhost_table* localhost_list = get_host_list_instance();
    REMOVE_UDP_SOCKET_ENTRY(localhost_list->entry,host);

    if (host->recv_buffer){
        rte_ring_free(host->recv_buffer);
    }
    if (host->send_buffer){
        rte_ring_free(host->send_buffer);
    }

    rte_free(host);
}

/**
 * @brief 将协议栈中的数据包解析并放入udp处理线程的接收队列
 * @param iphdr                 ip头
 * @param recv_msg              mbuf形式的协议栈缓冲数据
 *
 * @return
 *     -<em>成功or失败</em>
 */
int UDPRecvProcess(struct IPv4Header iphdr,unsigned char* recv_msg){
/**协议栈中到udp层前的数据包解析，然后将udp数据直接扔到环形缓冲区*/
    struct UDPHeader udphdr = TransUDPDisassemble(recv_msg);
    //两个字节以上都要转网络字节序(已经解析过了，不用转了)
    unsigned short length = udphdr.len;
    struct in_addr addr_src, addr_dst;
    addr_src.s_addr = htonl(iphdr.srcIP);
    addr_dst.s_addr = htonl(iphdr.dstIP);
    recv_msg = recv_msg + sizeof(struct UDPHeader);
    if (udphdr.dstPort == 8889){
        printf("UDP DATA: src ip: %s dst ip: %s src_port:%d dst_port:%d,payload length-->%d %s \n",
               inet_ntoa(addr_src), inet_ntoa(addr_dst), udphdr.srcPort, udphdr.dstPort, length - 8,
               (recv_msg));
    }

    /**关键点：从全局套接字列表中找到是否有这个套接字，和之前判断的端口一样，如果没有就丢弃*/
    struct localhost* host_socket = getHostInfoFromPortAndIP(iphdr.dstIP,udphdr.dstPort,iphdr.protocol);
    if (host_socket ==NULL){
        //rte_free(recv_msg);//在返回后有pktmbuf_free，这里是否也要释放缓冲区？？？
        return -1;//不是出错，而是执行混杂模式时的广播包直接不处理，返回-1
    }

    struct TransportRecvBufSingleData* t_recv_data = rte_malloc("TransportRecvBufSingleData",sizeof (struct TransportRecvBufSingleData),0);
    if (t_recv_data == NULL){
        //rte_free(recv_msg);//在返回后有pktmbuf_free，这里是否也要释放缓冲区？？？
        return -2;//申请准备放如udp接收缓冲区的指针失败
    }
    memset(t_recv_data,0,sizeof (struct TransportRecvBufSingleData));

    t_recv_data->srcIP = iphdr.srcIP;
    t_recv_data->dstIP = iphdr.dstIP;
    t_recv_data->srcPort = udphdr.srcPort;
    t_recv_data->dstPort = udphdr.dstPort;

    t_recv_data->protocol = IPPROTO_UDP;
    t_recv_data->transport_payload_length = udphdr.len - sizeof (struct rte_udp_hdr);

    t_recv_data->data = rte_malloc("transport layer payload",t_recv_data->transport_payload_length,0);
    if (t_recv_data->data == NULL){
        //rte_free(recv_msg);//在返回后有pktmbuf_free，这里是否也要释放缓冲区？？？
        return -2;
    }
    memset(t_recv_data->data,0,t_recv_data->transport_payload_length);
    memcpy(t_recv_data->data, recv_msg,t_recv_data->transport_payload_length);
    //入该套接字接收队列
    rte_ring_mp_enqueue(host_socket->recv_buffer,t_recv_data);//和burst有什么区别？？

    //条件变量实现同步，通知udp处理线程，接收队列有值
    pthread_mutex_lock(&(host_socket->mutex));
    pthread_cond_signal(&host_socket->cond);
    pthread_mutex_unlock(&(host_socket->mutex));

    //rte_free(recv_msg);//在返回后有pktmbuf_free，这里是否也要释放缓冲区？？？
    return 0;
}
/**
 * @brief 将协议栈中的数据包解析并放入udp处理线程的接收队列
 * @param iphdr                 ip头
 * @param recv_msg              mbuf形式的协议栈缓冲数据
 *
 * @return
 *     -<em>成功or失败</em>
 */
int UDPSendProcess(struct rte_mempool* mbuf_pool, struct rx_tx_queue* ring_buffer){
    struct localhost* host;
    struct localhost_table* lhost_list = get_host_list_instance();
    //遍历处理所有socket
    for (host = lhost_list->entry;host!= NULL;host = host->next){
        struct TransportRecvBufSingleData* send_data;
        int sendPacket = rte_ring_mc_dequeue(host->send_buffer,(void**)&send_data);//这里同样没有加引用
        if (sendPacket<0) continue;

        struct in_addr addr;
        addr.s_addr = htonl(send_data->dstIP);
        //printf("UDP SEND DATA TO ---> DSTIP %s PORT %d \n", inet_ntoa(addr), send_data->dstPort);

        unsigned char* dst_mac = arp_find_dst_mac_addr(send_data->dstIP);
        if (dst_mac == NULL){
            //send arp and continue
            struct rte_mbuf* arp_send_buf =  ARPSend(mbuf_pool, gDpdkPortID,broadcast_mac,
                    default_mac, localHostIP, send_data->dstIP, RTE_ARP_OP_REQUEST);
            rte_ring_mp_enqueue_burst(ring_buffer->tx_queue,(void**)&arp_send_buf,1,NULL);//这里同样没有加引用

            //刚才的数据放回udp发送队列
            rte_ring_mp_enqueue(host->send_buffer,send_data);
        }else{

            struct rte_mbuf *send_data_mbuf = UDPSend(mbuf_pool, gDpdkPortID, (send_data->data), send_data->transport_payload_length,
                                                      (unsigned char *) dst_mac,
                                                      send_data->srcIP, send_data->dstIP, send_data->srcPort, send_data->dstPort);
            rte_ring_mp_enqueue_burst(ring_buffer->tx_queue,(void**)&send_data_mbuf,1,NULL);
        }

    }
    return 0;
}
#endif //DPDKLEARNING_UDPANDTCPAPI_H
