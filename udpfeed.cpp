#define ELPP_NO_DEFAULT_LOG_FILE
#include "utils/easylogging++.h"
#include "utils/INIReader.h"

#include <chrono>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <unistd.h>

using namespace std;
using namespace std::chrono;

INITIALIZE_EASYLOGGINGPP

string app_id = "udpfeed";
string app_version = "v20200306";
string app_nic = "127.0.0.1";
string app_log_conf = "log.conf";
int    app_feed_sleep = 0;     //us
int    app_repeat = 0;

//multicast target
string mcast_host = "224.0.0.0";
int    mcast_port = 1000;

const char* log_inifile("log.conf");
const char* app_inifile("udpfeed.ini");

#pragma pack(push, 1)
struct header_t {
    uint16_t pktSize;
    uint8_t  msgCount;
    char     filler[1];
    uint32_t seqNum;
    uint64_t sendTime;
};

#pragma pack(pop)


void load_ini(const char* file) {
    INIReader reader(file);

    if (reader.ParseError() < 0) {
        LOG(ERROR) << "Can't load " << file;
    }

    app_id = reader.Get("app", "id", "udpfeed");
    app_nic = reader.Get("app", "nic", "127.0.0.1");
    app_log_conf  = reader.Get("app", "log_conf", "log.conf");
    app_feed_sleep =  reader.GetInteger("app", "feed_sleep", 20);
    app_repeat =  reader.GetInteger("app", "repeat", 0);

    mcast_host = reader.Get("multicast", "host", "224.0.0.0");
    mcast_port = reader.GetInteger("multicast", "port", 1000);
}

void initLog(const char* initFile)
{
    /*** init log ***/
    // Load configuration from file
    el::Configurations conf(initFile);
    // Reconfigure single logger
    el::Loggers::reconfigureLogger("default", conf);
    // Actually reconfigure all loggers instead
    el::Loggers::reconfigureAllLoggers(conf);
}

uint64_t millis()
{
    uint64_t ms = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
    return ms;
}

// Get time stamp in microseconds.
uint64_t micros()
{
    uint64_t us = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
    return us;
}

// Get time stamp in nanoseconds.
uint64_t nanos()
{
    uint64_t ns = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
    return ns;
}

bool char_array_is_zero(char array[], std::ptrdiff_t length) {
    return std::all_of(array, array+length,
                       [](char x) { return x == '\0'; });
}

const int BUFFER_LENGTH = 65507;
int main(int argc, char *argv[])
{
    if (argc < 3) {
        // Tell the user how to run the program
        std::cerr << "Usage: " << argv[0] << " <config> <input file>" << std::endl;
        /* "Usage messages" are a conventional way of telling the user
         * how to run a program if they enter the command incorrectly.
         */
        return 1;
    }

    load_ini(argv[1]);
    initLog(app_log_conf.c_str());

    LOG(INFO) << "........... Program " << app_id << " Start "<< app_version << " ...........";
    LOG(INFO) << "config[app]:app_id=" << app_id << ";app_nic=" << app_nic
              << ";app_log_conf=" << app_log_conf << ";app_feed_sleep=" << app_feed_sleep
              << ";app_repeat=" << app_repeat << ";READ_BUFFER_LENGTH=" << BUFFER_LENGTH;
    LOG(INFO) << "config[multicast]:mcast_host=" << mcast_host << ";mcast_port=" << mcast_port;

    ifstream inFile;
    size_t size = 0;

    //open file
    inFile.open( argv[2], std::ifstream::in|std::ifstream::binary );

    if(!inFile){
        return 1;
    }

    //init udp socket
    // create what looks like an ordinary UDP socket
    //
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("socket multicast!");
        return 1;
    }

    // set up destination address
    //
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(mcast_host.c_str());
    addr.sin_port = htons(mcast_port);

    char    readBuf[BUFFER_LENGTH] = {};

    const int checkCount = 10000;
    int       logCountArray[50] = {0};
    long      seqArray[50] = {0};

    int repeat = app_repeat;
    int line = 1;
    int cached = 0;
    char* pReader = readBuf;
    //用!fin.eof()來判斷，最後會多讀一次
    milliseconds ms_start = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch()
    );

    long count = 0;
    uint64_t lastCheck = millis();

    LOG(INFO) << "Messages,Total Execution Time(ms),Execution Time Per Msg(us/msg)";
    while( inFile.peek()!=EOF ){
        header_t *header_ = {};
        inFile.read(readBuf, BUFFER_LENGTH);
        count++;
        header_ = reinterpret_cast<header_t*>(readBuf);
//        if(header_->msgCount != 0){
            int nbytes = sendto(
                    socket_fd,
                    readBuf,
                    BUFFER_LENGTH,
                    0,
                    (struct sockaddr*) &addr,
                    sizeof(addr)
            );
//        }


        usleep(app_feed_sleep);
        if((line % checkCount) == 0) {
        	milliseconds ms_end = duration_cast< milliseconds >(
                    system_clock::now().time_since_epoch()
            );
            int ms_exection =  (ms_end - ms_start).count();
            double us_exection_avg = 1000*ms_exection/checkCount;

            LOG(INFO) << checkCount << "," << ms_exection << "," << us_exection_avg;
            ms_start = duration_cast< milliseconds >(
                    system_clock::now().time_since_epoch()
            );
            line = 0;
            cached = 0;
        }

        line++;
        if(inFile.peek() == EOF && repeat != 0){
            --repeat;
            inFile.clear();                 // clear fail and eof bits
            inFile.seekg(0, std::ios::beg);
        }
    }
    inFile.close();
    LOG(INFO) << "total count:" << count;
}