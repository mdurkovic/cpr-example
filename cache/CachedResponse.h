/**
 * @file CachedResponse.h
 * @author created by: Michal Durkovic
 * @author created on: 2016/02/22
 * @author \n
 * @author Copyright (c) 2016 ESET, spol. s r. o.
 * @note current owner: Michal Durkovic (michal.durkovic@eset.sk)
 * @note IMPORTANT: Before doing any signifcant change to this file check your plan with the current owner to avoid unexpected behaviour.
 */

#ifndef _ERA_REPOSITORY_HTTP_RESPONSE_H_
#define _ERA_REPOSITORY_HTTP_RESPONSE_H_

#include <cpr/response.h>
#include <ctime>

#include <boost/optional.hpp>

namespace Era {
namespace Common {
namespace Repository {
namespace Http {

class CCachedResponse
{
public:

    CCachedResponse() = default;

    CCachedResponse(CCachedResponse&& response) = default;
    CCachedResponse(const CCachedResponse& response) = default;

    CCachedResponse(cpr::Response&& response);
    CCachedResponse(const cpr::Response& response);

    const cpr::Response & original();

    std::time_t  timestamp;
    cpr::Header  header;
    std::string  path;

private:
    boost::optional<cpr::Response> original_;
};

CCachedResponse &
operator <<(CCachedResponse& cached_response, cpr::Header &&header);

CCachedResponse &
operator <<(CCachedResponse &cached_response, const cpr::Header &header);

}}}} // namespace Era::Common::Repository::Http

namespace boost { namespace serialization {

// boost::serialization implementation for CRequest
template<class Archive>
inline void serialize(
    Archive & ar,
    Era::Common::Repository::Http::CCachedResponse & t,
    const unsigned int file_version
){
    ar & t.timestamp;
    ar & t.header;
    ar & t.path;
}

// boost::serialization implementation for CRequest
template<class Archive>
inline void serialize(
    Archive & ar,
    cpr::Response & t,
    const unsigned int file_version
){
    ar & t.status_code;
    ar & t.text;
    ar & t.header;
    ar & t.url;
    ar & t.elapsed;
    //ar & t.cookies; // TODO
    //ar & t.error; // TODO
}

}} // namespace boost::serialization

#endif // _ERA_REPOSITORY_HTTP_RESPONSE_H_
