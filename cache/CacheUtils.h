/**
 * @file CacheUtils.h
 * @author created by: Michal Durkovic
 * @author created on: 2016/02/22
 * @author \n
 * @author Copyright (c) 2016 ESET, spol. s r. o.
 * @note current owner: Michal Durkovic (michal.durkovic@eset.sk)
 * @note IMPORTANT: Before doing any signifcant change to this file check your plan with the current owner to avoid unexpected behaviour.
 */

#ifndef _ERA_REPOSITORY_HTTP_CACHECONTROL_H
#define _ERA_REPOSITORY_HTTP_CACHECONTROL_H

#include "CachedResponse.h"

#include <string>
#include <chrono>

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>

namespace Era {
namespace Common {
namespace Repository {
namespace Http {
namespace CacheUtils {

struct CCacheControl
{
    enum {
        kPublic  = 0x1,
        kPrivate = 0x2,
        kNoCache = 0x4,
        kNoStore = 0x8
    };

    CCacheControl(const std::string &spec);

    bool is_public() { return flags_ & kPublic; }
    bool is_private() { return flags_ & kPrivate; }
    bool is_no_cache() { return flags_ & kNoCache; }
    bool is_no_store() { return flags_ & kNoStore; }

    boost::optional<std::chrono::seconds>
    max_age() { return max_age_; }

private:
    unsigned                              flags_;
    boost::optional<std::chrono::seconds> max_age_;
};

boost::optional<std::string>
get_header_value(const cpr::Header &header, const std::string &str);

boost::optional<std::chrono::system_clock::time_point>
get_header_value_as_timepoint(const cpr::Header &header, const std::string &str);

boost::optional<CCacheControl>
get_cache_control(const cpr::Header &header);

std::chrono::system_clock::time_point
parse_http_date(const std::string &date);

bool
is_response_stale(const CCachedResponse &cached_response);

bool
is_response_cacheable(const cpr::Response &response);

}}}}}  // namespace Era::Common::Repository::Http::CacheUtils

#endif // _ERA_REPOSITORY_HTTP_CACHECONTROL_H
