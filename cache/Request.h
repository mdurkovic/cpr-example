/**
 * @file Request.h
 * @author created by: Michal Durkovic
 * @author created on: 2016/02/22
 * @author \n
 * @author Copyright (c) 2016 ESET, spol. s r. o.
 * @note current owner: Michal Durkovic (michal.durkovic@eset.sk)
 * @note IMPORTANT: Before doing any signifcant change to this file check your plan with the current owner to avoid unexpected behaviour.
 */

#ifndef _ERA_REPOSITORY_HTTP_REQUEST_H_
#define _ERA_REPOSITORY_HTTP_REQUEST_H_

#include <cpr/cpr.h>

#include <boost/algorithm/string.hpp>

#include <boost/serialization/split_free.hpp>

namespace Era {
namespace Common {
namespace Repository {
namespace Http {

struct CRequest
{
    typedef cpr::Url url_t;
    typedef cpr::Header header_t;
    typedef cpr::Parameters parameters_t;

    CRequest() = default;

    CRequest(CRequest&& response) = default;
    CRequest(const CRequest& response) = default;

    void SetOption(url_t &&val) { url = val; }
    void SetOption(header_t &&val) { header = val; }
    void SetOption(parameters_t &&val) { parameters = val; }

    // serialized members
    url_t        url;
    header_t     header;
    parameters_t parameters;
};

bool operator <(const CRequest& r1, const CRequest& r2);

CRequest &operator <<(CRequest& request, CRequest::header_t &&header);

CRequest &operator <<(CRequest &request, const CRequest::header_t &header);

namespace priv {

#define FWD(...) ::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

template <typename T>
void set_option(CRequest& request, T&& t) {
    request.SetOption(FWD(t));
}

template <typename T, typename... Ts>
void set_option(CRequest& request, T&& t, Ts&&... ts) {
    set_option(request, FWD(t));
    set_option(request, FWD(ts)...);
}

} // namespace priv

template <typename... Ts>
CRequest MakeRequest(Ts&&... ts) {
    CRequest request;
    priv::set_option(request, FWD(ts)...);
    return request;
}

}}}} // namespace Era::Common::Repository::Http

namespace boost { namespace serialization {

// boost::serialization implementation for CRequest
template<class Archive>
inline void serialize(
    Archive & ar,
    Era::Common::Repository::Http::CRequest & t,
    const unsigned int file_version
){
    ar & t.url;
    ar & t.header;
    ar & t.parameters;
}

// boost::serialization implementation for CRequest::parameters_t
template<class Archive>
inline void serialize(
    Archive & ar,
    Era::Common::Repository::Http::CRequest::parameters_t & t,
    const unsigned int file_version
){
    ar & t.content;
}

}} // namespace boost::serialization

#endif // _ERA_REPOSITORY_HTTP_REQUEST_H_
