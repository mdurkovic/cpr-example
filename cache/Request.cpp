/**
 * @file Request.cpp
 * @author created by: Michal Durkovic
 * @author created on: 2016/02/22
 * @author \n
 * @author Copyright (c) 2016 ESET, spol. s r. o.
 * @note current owner: Michal Durkovic (michal.durkovic@eset.sk)
 * @note IMPORTANT: Before doing any significant change to this file check your plan with the current owner to avoid unexpected behaviour.
 */


#include "Request.h"

namespace Era {
namespace Common {
namespace Repository {
namespace Http {

std::string request_compare_key(const CRequest &req)
{
    auto user_agent = req.header.find("User-Agent");
    return req.url + req.parameters.content + (user_agent != req.header.end() ? user_agent->second : std::string());
}

bool operator <(const CRequest& r1, const CRequest& r2)
{
    return boost::lexicographical_compare(request_compare_key(r1),
                                          request_compare_key(r2));
}

CRequest &operator <<(CRequest &request, CRequest::header_t &&header)
{
    header.insert(request.header.begin(), request.header.end());
    request.header = std::move(header);

    return request;
}

CRequest &operator <<(CRequest &request, const CRequest::header_t &header)
{
    CRequest::header_t new_header(header);

    new_header.insert(request.header.begin(), request.header.end());
    request.header = std::move(new_header);

    return request;
}

}
}
}
}
