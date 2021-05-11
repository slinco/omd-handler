#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <iostream>
#include <string>
#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
#include "Settings.hpp"
#include "UdpClient.cpp"
#include <thread>

using boost::shared_ptr;
using boost::make_shared;

std::string omd::Settings::CfgFile = "conf/omd.xml";
std::string app_version = "v20200611";

void initLogger(std::string name, std::string logfilename, std::string format, bool isDebug = false)
{
    spdlog::flush_every(std::chrono::seconds(1));

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
//    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logfilename, 23, 59);
    auto rotate_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logfilename, 1048576 * 50, 100);
    auto logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list({console_sink, rotate_sink}));
    logger->set_pattern(format);

    if(isDebug)
      logger->set_level(spdlog::level::debug);
    else
      logger->set_level(spdlog::level::info);

    spdlog::register_logger(logger);
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "syntax: omd [cfg file]\n";
        return -1;
    }

    if (argc > 1) omd::Settings::CfgFile = argv[1];
    try {
      auto &settings = omd::Settings::GetInstance();

      initLogger("main", settings.getLogFileName(), settings.getLogFormat(), settings.isLogDebug());
      auto logger = spdlog::get("main");
      logger->info("...........  Program Start {} ...........", app_version);

      //init loggers
      auto loggermap = settings.getLoggerMap();
      for(std::map<std::string, omd::Settings::Logger>::iterator iter = loggermap.begin(); iter != loggermap.end(); iter++) {
        omd::Settings::Logger logger = iter->second;
        initLogger(logger.name, logger.filename, logger.format, logger.isDebug);
      }

      auto udpmap = settings.getChannelMap();
      std::vector<shared_ptr<omd::UdpClient>> vUdpClient;

      for(std::map<std::string, omd::Settings::Udp>::iterator iter = udpmap.begin(); iter != udpmap.end(); iter++) {
          omd::Settings::Udp udp = iter->second;
          logger->info("[Settings] channel - udp_name:{}, isActive:{}, isRecvOnly:{}, nic:{}, host:{}, port:{}, logger:{}", udp.name, udp.isActive, udp.isRecvOnly, udp.nic, udp.host, udp.port, udp.logger);
          boost::shared_ptr<omd::UdpClient> _udpClient = boost::make_shared<omd::UdpClient>(udp.logger, udp.nic, udp.port, udp.host);
          _udpClient->start(udp.isRecvOnly);
          vUdpClient.push_back(_udpClient);
      }

      for (unsigned int i = 0; i < vUdpClient.size(); ++i)
      {
          vUdpClient[i]->join();
      }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
