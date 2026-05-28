#include <stdio.h>
#include <stdlib.h>  // malloc(), free(), exit()
#include <unistd.h>  // read(), write(), close()
#include <string.h>

#include <sys/socket.h>  // socket(), bind(), recvfrom(), sendto()
#include <arpa/inet.h>  // htons()

#include <linux/if_packet.h>  // defines AF_PACKET (for capturing packets)

#include <net/ethernet.h>  // includes struct ethhdr -> containing MAC, EtherType
#include <netinet/ip.h>  // includes struct iphdr -> containing IP, protocol, TTL, packet length
#include <netinet/tcp.h>  // includes struct tcphdr -> containing Port, SYN/ACK flags, sdequence numbers
#include <netinet/udp.h>  // includes struct udphdr

#include <time.h>

#define MAX_CONNECTIONS 5000

void error(char* msg){
    perror(msg);
    exit(1);
}

struct connection{
    char src_IP[20];
    char dst_IP[20];

    unsigned short src_port;
    unsigned short dst_port;

    unsigned char protocol_id;

    unsigned long total_pkts;
    unsigned long total_bytes;

    unsigned long incoming_pkts;
    unsigned long outgoing_pkts;

    time_t first_seen;
    time_t last_seen;
};

void process_packet(unsigned char* workspace, int size, struct connection* connection_table, int* connection_count){
    // --------------------- Ethernet header ---------------------
    struct ethhdr* eth = (struct ethhdr*) workspace;
    if(ntohs(eth->h_proto) != ETH_P_IP){
        return;  // Ignore Non-IP packets
    }

    // --------------------- IP header ---------------------
    struct iphdr* ip = (struct iphdr*) (workspace + sizeof(struct ethhdr));
    if(ip->protocol != IPPROTO_TCP && ip->protocol != IPPROTO_UDP){
        return;  // Consider only TCP and UDP packets
    }
    int ip_header_len = ip->ihl * 4;  // Length of the IP header in bytes

    // --------------------- Transport header and Extracting Ports ---------------------
    unsigned short src_port;
    unsigned short dst_port;

    if(ip->protocol == IPPROTO_TCP){
    struct tcphdr* tcp = (struct tcphdr*) (workspace + sizeof(struct ethhdr) + ip_header_len);  // Extracting the TCP header
    src_port = ntohs(tcp->source);
    dst_port = ntohs(tcp->dest);
    }

    if(ip->protocol == IPPROTO_UDP){
    struct udphdr* udp = (struct udphdr*) (workspace + sizeof(struct ethhdr) + ip_header_len);  // Extracting the UDP header
    src_port = ntohs(udp->source);
    dst_port = ntohs(udp->dest);
    }

    // --------------------- Extracting IPs ---------------------
    struct sockaddr_in src, dst;

    src.sin_addr.s_addr = ip->saddr;
    dst.sin_addr.s_addr = ip->daddr;

    char src_ip[20];
    char dst_ip[20];

    strcpy(src_ip, inet_ntoa(src.sin_addr));
    strcpy(dst_ip, inet_ntoa(dst.sin_addr));

    time_t current_time = time(NULL);

    // --------------------- Existing Connection ---------------------
    int found = 0;

    for(int i = 0; i < *connection_count; i++){
        if(
            (
                strcmp(connection_table[i].src_IP, src_ip) == 0 && 
                strcmp(connection_table[i].dst_IP, dst_ip) == 0 && 
                connection_table[i].src_port == src_port &&
                connection_table[i].dst_port == dst_port &&
                connection_table[i].protocol_id == ip->protocol
            )
            ||
            (
                strcmp(connection_table[i].src_IP, dst_ip) == 0 && 
                strcmp(connection_table[i].dst_IP, src_ip) == 0 && 
                connection_table[i].src_port == dst_port &&
                connection_table[i].dst_port == src_port &&
                connection_table[i].protocol_id == ip->protocol
            )
        )
        {
            found = 1;

            connection_table[i].last_seen = current_time;

            connection_table[i].total_pkts += 1;
            connection_table[i].total_bytes += size;

            break;
        }
    }

    // --------------------- New Connection ---------------------

    if(found == 0){
        if(*connection_count >= MAX_CONNECTIONS){
            printf("Connection table is full.\n");
            return;
        }

        int new = *connection_count;

        strcpy(connection_table[new].src_IP, src_ip);
        strcpy(connection_table[new].dst_IP, dst_ip);

        connection_table[new].src_port = src_port;
        connection_table[new].dst_port = dst_port;

        connection_table[new].protocol_id = ip->protocol;

        connection_table[new].total_pkts = 1;
        connection_table[new].total_bytes = size;

        connection_table[new].first_seen = current_time;
        connection_table[new].last_seen = current_time;

        *connection_count += 1;
    }

    // --------------------- Printing Connection Table ---------------------

    unsigned long Total_pkts[3] = {0, 0, 0};  // {Total, TCP, UDP}
    unsigned long Total_bytes[3] = {0, 0, 0};  // {Total, TCP, UDP}

    system("clear");

    printf("========================= PROTOCOL TABLE =========================\n");
    printf("Protocol ID: 6  | TCP\n");
    printf("Protocol ID: 17 | UDP\n\n");

    printf("========================= CONNECTION TABLE =========================\n\n");

    for(int i = 0; i < *connection_count; i++){

        char first[100];
        char last[100];

        strftime(first, sizeof(first), "%d/ %H:%M:%S", localtime(&connection_table[i].first_seen));
        strftime(last, sizeof(last), "%d/ %H:%M:%S", localtime(&connection_table[i].last_seen));

        printf(
            "%d  -  %s:%u ---> %s:%u | Protocol ID: %u | Packets: %lu | Bytes: %lu | First seen: %s | Last seen: %s\n",

            i+1,
            connection_table[i].src_IP,
            connection_table[i].src_port,
            connection_table[i].dst_IP,
            connection_table[i].dst_port,
            connection_table[i].protocol_id,
            connection_table[i].total_pkts,
            connection_table[i].total_bytes,
            first,
            last);
        
        Total_pkts[0] += connection_table[i].total_pkts;
        Total_bytes[0] += connection_table[i].total_bytes;

        if(connection_table[i].protocol_id == IPPROTO_TCP){
            Total_pkts[1] += connection_table[i].total_pkts;
            Total_bytes[1] += connection_table[i].total_bytes;
        }

        if(connection_table[i].protocol_id == IPPROTO_UDP){
            Total_pkts[2] += connection_table[i].total_pkts;
            Total_bytes[2] += connection_table[i].total_bytes;
        }
    }

    printf("\n========================= OVERVIEW BY TCP/UDP =========================\n");
    printf("Total Packets : %lu\n", Total_pkts[0]);
    printf("Total Bytes (kB): %lu\n\n", (Total_bytes[0]/1024));

    printf("TCP Total Packets : %lu\n", Total_pkts[1]);
    printf("TCP Total Bytes (kB): %lu\n\n", (Total_bytes[1]/1024));

    printf("UDP Total Packets : %lu\n", Total_pkts[2]);
    printf("UDP Total Bytes (kB): %lu\n\n", (Total_bytes[2]/1024));


}

int main(){

    int sockfd;
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(sockfd < 0){
        error("socket - Socket creation unsuccessful.");
    }

    unsigned char workspace[65536];
    struct connection connection_table[MAX_CONNECTIONS];
    int connection_count = 0;

    unsigned long Total_pkts = 0;
    unsigned long Total_bytes = 0;

    while(1){
        // struct sockaddr_ll addr;  // For identifying incoming and outgoing packets
        // socklen_t addr_len = sizeof(addr);
        
        int packet_size = recvfrom(sockfd, workspace, sizeof(workspace), 0, NULL, NULL);  // Copy packets from the network stack to the buffer and return the packet_length.
        if(packet_size < 0){
            error("recvfrom - packet reciveing unsuccessful.");
        }

        process_packet(workspace, packet_size, connection_table, &connection_count);  // process the packet layer by layer.
    }

    return 0;
}