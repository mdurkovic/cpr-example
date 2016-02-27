/**
 * @file CacheUtils.cpp
 * @author created by: Michal Durkovic
 * @author created on: 2016/02/22
 * @author \n
 * @author Copyright (c) 2016 ESET, spol. s r. o.
 * @note current owner: Michal Durkovic (michal.durkovic@eset.sk)
 * @note IMPORTANT: Before doing any signifcant change to this file check your plan with the current owner to avoid unexpected behaviour.
 */

#include "CacheUtils.h"

#include <time.h>
#include <chrono>
#include <vector>

namespace Era { namespace Common { namespace Repository { namespace Http { namespace CacheUtils {


CCacheControl::CCacheControl(const std::string &spec)
{
    std::vector<std::string> tokens;
    boost::algorithm::split(tokens, spec,
                            boost::is_any_of(","),
                            boost::algorithm::token_compress_on);

    for (auto &token : tokens)
    {
        boost::algorithm::trim(token);
        if (token.empty()) continue;

        if (boost::iequals(token, "public"))        { flags_ |= kPublic; }
        else if (boost::iequals(token, "private"))  { flags_ |= kPrivate; }
        else if (boost::iequals(token, "no-cache")) { flags_ |= kNoCache; }
        else if (boost::iequals(token, "no-store")) { flags_ |= kNoStore; }
        else if (boost::starts_with(token, "max-age="))
        {
            max_age_ = std::chrono::seconds(std::stoul(token.substr(sizeof("max-age"))));
        }
    }
}

boost::optional<std::string>
get_header_value(const cpr::Header &header, const std::string &str)
{
    auto value = header.find(str);
    if ( value != header.end() )
    {
        return value->second;
    }

    return boost::none;
}


boost::optional<std::chrono::system_clock::time_point>
get_header_value_as_timepoint(const cpr::Header &header, const std::string &str)
{
    auto value = get_header_value(header, str);
    if ( value )
    {
        return parse_http_date(value.get());
    }

    return boost::none;
}

boost::optional<CCacheControl>
get_cache_control(const cpr::Header &header)
{
    auto cache_control = get_header_value(header, "Cache-Control");
    if (cache_control)
    {
        return CCacheControl(cache_control.get());
    }

    return boost::none;
}

std::chrono::system_clock::time_point
parse_http_date(const std::string &date)
{
    struct tm time;

    auto ret = strptime(date.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &time);
    if (NULL == ret)
    {
        ret = strptime(date.c_str(), "%a, %d-%b-%Y %H:%M:%S GMT", &time);
        if (NULL == ret)
        {
            ret = strptime(date.c_str(), "%a %b %a %H:%M:%S %Y", &time);
            if (NULL == ret)
            {
                throw std::runtime_error("Failed to parse http date '" + date + "'");
            }
        }
    }

    return std::chrono::system_clock::from_time_t(std::mktime(&time));
}

bool
is_response_stale(const CCachedResponse& cached_response)
{
    auto cache_control = get_cache_control(cached_response.header);
    if (cache_control)
    {
        auto max_age = cache_control->max_age();
        if (max_age)
        {
            auto limit = std::chrono::system_clock::from_time_t(cached_response.timestamp);
            limit += std::chrono::seconds(max_age.get());

            return limit < std::chrono::system_clock::now();
        }
    }
    else
    {
        auto expires = get_header_value_as_timepoint(cached_response.header, "Expires");
        if (expires)
        {
            auto &limit = expires.get();
            return limit < std::chrono::system_clock::now();
        }
    }

    return true;
}

bool
is_response_cacheable(const cpr::Response &response)
{
    return true;
}

}}}}} // namespace Era::Common::Repository::Http::CacheUtils
