#include <arpa/inet.h>
#include <string.h>  // memset()
#include <stdio.h>   // perror()
#include <stdlib.h>  // exit()
#include <unistd.h>
#include <string>

using namespace std;

int initUDPSource(string nic, int port, string multicast_ip){
    struct sockaddr_in localSock;
    struct ip_mreq     group;
    /* Create a datagram socket on which to receive. */
    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0){
        perror("Opening datagram socket error");
        exit(1);
    }else{
        perror("Opening datagram socket....OK.");
    }

    /* Enable SO_REUSEADDR to allow multiple instances of this */
    /* application to receive copies of the multicast datagrams. */
    {
        int reuse = 1;
        if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0){
            perror("Setting SO_REUSEADDR error");
            close(sd);
            exit(1);
        }
        else {
            perror("Setting SO_REUSEADDR...OK.");
        }
    }

    /* Bind to the proper port number with the IP address */
    /* specified as INADDR_ANY. */
    memset((char *) &localSock, 0, sizeof(localSock));
    localSock.sin_family = AF_INET;
    localSock.sin_port = htons(port);
    localSock.sin_addr.s_addr = INADDR_ANY;

    if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock))){
        perror("Binding datagram socket error");
        close(sd);
        exit(1);
    }else {
        perror("Binding datagram socket...OK.");
    }

    /* Join the multicast group 226.1.1.1 on the local 203.106.93.94 */
    /* interface. Note that this IP_ADD_MEMBERSHIP option must be */
    /* called for each local interface over which the multicast */
     /* datagrams are to be received. */
    group.imr_multiaddr.s_addr = inet_addr(multicast_ip.c_str());
    group.imr_interface.s_addr = inet_addr(nic.c_str());
    if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0){
        perror("Adding multicast group error");
        close(sd);
        exit(1);
    }else{
        perror("Adding multicast group...OK.");
    }
    return sd;
}

int main() {


    int fd = initUDPSource("192.168.20.31",51000, "239.1.1.1");

    for (;;) {
        char               buffer[65507];  // Max UDP packet size ((2^16 - 1) - (8 byte UDP header) - (20 byte IP header)).
        struct sockaddr_in other;
        socklen_t          other_len = sizeof(other);

        ssize_t n = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&other, &other_len);

        if (n == -1) {
            perror("recvfrom");
            exit(1);
        }

//        if (sendto(fd, buffer, n, 0, (struct sockaddr*)&other, other_len) != n) {
//            perror("sendto");
//            exit(1);
//        }
    }
}

