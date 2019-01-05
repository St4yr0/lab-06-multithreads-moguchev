//  Copyright 2018 (c) Moguchev Leonid abracadabra.14@mail.ru
#include <picosha2.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <ostream>
#include <fstream>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/optional.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/attributes/value_extraction.hpp>

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

using byte = unsigned char;
using text_sink = sinks::synchronous_sink<sinks::text_ostream_backend>;

void my_formatter(logging::record_view const &rec,
    logging::formatting_ostream &strm) {
    using formatter = boost::log::formatter;
    formatter f = expr::stream
        << expr::format_date_time<boost::posix_time::ptime>(
            "TimeStamp", "[%Y/%m/%d  %H:%M:%S] ") << '<'
        << rec[logging::trivial::severity] << "> "
        << rec[expr::smessage];
    f(rec, strm);
}

void init(const std::string &logfilename) {
    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();
    sink->locked_backend()->add_stream(
        boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));

    sink->locked_backend()->auto_flush(true);

    sink->set_formatter(&my_formatter);

    logging::add_file_log(
        keywords::file_name = logfilename,
        keywords::format =
        (expr::stream << expr::format_date_time<boost::posix_time::ptime>(
            "TimeStamp", "[%Y-%m-%d %H:%M:%S]") << ": <"
            << logging::trivial::severity << "> "
            << expr::smessage));

    logging::core::get()->set_filter(
        logging::trivial::severity == logging::trivial::info ||
        logging::trivial::severity == logging::trivial::trace);

    logging::core::get()->add_sink(sink);
}

void search_correct_hash(int i, std::mutex *mtx) {
    init("Log.log");

    for (size_t i = 0; i < 65535; ++i) {
        std::vector<byte> data{
            (unsigned char)std::rand(), (unsigned char)std::rand(),
            (unsigned char)std::rand(), (unsigned char)std::rand() };
        const auto hash = picosha2::hash256_hex_string(data);

        std::lock_guard<std::mutex> mut(*mtx);
        if (hash.find(("0000"), hash.size() - 4) != std::string::npos) {
            BOOST_LOG_TRIVIAL(trace) << "Hash : " << hash;
        } else {
            BOOST_LOG_TRIVIAL(info) << "Thread : " << i << " Hash : " << hash;
        }
    }
}

int main(int argc, char *argv[]) {
    std::srand(std::time(nullptr));
    logging::add_common_attributes();

    auto num_of_threads = 0;
    if (argc == 2) {
        num_of_threads = std::atoi(argv[1]);
    } else {
        num_of_threads = std::thread::hardware_concurrency();
    }

    std::vector<std::shared_ptr<std::thread>> threads;
    std::mutex mtx;

    for (auto i = 0; i < num_of_threads; ++i) {
        threads.push_back(
            std::make_shared<std::thread>(
                search_correct_hash, i, &mtx));
    }

    for (auto &t : threads) t->join();

    system("pause");
    return EXIT_SUCCESS;
}
