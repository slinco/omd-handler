
#include <stdint.h>
#include <stdlib.h>
#include "common.h"
#include <stdio.h>
#include <iostream>
#include <chrono>
#include "latency.hpp"

static int s_fd_max = 0;

#ifdef ST_TEST
int prepare_socket(int fd, struct fds_data *p_data, bool stTest = false);
#else
int prepare_socket(int fd, struct fds_data *p_data);
#endif
void cleanup();

#ifndef VERSION
#define VERSION "1.0"
#endif

os_mutex_t _mutex;
//------------------------------------------------------------------------------
void cleanup() {
    os_mutex_lock(&_mutex);
    int ifd;
    if (g_fds_array) {
        for (ifd = 0; ifd <= s_fd_max; ifd++) {
            if (g_fds_array[ifd]) {
                close(ifd);
                if (g_fds_array[ifd]->active_fd_list) {
                    FREE(g_fds_array[ifd]->active_fd_list);
                }
                if (g_fds_array[ifd]->recv.buf) {
                    FREE(g_fds_array[ifd]->recv.buf);
                }
                if (g_fds_array[ifd]->is_multicast) {
                    FREE(g_fds_array[ifd]->memberships_addr);
                }
                FREE(g_fds_array[ifd]);
            }
        }
    }

    if (s_user_params.select_timeout) {
        FREE(s_user_params.select_timeout);
    }
#ifdef USING_VMA_EXTRA_API
    if (g_vma_api && s_user_params.is_vmazcopyread) {
        zeroCopyMap::iterator it;
        while ((it = g_zeroCopyData.begin()) != g_zeroCopyData.end()) {
            delete it->second;
            g_zeroCopyData.erase(it);
        }
    }
#endif

    if (g_fds_array) {
        FREE(g_fds_array);
    }

    if (NULL != g_pPacketTimes) {
        delete g_pPacketTimes;
        g_pPacketTimes = NULL;
    }
    os_mutex_unlock(&_mutex);
}

//------------------------------------------------------------------------------
/* set the timeout of select*/
static void set_select_timeout(int time_out_msec) {
    if (!s_user_params.select_timeout) {
        s_user_params.select_timeout = (struct timeval *)MALLOC(sizeof(struct timeval));
        if (!s_user_params.select_timeout) {
            log_err("Failed to allocate memory for pointer select timeout structure");
            exit_with_log(SOCKPERF_ERR_NO_MEMORY);
        }
    }
    if (time_out_msec >= 0) {
        // Update timeout
        s_user_params.select_timeout->tv_sec = time_out_msec / 1000;
        s_user_params.select_timeout->tv_usec =
                1000 * (time_out_msec - s_user_params.select_timeout->tv_sec * 1000);
    } else {
        // Clear timeout
        FREE(s_user_params.select_timeout);
    }
}

int sock_set_accl(int fd) {
    int rc = SOCKPERF_ERR_NONE;
    if (setsockopt(fd, SOL_SOCKET, 100, NULL, 0) < 0) {
        log_err("setsockopt(100), set sock-accl failed.  It could be that this option is not "
                "supported in your system");
        rc = SOCKPERF_ERR_SOCKET;
    }
    log_msg("succeed to set sock-accl");
    return rc;
}

int sock_set_reuseaddr(int fd) {
    int rc = SOCKPERF_ERR_NONE;
    u_int reuseaddr_true = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_true, sizeof(reuseaddr_true)) < 0) {
        log_err("setsockopt(SO_REUSEADDR) failed");
        rc = SOCKPERF_ERR_SOCKET;
    }
    return rc;
}

#ifndef SO_LL
#define SO_LL 46
#endif
int sock_set_lls(int fd) {
    int rc = SOCKPERF_ERR_NONE;
    if (setsockopt(fd, SOL_SOCKET, SO_LL, &(s_user_params.lls_usecs),
                   sizeof(s_user_params.lls_usecs)) < 0) {
        log_err("setsockopt(SO_LL) failed");
        rc = SOCKPERF_ERR_SOCKET;
    }
    return rc;
}

int sock_set_snd_rcv_bufs(int fd) {
    /*
     * Sets or gets the maximum socket receive buffer in bytes. The kernel
     * doubles this value (to allow space for bookkeeping overhead) when it
     * is set using setsockopt(), and this doubled value is returned by
     * getsockopt().
     */

    int rc = SOCKPERF_ERR_NONE;
    int size = sizeof(int);
    int rcv_buff_size = 0;
    int snd_buff_size = 0;

    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &(s_user_params.sock_buff_size),
                   sizeof(s_user_params.sock_buff_size)) < 0) {
        log_err("setsockopt(SO_RCVBUF) failed");
        rc = SOCKPERF_ERR_SOCKET;
    }
    if (!rc && (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcv_buff_size, (socklen_t *)&size) < 0)) {
        log_err("getsockopt(SO_RCVBUF) failed");
        rc = SOCKPERF_ERR_SOCKET;
    }
    /*
     * Sets or gets the maximum socket send buffer in bytes. The kernel
     * doubles this value (to allow space for bookkeeping overhead) when it
     * is set using setsockopt(), and this doubled value is returned by
     * getsockopt().
     */
    if (!rc && (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &(s_user_params.sock_buff_size),
                           sizeof(s_user_params.sock_buff_size)) < 0)) {
        log_err("setsockopt(SO_SNDBUF) failed");
        rc = SOCKPERF_ERR_SOCKET;
    }
    if (!rc && (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &snd_buff_size, (socklen_t *)&size) < 0)) {
        log_err("getsockopt(SO_SNDBUF) failed");
        rc = SOCKPERF_ERR_SOCKET;
    }
    if (!rc) {
        log_msg("Socket buffers sizes of fd %d: RX: %d Byte, TX: %d Byte", fd, rcv_buff_size,
                snd_buff_size);
        if (rcv_buff_size < s_user_params.sock_buff_size * 2 ||
            snd_buff_size < s_user_params.sock_buff_size * 2) {
            log_msg("WARNING: Failed setting receive or send socket buffer size to %d bytes (check "
                    "'sysctl net.core.rmem_max' value)",
                    s_user_params.sock_buff_size);
        }
    }

    return rc;
}

int sock_set_tcp_nodelay(int fd) {
    int rc = SOCKPERF_ERR_NONE;
    if (s_user_params.tcp_nodelay) {
        /* set Delivering Messages Immediately */
        int tcp_nodelay = 1;
        if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&tcp_nodelay, sizeof(tcp_nodelay)) <
            0) {
            log_err("setsockopt(TCP_NODELAY)");
            rc = SOCKPERF_ERR_SOCKET;
        }
    }
    return rc;
}

int sock_set_tos(int fd) {
    int rc = SOCKPERF_ERR_NONE;
    if (s_user_params.tos) {
        socklen_t len = sizeof(s_user_params.tos);
        if (setsockopt(fd, IPPROTO_IP, IP_TOS, (char *)&s_user_params.tos, len) < 0) {
            log_err("setsockopt(TOS), set  failed.  It could be that this option is not supported "
                    "in your system");
            rc = SOCKPERF_ERR_SOCKET;
        }
    }
    return rc;
}

int sock_set_multicast(int fd, struct fds_data *p_data) {
    int rc = SOCKPERF_ERR_NONE;
    struct sockaddr_in *p_addr = NULL;
    p_addr = &(p_data->server_addr);

    /* use setsockopt() to request that the kernel join a multicast group */
    /* and specify a specific interface address on which to receive the packets of this socket */
    /* and may specify message source IP address on which to receive from */
    /* NOTE: we don't do this if case of client (sender) in stream mode */
    if (!s_user_params.b_stream || s_user_params.mode != MODE_CLIENT) {
        if (p_data->mc_source_ip_addr.s_addr != INADDR_ANY) {
            struct ip_mreq_source mreq_src;
            memset(&mreq_src, 0, sizeof(struct ip_mreq_source));
            mreq_src.imr_multiaddr = p_addr->sin_addr;
            mreq_src.imr_interface.s_addr = s_user_params.rx_mc_if_addr.s_addr;
            mreq_src.imr_sourceaddr.s_addr = p_data->mc_source_ip_addr.s_addr;
            if (setsockopt(fd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq_src, sizeof(mreq_src)) <
                0) {
                if (errno == ENOBUFS) {
                    log_err("setsockopt(IP_ADD_SOURCE_MEMBERSHIP) - Maximum multicast source "
                            "addresses that can be filtered is limited by "
                            "/proc/sys/net/ipv4/igmp_max_msf");
                } else {
                    log_err("setsockopt(IP_ADD_SOURCE_MEMBERSHIP)");
                }
                rc = SOCKPERF_ERR_SOCKET;
            }
        } else {
            struct ip_mreq mreq;
            memset(&mreq, 0, sizeof(struct ip_mreq));
            mreq.imr_multiaddr = p_addr->sin_addr;
            mreq.imr_interface.s_addr = s_user_params.rx_mc_if_addr.s_addr;
            if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                if (errno == ENOBUFS) {
                    log_err("setsockopt(IP_ADD_MEMBERSHIP) - Maximum multicast addresses that can "
                            "join same group is limited by "
                            "/proc/sys/net/ipv4/igmp_max_memberships");
                } else {
                    log_err("setsockopt(IP_ADD_MEMBERSHIP)");
                }
                rc = SOCKPERF_ERR_SOCKET;
            }
        }
    }

    /* specify a specific interface address on which to transmitted the multicast packets of this
     * socket */
    if (!rc && (s_user_params.tx_mc_if_addr.s_addr != INADDR_ANY)) {
        if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &s_user_params.tx_mc_if_addr,
                       sizeof(s_user_params.tx_mc_if_addr)) < 0) {
            log_err("setsockopt(IP_MULTICAST_IF)");
            rc = SOCKPERF_ERR_SOCKET;
        }
    }

    if (!rc && (s_user_params.mc_loop_disable)) {
        /* disable multicast loop of all transmitted packets */
        u_char loop_disabled = 0;
        if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop_disabled, sizeof(loop_disabled)) <
            0) {
            log_err("setsockopt(IP_MULTICAST_LOOP)");
            rc = SOCKPERF_ERR_SOCKET;
        }
    }

    if (!rc) {
        /* the IP_MULTICAST_TTL socket option allows the application to primarily
         * limit the lifetime of the packet in the Internet and prevent it from
         * circulating
         */
        int value = s_user_params.mc_ttl;
        if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&value, sizeof(value)) < 0) {
            log_err("setsockopt(IP_MULTICAST_TTL)");
            rc = SOCKPERF_ERR_SOCKET;
        }
    }

    return rc;
}

//------------------------------------------------------------------------------
/* returns the new socket fd
 or exit with error code */
#ifdef ST_TEST
int prepare_socket(int fd, struct fds_data *p_data, bool stTest = false)
#else
int prepare_socket(int fd, struct fds_data *p_data)
#endif
{
    int rc = SOCKPERF_ERR_NONE;

    if (!p_data) {
        return (int)INVALID_SOCKET; // TODO: use SOCKET all over the way and avoid this cast
    }

    if (!rc && !s_user_params.is_blocked) {
        /*Uncomment to test FIONBIO command of ioctl
         * int opt = 1;
         * ioctl(fd, FIONBIO, &opt);
         */

        /* change socket to non-blocking */
        if (os_set_nonblocking_socket(fd)) {
            log_err("failed setting socket as nonblocking\n");
            rc = SOCKPERF_ERR_SOCKET;
        }
    }

    if (!rc && (s_user_params.withsock_accl == true)) {
        rc = sock_set_accl(fd);
    }

    if (!rc && ((p_data->is_multicast) || (s_user_params.uc_reuseaddr)) &&
        ((s_user_params.mode == MODE_SERVER && p_data->server_addr.sin_port) ||
         (s_user_params.mode == MODE_CLIENT && s_user_params.client_bind_info.sin_port))) {
        /* allow multiple sockets to use the same PORT (SO_REUSEADDR) number
         * only if it is a well know L4 port only for MC or if uc_reuseaddr parameter was set.
         */
        rc = sock_set_reuseaddr(fd);
    }

    if (!rc && (s_user_params.lls_is_set == true)) {
        rc = sock_set_lls(fd);
    }

    if (!rc && (s_user_params.sock_buff_size > 0)) {
        rc = sock_set_snd_rcv_bufs(fd);
    }

    if (!rc && (p_data->is_multicast)) {
        rc = sock_set_multicast(fd, p_data);
    }

    if (!rc && (p_data->sock_type == SOCK_STREAM)) {
        rc = sock_set_tcp_nodelay(fd);
    }

    if (!rc && (s_user_params.tos)) {
        rc = sock_set_tos(fd);
    }

#ifdef USING_VMA_EXTRA_API
    #ifdef ST_TEST
    if (!stTest)
#endif
        if (!rc && (s_user_params.is_vmarxfiltercb && g_vma_api)) {
            // Try to register application with VMA's special receive notification callback logic
            if (g_vma_api->register_recv_callback(fd, myapp_vma_recv_pkt_filter_callback, NULL) <
                0) {
                log_err("vma_api->register_recv_callback failed. Try running without option "
                        "'vmarxfiltercb'");
            } else {
                log_dbg("vma_api->register_recv_callback successful registered");
            }
        } else if (!rc && (s_user_params.is_vmazcopyread && g_vma_api)) {
            g_zeroCopyData[fd] = new ZeroCopyData();
            g_zeroCopyData[fd]->allocate();
        }
#endif

    return (!rc ? fd
                : (int)INVALID_SOCKET); // TODO: use SOCKET all over the way and avoid this cast
}

int initUDPSource(const std::string& nic, int listen_port, const std::string& group_ip){
    struct sockaddr_in localSock;
    struct ip_mreq     group;
    /* Create a datagram socket on which to receive. */
    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0){
        perror("Opening datagram socket error");
        exit(1);
    }else{
        std::cout << "Opening datagram socket....OK." << std::endl;
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
            std::cout << "Setting SO_REUSEADDR...OK." << std::endl;
        }
    }

    /* Bind to the proper port number with the IP address */
    /* specified as INADDR_ANY. */
    memset((char *) &localSock, 0, sizeof(localSock));
    localSock.sin_family = AF_INET;
    localSock.sin_port = htons(listen_port);
    localSock.sin_addr.s_addr = INADDR_ANY;

    if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock))){
        perror("Binding datagram socket error");
        close(sd);
        exit(1);
    }else {
        std::cout << "Binding datagram socket...OK." << std::endl;
    }

    /* Join the multicast group 226.1.1.1 on the local 203.106.93.94 */
    /* interface. Note that this IP_ADD_MEMBERSHIP option must be */
    /* called for each local interface over which the multicast */
    /* datagrams are to be received. */
    group.imr_multiaddr.s_addr = inet_addr(group_ip.c_str());
    group.imr_interface.s_addr = inet_addr(nic.c_str());
    if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0){
        perror("Adding multicast group error");
        close(sd);
        exit(1);
    }else{
        std::cout << "Adding multicast group...OK." << std::endl;
    }
    return sd;
}

int ping_udp(const std::string& nic, const std::string& group_ip, int port, int size, size_t count)
{
    std::cout << "Ping " << group_ip << " * " << count << " ..." << std::endl;
    std::cout << "port :  " << port << std::endl;
    std::cout << "buffer size : " << size << std::endl;
    std::cout << "count :  " << count << std::endl;

    auto l = latency<(std::uint64_t)1 * 1000 * 1000 * 1000>();

    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0)
    {
        perror("Opening datagram socket error");
        exit(1);
    }else{
        std::cout << "Opening datagram socket....OK." << std::endl;
    }

    /* Initialize the group sockaddr structure with a */
    /* group address of 225.1.1.1 and port 5555. */
    struct sockaddr_in groupSock;
    memset((char *) &groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr(group_ip.c_str());
    groupSock.sin_port = htons(port);
    socklen_t groupSock_len = sizeof(groupSock);

    /* Set local interface for outbound multicast datagrams. */
    /* The IP address specified must be associated with a local, */
    /* multicast capable interface. */
    struct in_addr localInterface;
    localInterface.s_addr = inet_addr(nic.c_str());
    if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
    {
        perror("Setting local interface error");
        exit(1);
    }else{
        std::cout <<"Setting the local interface...OK" << std::endl;
    }
    char data[size];
    auto clock = std::chrono::high_resolution_clock{};

    for(size_t i = 0; i < count; i++)
    {

        bool warm = i >= count/2;
        auto start = clock.now();
        sendto(sd, data, sizeof(data), 0, (struct sockaddr*) &groupSock, sizeof(groupSock));
        if(recvfrom(sd, data, sizeof(data), 0, (struct sockaddr *)&groupSock, &groupSock_len) == size){
            if(warm){
                l.add((clock.now() - start));
            }
        }else{
            perror("recvfrom err!");
            exit(1);
        }
    }

    l.generate(std::cout, true);
    close(sd);
    return 0;
}

int pong_udp(const std::string& nic, int listen_port, const std::string& group_ip)
{
    std::cout << "Server Startup ..." << std::endl;
    std::cout << "NIC : " << nic << std::endl;
    std::cout << "Listening Port : " << listen_port << std::endl;
    std::cout << "Group IP : " << group_ip << std::endl;

    int sd = initUDPSource(nic, listen_port, group_ip);

    char data[65507];
    struct sockaddr_in other;
    socklen_t          other_len = sizeof(other);
    while(true)
    {
        ssize_t n = recvfrom(sd, data, sizeof(data), 0,(struct sockaddr *)&other, &other_len);
        if (n != -1)
            sendto(sd, data, n, 0, (struct sockaddr*)&other, other_len);

    }
    close(sd);
    return 0;
}

int main(int argc, char *argv[]) {if(argc < 2){
        std::cerr << "Usage: " << argv[0] << " -client <NIC> <group ip> <group port> <buffer size> <count>" << std::endl;
        std::cerr << "Usage: " << argv[0] << " -server <NIC> <listen port> <group ip>" << std::endl;
        return 1;
    }

    //set_low_latency();
    //load vma
    bool success = vma_set_func_pointers(false);
    if (!success) {
        std::cerr << "Failed to set function pointers for system functions." << std::endl;
        std::cerr << "Check vma-redirect.cpp for functions which your OS implementation is missing. "
                     "Re-compile sockperf without them." << std::endl;
    }

    auto mode = std::string(argv[1]);
    if(mode == "-client")
    {
        if(argc < 6){
            std::cerr << "Usage: " << argv[0] << " -client <NIC> <group ip> <group port> <buffer size> <count>" << std::endl;
            return 1;
        }

        auto nic =  std::string(argv[2]);
        auto group_ip = std::string(argv[3]);
        auto group_port = atoi(argv[4]);
        auto buffer_size = atoi(argv[5]);
        auto count = atol(argv[6]);

        return ping_udp(nic, group_ip, group_port, buffer_size, count);
    }
    else if (mode == "-server")
    {
        if(argc < 5){
            std::cerr << "Usage: " << argv[0] << " -server <NIC> <listen port> <group ip>" << std::endl;
            return 1;
        }
        auto nic = std::string(argv[2]);
        auto listen_port = atoi(argv[3]);
        auto group_ip = std::string(argv[4]);

        return pong_udp(nic, listen_port, group_ip);
    }
}