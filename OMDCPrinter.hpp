#pragma once
#ifndef OMD_OMDCPRINTER_HPP_
#define OMD_OMDCPRINTER_HPP_

#include "spdlog/spdlog.h"
#include "openomd/omdcprocessor.h"
#include "openomd/nooplinearbitration.h"
#include "openomd/omdbaseprocessor.h"
#include <chrono>

using namespace std::chrono;

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

namespace omd {

    class OMDCPrinter : public openomd::OMDCProcessor<openomd::NoopLineArbitration, openomd::BaseProcessor> {
    public:
      OMDCPrinter(uint64_t &sendtime) : _sendtime{sendtime} {}
      void update(uint64_t &st){
          _sendtime = st;
      }

      void initlog(const std::string logger_name){
        log = spdlog::get(logger_name);
      }
    protected:
      uint64_t &_sendtime;
      std::shared_ptr<spdlog::logger> log;
    };

    struct PrintProcessor : public OMDCPrinter
    {
        using OMDCPrinter::OMDCPrinter;

        //Sequence Reset (100)
        void onMessage(omdc::sbe::SequenceReset const& sr, uint32_t)
        {
          log->debug("[{}][SequenceReset]NewSeqNo:{}", _sendtime, sr.newSeqNo());
            
        }

        //Disaster Recovery Signal (105)
        void onMessage(omdc::sbe::DisasterRecoverySignal const& drs, uint32_t)
        {
            log->info("[DisasterRecoverySignal]DRStatus:{}", drs.drStatus());
            
        }

        //Logon (101)
        void onMessage(omdc::sbe::Logon const& logon, uint32_t)
        {
            log->debug("[Logon]Username:{}", logon.username());
            
        }

        //Logon Response (102)
        void onMessage(omdc::sbe::LogonResponse const& lresp, uint32_t)
        {
            log->debug("[LogonResponse]SessionStatus:{}", lresp.sessionStatus());
            
        }

        //Retransmission Request (201)
        void onMessage(omdc::sbe::RetransmissionReq const& rreq, uint32_t)
        {
            log->debug("[RetransmissionReq]ChannelID:{0}, BeginSeqNum:{1}, EndSeqNum{2}", rreq.channelID(), rreq.beginSeqNum(), rreq.endSeqNum());
            
        }

        //Retransmission Response (202)
        void onMessage(omdc::sbe::RetransmissionResp const& rresp, uint32_t)
        {
            log->debug("[RetransmissionResp]ChannelID:{0}, RetransStatus:{1}, BeginSeqNum:{2}, EndSeqNum{3}",
                          rresp.channelID(), rresp.retransStatus(), rresp.beginSeqNum(), rresp.endSeqNum());
            
        }

        //Refresh Complete (203)
        void onMessage(omdc::sbe::RefreshComplete const& rc, uint32_t)
        {
            log->debug("[RefreshComplete]LastSeqNum:{0}", rc.lastSeqNum());
            
        }

        //Market Definition (10)
        void onMessage(omdc::sbe::MarketDefinition const& md, uint32_t)
        {
            log->debug("[MarketDefinition]MarketCode:{0}, MarketName:{1}, CurrencyCode:{2}, NumberOfSecurities:{3}"
                    , md.marketCode(), md.marketName(), md.currencyCode(), md.numberOfSecurities());
            
        }

        //Security Definition (11)
        void onMessage(omdc::sbe::SecurityDefinition const& sd, uint32_t value)
        {
            log->debug("[SecurityDefinition]SecurityCode:{0}, MarketCode:{1}, ISINCode:{2}, InstrumentType:{3}, ProductType:{4}"
                    , sd.securityCode(), sd.getMarketCodeAsString(), sd.getIsinCodeAsString(), sd.getInstrumentTypeAsString(), sd.productType());
            
        }

        //Liquidity Provider (13)
        void onMessage(omdc::sbe::LiquidityProvider& lp, uint32_t) {
            auto &lps = lp.noLiquidityProviders();
            log->debug("[LiquidityProvider]SecurityCode:{0}, count:{1}", lp.securityCode(), lps.count());
            while (lps.hasNext()){
                lps.next();
                log->debug("[LiquidityProvider]LPBrokerNumber:{0}", lps.lpBrokerNumber());
            }
            
        }

        //Currency Rate (14)
        void onMessage(omdc::sbe::CurrencyRate const& cr, uint32_t)
        {
            log->debug("[CurrencyRate]CurrencyCode:{0}, CurrencyFactor:{1}, CurrencyRate:{2}"
                    , cr.currencyCode(), cr.currencyFactor(), cr.currencyRate());
            
        }

        //Trading Session Status (20)
        void onMessage(omdc::sbe::TradingSessionStatus const& tss, uint32_t)
        {
            log->info("[TradingSessionStatus]MarketCode:{0}, TradingSessionSubID:{1}, TradingSesStatus{2}, TradingSesControlFlag:{3}, StartDateTime:{4}, EndDateTime:{5}"
                    , tss.marketCode(), tss.tradingSessionSubID(), tss.tradingSesStatus(), tss.tradingSesControlFlag(), tss.startDateTime(), tss.endDateTime());
            
        }

        //Security Status (21)
        void onMessage(omdc::sbe::SecurityStatus const& ss, uint32_t)
        {
            log->debug("[SecurityStatus]SecurityCode:{0}, SuspensionIndicator:{1}"
                    , ss.securityCode(), ss.suspensionIndicator());
            
        }

        //Add Order (30)
        void onMessage(omdc::sbe::AddOrder const& ao, uint32_t)
        {
            log->info("[{7}-{8}][AddOrder]SecurityCode:{0}, OrderId:{1}, Price:{2}, Quantity:{3}, Side:{4}, OrderType:{5}, OrderBookPosition:{6}"
                    , ao.securityCode(), ao.orderID(), ao.price(), ao.quantity(), ao.side(), ao.orderType(), ao.orderBookPosition(), _sendtime, micros() );
            
        }

        //Modify Order (31)
        void onMessage(omdc::sbe::ModifyOrder const& mo, uint32_t)
        {
            log->info("[ModifyOrder]SecurityCode:{0}, OrderId:{1}, Quantity:{2}, Side:{3}, OrderBookPosition:{4}"
                    , mo.securityCode(), mo.orderID(), mo.quantity(), mo.side(), mo.orderBookPosition());
            
        }

        //Delete Order (32)
        void onMessage(omdc::sbe::DeleteOrder const& d, uint32_t)
        {
            log->info("[{3}-{4}][DeleteOrder]SecurityCode:{0}, OrderId:{1}, Side:{2}"
                    , d.securityCode(), d.orderID(), d.side(), _sendtime, micros());
            
        }

        //Add Odd Lot Order (33)
        void onMessage(omdc::sbe::AddOddLotOrder const& aolo, uint32_t)
        {
            log->debug("[AddOddLotOrder]SecurityCode:{0}, OrderId:{1}, Price:{2}, Quantity:{3}, BrokerID:{4}, Side:{5}"
                    , aolo.securityCode(), aolo.orderID(), aolo.price(), aolo.quantity(), aolo.brokerID(), aolo.side());
            
        }

        //Delete Odd Lot Order (34)
        void onMessage(omdc::sbe::DeleteOddLotOrder const& dolo, uint32_t)
        {
            log->debug("[DeleteOddLotOrder]SecurityCode:{0}, OrderId:{1}, BrokerID:{2}, Side:{3}"
                    , dolo.securityCode(), dolo.orderID(), dolo.brokerID(), dolo.side());
            
        }

        //Aggregate Order Book Update (53)
        void onMessage(omdc::sbe::AggregateOrderBookUpdate& aobu, uint32_t)
        {
            auto& en = aobu.noEntries();
            log->debug("[AggregateOrderBookUpdate]SecurityCode:{0}, NoEntries:{1}"
                    , aobu.securityCode(), en.count());
            int i = 0;
            while(en.hasNext()){
                en.next();
                log->debug("[AggregateOrderBookUpdate]({0})AggregateQuantity:{1}, Price:{2}, NumberOfOrders:{3}, Side:{4}, PriceLevel:{5}, UpdateAction:{6}"
                        , ++i, en.aggregateQuantity(), en.price(), en.numberOfOrders(), en.side(), en.priceLevel(), en.updateAction());
            }
            
        }

        //Broker Queue (54)
        void onMessage(omdc::sbe::BrokerQueue& bq, uint32_t)
        {
            auto& items = bq.items();
            log->debug("[BrokerQueue]SecurityCode:{0}, ItemCount:{1}, Side:{2}, BQMoreFlag:{3}"
                    , bq.securityCode(), items.count(), items.side(), items.bqMoreFlag());
            int i = 0;
            while(items.hasNext()){
                items.next();
                log->debug("[BrokerQueue]({0})Item:{1}, Type:{2}"
                        , ++i, items.item(), items.itemType());
            }
            
        }

        //Order Imbalance (56)
        void onMessage(omdc::sbe::OrderImbalance const& oib, uint32_t)
        {
            log->debug("[OrderImbalance]SecurityCode:{0}, OrderImbalanceDirection:{1}, OrderImbalanceQuantity:{2}"
                    , oib.securityCode(), oib.orderImbalanceDirection(), oib.orderImbalanceQuantity());
            
        }

        //Trade (50)
        void onMessage(omdc::sbe::Trade const& t, uint32_t)
        {
            log->info("[Trade]SecurityCode:{0}, TradeID:{1}, Price:{2}, Quantity:{3}, TrdType:{4}, TradeTime:{5}"
                    , t.securityCode(), t.tradeID(), t.price(), t.quantity(), t.trdType(), t.tradeTime());
            
        }

        //Trade Cancel (51)
        void onMessage(omdc::sbe::TradeCancel const& tc, uint32_t)
        {
            log->info("[TradeCancel]SecurityCode:{0}, TradeID:{1}"
                    , tc.securityCode(), tc.tradeID());
            
        }

        //Trade Ticker (52)
        void onMessage(omdc::sbe::TradeTicker const& tt, uint32_t)
        {
            log->debug("[TradeTicker]SecurityCode:{0}, TickerID:{1}, Price:{2}, AggregateQuantity:{3}, TradeTime:{4}, TrdType:{5}, TrdCancelFlag:{6}"
                    , tt.securityCode(), tt.tickerID(), tt.price(), tt.aggregateQuantity(), tt.tradeTime(), tt.trdType(), tt.trdCancelFlag());
            
        }

        //Closing Price (62)
        void onMessage(omdc::sbe::ClosingPrice const& cp, uint32_t)
        {
            log->debug("[ClosingPrice]SecurityCode:{0}, ClosingPrice:{1}, NumberOfTrades:{2}"
                    , cp.securityCode(), cp.closingPrice(), cp.numberOfTrades());
            
        }

        //Nominal Price (40)
        void onMessage(omdc::sbe::NominalPrice const& np, uint32_t)
        {
            log->debug("[ClosingPrice]SecurityCode:{0}, NominalPrice:{1}"
                    , np.securityCode(), np.nominalPrice());
            
        }

        //Indicative Equilibrium Price (41)
        void onMessage(omdc::sbe::IndicativeEquilibriumPrice const& iep, uint32_t)
        {
            log->debug("[IndicativeEquilibriumPrice]SecurityCode:{0}, Price:{1}, AggregateQuantity:{2}"
                    , iep.securityCode(), iep.price(), iep.aggregateQuantity());
            
        }

        //Reference Price (43)
        void onMessage(omdc::sbe::ReferencePrice const& rp, uint32_t)
        {
            log->debug("[ReferencePrice]SecurityCode:{0}, ReferencePrice:{1}, LowerPrice:{2}, UpperPrice:{3}"
                    , rp.securityCode(), rp.referencePrice(), rp.lowerPrice(), rp.upperPrice());
            
        }

        //VCM Trigger (23)
        void onMessage(omdc::sbe::VCMTrigger const& vcm, uint32_t)
        {
            log->debug("[VCMTrigger]SecurityCode:{0}, CoolingOffStartTime:{1}, CoolingOffEndTime:{2}, VCMReferencePrice:{3}, VCMLowerPrice:{4}, VCMUpperPrice:{5}"
                    , vcm.securityCode(), vcm.coolingOffStartTime(), vcm.coolingOffEndTime(), vcm.vcmReferencePrice(), vcm.vcmLowerPrice(), vcm.vcmUpperPrice());
            
        }

        //Statistics (60)
        void onMessage(omdc::sbe::Statistics const& stat, uint32_t)
        {
            log->debug("[Statistics]SecurityCode:{0}, SharesTraded:{1}, Turnover:{2}, HighPrice:{3}, LowPrice:{4}, LastPrice:{5}, VWAP:{6}, ShortSellSharesTraded:{7}, ShortSellTurnover:{8}"
                    , stat.securityCode(), stat.sharesTraded(), stat.turnover(), stat.highPrice(), stat.lowPrice(), stat.lastPrice(), stat.vwap(), stat.shortSellSharesTraded(), stat.shortSellTurnover());
            
        }

        //Market Turnover (61)
        void onMessage(omdc::sbe::MarketTurnover const& mt, uint32_t)
        {
            log->debug("[MarketTurnover]MarketCode:{0}, CurrencyCode:{1}, Turnover:{2}"
                    , mt.getMarketCodeAsString(), mt.getCurrencyCodeAsString(), mt.turnover());
            
        }

        //Yield (44)
        void onMessage(omdc::sbe::Yield const& y, uint32_t)
        {
            log->debug("[Yield]SecurityCode:{0}, Yield:{1}"
                    , y.securityCode(), y.yield());
            
        }

        //News (22)
        void onMessage(omdc::sbe::News& news, uint32_t)
        {
            auto& market = news.noMarketCodes();
            auto& sec = news.noSecurityCodes();
            auto& newsLines = news.noNewsLines();
            log->debug("[News]NewsType:{0}, NewsID:{1}, Headline:{2}, CancelFlag:{3}, LastFragment:{4}, ReleaseTime:{5}, NoMarketCodes:{6}, NoSecurityCodes:{7}, NoNewsLines:{8}"
                    , news.getNewsTypeAsString(), news.getNewsIDAsString(), news.getHeadlineAsString(), news.cancelFlag(), news.lastFragment(), news.releaseTimeEncodingLength(), market.count(), sec.count(), newsLines.count());

            int i = 0;
            while(market.hasNext()){
                market.next();
                log->debug("[News]({0})MarketCode:{1},"
                        , ++i, market.getMarketCodeAsString(), news.getNewsIDAsString(), news.getHeadlineAsString(), news.cancelFlag(), news.lastFragment(), news.releaseTimeEncodingLength());

            }

            int j = 0;
            while(sec.hasNext()){
                sec.next();
                log->debug("[News]({0})SecurityCode:{1}"
                        , ++j, sec.securityCode());

            }

            int k = 0;
            while(newsLines.hasNext()){
                newsLines.next();
                log->debug("[News]({0})NewsLine Size:{1}"
                        , ++k, newsLines.getNewsLineAsString().size());

            }
            
        }

        //Index Definition (70)
        void onMessage(omdc::sbe::IndexDefinition const& idd, uint32_t)
        {
            log->debug("[IndexDefinition]IndexCode:{0}, IndexSource:{1}, CurrencyCode:{2}"
                    , idd.getIndexCodeAsString(), idd.indexSource(), idd.getCurrencyCodeAsString());
            
        }

        //Index Data (71)
        void onMessage(omdc::sbe::IndexData const& id, uint32_t)
        {
            log->debug("[IndexData]IndexCode:{0}, IndexStatus:{1}, IndexTime:{2}, IndexValue:{3}, NetChgPrevDay:{4}, HighValue:{5}"
                          ", LowValue:{6}, EASValue:{7}, IndexTurnover:{8}, OpeningValue:{9}, ClosingValue:{10}"
                          ", PreviousSesClose:{11}, IndexVolume{12}, NetChgPrevDayPct:{13}, Exception:{14}"
                    , id.getIndexCodeAsString(), id.indexStatus(), id.indexTime(), id.indexValue(), id.netChgPrevDay()
                    , id.highValue(), id.lowValue(), id.easValue(), id.indexTurnover(), id.openingValue()
                    , id.closingValue(), id.previousSesClose(), id.indexVolume(), id.netChgPrevDayPct(), id.exception());
            
        }

        //Stock Connect Daily Quota Balance (80)
        void onMessage(omdc::sbe::StockConnectDailyQuotaBalance const& scDQB, uint32_t)
        {
            log->debug("[StockConnectDailyQuotaBalance]StockConnectMarket:{0}, TradingDirection:{1}, DailyQuotaBalance:{2}, DailyQuotaBalanceTime:{3}"
                    , scDQB.getStockConnectMarketAsString(), scDQB.getTradingDirectionAsString(), scDQB.dailyQuotaBalance(), scDQB.dailyQuotaBalanceTime());
            
        }

        //Stock Connect Market Turnover (81)
        void onMessage(omdc::sbe::StockConnectMarketTurnover const& scMT, uint32_t)
        {
            log->debug("[StockConnectMarketTurnover]StockConnectMarket:{0}, TradingDirection:{1}, BuyTurnover:{2}, SellTurnover:{3}, BuySellTurnover:{4}"
                    , scMT.getStockConnectMarketAsString(), scMT.getTradingDirectionAsString(), scMT.buyTurnover(), scMT.sellTurnover(), scMT.buySellTurnover());
            
        }

        void onHeartbeat()
        {
            log->debug("on heat beat");
        }

        void onUnknownMessage(uint16_t, uint16_t)
        {
            log->debug("Unknown Message");
        }
        using OMDCProcessor::onMessage;
    };
}

#endif