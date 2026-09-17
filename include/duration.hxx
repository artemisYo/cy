#pragma once

#include <cstdint>
#include <cstdio>
#include <ctime>

namespace cy {

class Time {
    static constexpr uint64_t ns_per_micro = 1000;
    static constexpr uint64_t ns_per_milli = 1000 * ns_per_micro;
    static constexpr uint64_t ns_per_sec = 1000 * ns_per_milli;

    uint64_t m_ns;

    Time(uint64_t ns) : m_ns(ns) {}

  public:
    static Time now() {
#if __APPLE__
        uint64_t ns = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
        return Time{ns};
#else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return Time{ts.tv_sec * ns_per_sec + ts.tv_nsec};
#endif
    }

    Time operator-(Time other) {
        return Time{m_ns - other.m_ns};
    }

    void print() {
        uint64_t ns = m_ns;
        if (ns > ns_per_sec) {
            auto s = ns / ns_per_sec;
            auto ms = (ns % ns_per_sec) / ns_per_milli;
            printf("%3llus %3llums", s, ms);
            return;
        }
        if (ns > ns_per_milli) {
            auto ms = ns / ns_per_milli;
            auto mis = (ns % ns_per_milli) / ns_per_micro;
            printf("%3llums %3lluµs", ms, mis);
            return;
        }
        auto mis = ns / ns_per_micro;
        ns = ns % ns_per_micro;
        printf("%3lluµs %3lluns", mis, ns);
    }
};

} // namespace cy
