/**
 * @file CachedResponse.cpp
 * @author created by: Michal Durkovic
 * @author created on: 2016/02/22
 * @author \n
 * @author Copyright (c) 2016 ESET, spol. s r. o.
 * @note current owner: Michal Durkovic (michal.durkovic@eset.sk)
 * @note IMPORTANT: Before doing any significant change to this file check your plan with the current owner to avoid unexpected behaviour.
 */


#include "CachedResponse.h"
#include "CacheUtils.h"

#include <chrono>
#include <fstream>
#include <sstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>

namespace Era {
namespace Common {
namespace Repository {
namespace Http {

CCachedResponse::CCachedResponse(cpr::Response&& response)
    : timestamp(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()))
    , header(response.header) // intentional copy, original header stays in the original response
    , original_(std::move(response))

{
    if (original_->error) {
        throw std::runtime_error("Error creating cached response: " + original_->error.message);
    }
}

CCachedResponse::CCachedResponse(const cpr::Response& response)
    : timestamp(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()))
    , header(response.header)
    , original_(response)
{
    if (original_->error) {
        throw std::runtime_error("Error creating cached response: " + original_->error.message);
    }
}

const cpr::Response &
CCachedResponse::original()
{
    if (!original_)
    {
        cpr::Response response;

        std::ifstream ifs(path);
        boost::archive::binary_iarchive ia(ifs);
        ia >> response;

        original_.emplace(std::move(response));
    }

    std::vector<std::string> hdrs = {"Date", "Expires", "Cache-Control", "ETag"};
    for (const auto &hdr : hdrs)
    {
        auto value = Http::CacheUtils::get_header_value(header, hdr);
        if (value)
        {
            original_->header[hdr] = std::move(value.get());
        }
    }

    return original_.get();
}

}
}
}
}
