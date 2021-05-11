//
// Created by Sean on 2020/3/19.
//

#ifndef OMD_HANDLER_LATENCY_HPP
#define OMD_HANDLER_LATENCY_HPP
#include <iostream>
#include <chrono>
#include <iomanip>
#include <array>
#include <vector>
#include <cmath>

template <std::uint64_t TMaxDurationNs, size_t TBucketCount = 1000000>
class latency
{
private:
    size_t count { 0 };
    std::chrono::nanoseconds bucketDuration{TMaxDurationNs / TBucketCount};
    std::array<size_t, TBucketCount> buckets{};
    std::vector< uint64_t > data;
public:
    latency(){}
    template <typename T>
    void add(T duration)
    {
        auto bucketIndex = duration / bucketDuration;
        if(bucketIndex > TBucketCount)
            throw std::runtime_error("Latency computing failed");

        buckets[bucketIndex]++;
        data.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
        count++;
    }

    template <typename O, typename TRatio = std::chrono::nanoseconds>
    void generate(O& output, bool isRtt)
    {
        std::cout   << std::setw(7) << "Percentile"
                    << std::setw(14) << "Latency(us)"
                    << std::endl;

        size_t cum = 0;
        double mean = 0;
        int index = 0;
        for(size_t i = 0; i < TBucketCount; i++)
        {
            auto current = buckets[i];
            if (current == 0)
                continue;

            cum += current;

            auto percentile = (double)cum / count * 100.0;
            double rang_percentile[5] = {50.0, 75.0, 90.0, 99.9, 99.99};
            if(percentile > rang_percentile[index] && index < sizeof(rang_percentile)) {
                double dur = (double)std::chrono::duration_cast<TRatio>((i + 1) * bucketDuration).count()/1000;
                printf("%8.2f%% \t %.2f us\n", percentile, dur);
                index++;
            }

            mean = (mean * (cum-current) + current * (i+1)) / cum;
        }

        int denominator = isRtt ? 2 : 1;
        float standardDeviation = 0.0;
        float average = std::chrono::duration_cast<TRatio>(mean * bucketDuration).count();
        for(int k = 0; k < count; ++k)
            standardDeviation += std::pow(data[k] - average, 2);
        std::cout << "--------------------------" << std::endl;
        std::cout << "avg-latency = " << (double)std::chrono::duration_cast<TRatio>(mean * bucketDuration).count()/1000/denominator << "us (std-dev = " << sqrt(standardDeviation / count)/1000/denominator << "us)" << std::endl;
    }
};

#endif //OMD_HANDLER_LATENCY_HPP
