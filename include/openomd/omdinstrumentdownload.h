#pragma once
#include "openomd/multicastutil.h"
#include "openomd/channelconfig.h"

namespace openomd
{
template <typename _Processor, typename _RefreshProcessor, typename _Parser>
class OmdInstrumentDownload
{
public:
    struct Runner
    {
        _Processor& _processor;
        MulticastReceiver _refreshReceiver;
        _RefreshProcessor _refreshProcessor;
        _Parser _parser;
        bool _completed = false;

        Runner(IOServiceLC& ioService, _Processor& processor, ChannelConfig const& c)
            : _processor{processor},
              _refreshReceiver{c.channel, c.refreshPort, c.refreshListenIp, c.refreshIp, ioService }
        {
            _refreshProcessor.channel(c.channel);
        }

        void init()
        {
            _refreshReceiver.init();
            _refreshReceiver.registerAsyncReceive(&Runner::processRefresh, this);
            std::stringstream ss;
            ss << _refreshReceiver.name() << " init successfully";
            _processor.info(ss.str());
        }

        void start()
        {
            _refreshReceiver.start();
        }

        void stop()
        {
            _refreshReceiver.stop();
        }

        void processRefresh(const boost::system::error_code& error, size_t bytesRecvd, char* data, size_t maxLength)
        {
            if (error)
            {
                std::stringstream ss;
                ss << _processor.channel() << " processRefresh error: " << error.message();
                _processor.error(ss.str());
            }
            else
            {
                _parser.parse(data, bytesRecvd, _refreshProcessor);
                if (_refreshProcessor.refreshCompleted())
                {
                    _refreshProcessor.consumeAll([&](char* d, size_t s) {
                        _parser.parseRefresh(d, s, _processor);
                    });
                    _refreshReceiver.stop();
                    _completed = true;
                }
                _refreshReceiver.registerAsyncReceive(&Runner::processRefresh, this);
            }

        }
    };

    OmdInstrumentDownload(std::vector<ChannelConfig> const& channelConfig, std::ostream& ostream)
        : _processor{ostream}
    {
        for_each(channelConfig.begin(), channelConfig.end(), [&](auto const& c) {
            _runner.emplace_back(std::make_unique<Runner>(_ioServiceLC, _processor, c));
        });
    }

    void init()
    {
        _ioServiceLC.init();
        for_each(_runner.begin(), _runner.end(), [](auto& r) { r->init(); });
    }

    void start()
    {
        _ioServiceLC.start();
        for_each(_runner.begin(), _runner.end(), [](auto& r) { r->start(); });
        for (;;)
        {
            _ioServiceLC.ioService().poll();
            bool allCompleted = true;
            for_each(_runner.begin(), _runner.end(), [&](auto& r) { allCompleted &= r->_completed; });
            if (allCompleted)
            {
                break;
            }
        }
    }

    void run()
    {
    }

    void stop()
    {
        for_each(_runner.begin(), _runner.end(), [](auto& r) { r->stop(); });
        _ioServiceLC.stop();

        _processor.dump();
    }

protected:
    IOServiceLC _ioServiceLC;
    std::vector<std::unique_ptr<Runner>> _runner;
    _Processor _processor;
};
}
