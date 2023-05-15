/**
* @File name: Header.h
* @Author: Martinal
* @Version: -
* @Description: protocol header file
* */


#ifndef DPDKLEARNING_HEADER_H
#define DPDKLEARNING_HEADER_H
#pragma pack(1)

/**
 * pcap文件头结构体
 * */
typedef struct pcapFileHeader
{
    unsigned int magic;       /* 0xa1b2c3d4 0xA1 B2 C3 D4:用来标示文件的开始 */
    unsigned short version_major;   /* 当前文件主要的版本号 Version 2 */
    unsigned short version_minor;   /* 当前文件次要的版本号 Version 4 */
    unsigned int thisZone;      /* 4B当地的标准时间 */
    unsigned int sigFigs;     /* 4B时间戳的精度 */
    unsigned int snapLen;     /* 4B最大的存储长度 */
    /** 常用链路类型
0            BSD loopback devices, except for later OpenBSD
1            Ethernet, and Linux loopback devices
6            802.5 Token Ring
7            ARCnet
8            SLIP
9            PPP
10           FDDI
100         LLC/SNAP-encapsulated ATM
101         “raw IP”, with no link
102         BSD/OS SLIP
103         BSD/OS PPP
104         Cisco HDLC
105         802.11
108         later OpenBSD loopback devices (with the AF_value in network byte order)
113         special Linux “cooked” capture
114         LocalTalk
     * */
    unsigned int linkType;    /* 4B链路类型 */
}pcapFileHeader;

//时间戳
struct timeStamp
{
    unsigned int tv_sec;         /* seconds 含义同 time_t 对象的值 */
    unsigned int tv_usec;        /* and microseconds */
}timeStamp;

//pcap数据包头结构体
struct pcapHeader
{
    struct timeStamp ts;  /* 时间戳 */
    unsigned int capLen;   /* 当前数据区的长度，即抓取到的数据帧长度 */
    unsigned int len;      /* 离线数据长度：网络中实际数据帧的长度，一般不大于capLen，多数情况下和capLen数值相等 */
};

/**
 * 数据链路层
 * */
//以太网帧头（802.3）
struct ethernetHeader
{
    unsigned char dstMAC[6]; //目的MAC地址
    unsigned char  srcMAC[6]; //源MAC地址
    unsigned short  frameType;    //帧类型
}ethernetHeader;

/**
 * 网络层
 * */
//IPv4包头
struct IPv4Header
{
    unsigned char  verHLen;       //版本+报头长度
    unsigned char  tos;            //服务类型
    unsigned short totalLen;       //总长度
    unsigned short id; //标识
    unsigned short flagSegment;   //标志+片偏移
    unsigned char  ttl;            //生存周期
    unsigned char  protocol;       //协议类型
    unsigned short checksum;       //头部校验和
    unsigned int srcIP; //源IP地址
    unsigned int dstIP; //目的IP地址
}IPv4Header;

// icmp报文头
struct ICMPHeader
{
    unsigned char  type;       //类型
    unsigned char  code;          //代码（网络层协议号）
    unsigned short   crc;        //校验和
    unsigned short   id;       //与request的id一致
    unsigned short  seq;       //序号 与request一致(若是request则每次加1)

}ICMPHeader;

//arp头
struct ARPHeader
{
    unsigned short  hardwareType;       //硬件类型（数据链路层协议）
    unsigned short  protoType;          //协议类型（网络层协议号）
    unsigned char   hardAddrLen;        //硬件地址长度
    unsigned char   protoAddrLen;       //协议地址长度
    unsigned short  operation;          //1 请求报文  2 响应报文
    unsigned char srcMAC[6]; //目的MAC地址  //源站MAC
    unsigned int  srcIPAddr;     //源站IP地址
    unsigned char  dstMAC[6];    //目的站物理地址
    unsigned int    dstIPAddr;          //目的站IP地址
}ARPHeader;

/**
 * 传输层
 * */

//TCP数据报头
struct TCPHeader
{
    unsigned short srcPort; //源端口
    unsigned short dstPort; //目的端口
    unsigned int seqNo; //序号
    unsigned int ackNo; //确认号
    unsigned char headerLen; //数据报头的长度(4 bit) + 保留(4 bit)
    unsigned char flags; //标识TCP不同的控制消息
    unsigned short window; //窗口大小
    unsigned short checksum; //校验和
    unsigned short urgentPointer;  //紧急指针
}TCPHeader;

//UDP数据
struct UDPHeader
{
    unsigned short srcPort;     // 源端口号16bit
    unsigned short dstPort;    // 目的端口号16bit
    unsigned short len;        // 数据包长度16bit
    unsigned short checkSum;   // 校验和16bit
}UDPHeader;

/**
 * 应用层
 * */
//DNS数据解析
//http数据解析

#pragma pack()

#endif //DPDKLEARNING_HEADER_H
