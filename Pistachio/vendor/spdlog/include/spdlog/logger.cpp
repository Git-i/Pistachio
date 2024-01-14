#include "logger.h"
namespace spdlog
{
    // Empty logger
    logger::logger(std::string name)
        : name_(std::move(name))
        , sinks_()
    {}

    // Logger with range on sinks
    template<typename It>
    logger::logger(std::string name, It begin, It end)
        : name_(std::move(name))
        , sinks_(begin, end)
    {}

    // Logger with single sink

    logger::logger(std::string name, sink_ptr single_sink)
        : logger(std::move(name), { std::move(single_sink) })
    {}

    // Logger with sinks init list
    logger::logger(std::string name, sinks_init_list sinks)
        : logger(std::move(name), sinks.begin(), sinks.end())
    {}
}
