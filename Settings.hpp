#pragma once
#ifndef OMD_SETTINGS_HPP_
#define OMD_SETTINGS_HPP_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/noncopyable.hpp>
#include <boost/container/static_vector.hpp>
#include <map>
#include <iostream>

namespace omd {

    const std::string root("OMD");

    class Settings : boost::noncopyable
    {
    private:
        boost::property_tree::ptree ptree_;

        Settings() {}

    public:

        struct Udp {
            std::string name;
            bool isActive;
            bool isRecvOnly;
            std::string nic;
            std::string host;
            unsigned short port;
            std::string logger;

            Udp() = default;

            Udp(boost::property_tree::ptree const& pt) {
                name = pt.get<std::string>("<xmlattr>.name");
                isActive = pt.get<std::string>("<xmlattr>.active")=="true";
                isRecvOnly = pt.get<std::string>("<xmlattr>.isRecvOnly")=="true";
                nic = pt.get<std::string>("<xmlattr>.nic");
                host = pt.get<std::string>("<xmlattr>.host");
                port = pt.get<unsigned short>("<xmlattr>.port");
                logger = pt.get<std::string>("<xmlattr>.logger");
            }

        };

        struct UdpChannel {
            bool active;
            unsigned short id;
            std::string nic;
            std::string name;
            boost::container::static_vector<Udp, 2> lives;
            boost::container::static_vector<Udp, 2> refreshes;
        };

        struct Logger{
          std::string name;
          std::string filename;
          std::string format;
          bool isDebug;

          Logger() = default;

          Logger(boost::property_tree::ptree const& pt) {
            name = pt.get<std::string>("<xmlattr>.name");
            filename = pt.get<std::string>("filename");
            format = pt.get<std::string>("format");
            isDebug = pt.get<std::string>("debug") =="true";
          }
        };

        static std::string CfgFile;  // need to be set outside

        static Settings& GetInstance() {
            static Settings singleton;

            using boost::property_tree::read_xml;
            using boost::property_tree::xml_parser::no_comments;

            if (singleton.ptree_.empty() && !CfgFile.empty()) {
                read_xml(CfgFile, singleton.ptree_, no_comments);
            }

            return singleton;
        }

        ~Settings() {
            // write_xml(CfgFile, pt);
        }

        std::string getLogFileName() const {
            return ptree_.get<std::string>(root + ".root-logger.filename");
        }

        std::string getLogFormat() const {
            return ptree_.get<std::string>(root + ".root-logger.format");
        }

        bool isLogDebug() const {
            return ptree_.get<std::string>(root + ".root-logger.debug") == "true";
        }

        std::map<std::string, Udp> getChannelMap() const
        {
            std::map<std::string, Udp> chsmap;
            auto& pt = ptree_.get_child(root + ".channels");
            for (auto const& v : pt) {
              auto udp = Udp(v.second);
              std::string name = v.second.get<std::string>("<xmlattr>.name");
              chsmap[name] = udp;
            }
            return chsmap;
        }

        std::map<std::string, Logger> getLoggerMap() const
        {
          std::map<std::string, Logger> logmap;
          auto& pt = ptree_.get_child(root + ".loggers");
          for (auto const& v : pt) {
            auto logger = Logger(v.second);
            std::string name = v.second.get<std::string>("<xmlattr>.name");
            logmap[name] = logger;
          }
          return logmap;
        }

    };

} // namespace omd

#endif
