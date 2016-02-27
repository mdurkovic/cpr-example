/**
 * @file Cache.h
 * @author created by: Michal Durkovic
 * @author created on: 2016/02/22
 * @author \n
 * @author Copyright (c) 2016 ESET, spol. s r. o.
 * @note current owner: Michal Durkovic (michal.durkovic@eset.sk)
 * @note IMPORTANT: Before doing any signifcant change to this file check your plan with the current owner to avoid unexpected behaviour.
 */

#ifndef _ERA_REPOSITORY_HTTP_CACHE_H_
#define _ERA_REPOSITORY_HTTP_CACHE_H_

#include "Request.h"
#include "CachedResponse.h"

#include <map>
#include <algorithm>
#include <iterator>

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

namespace Era {
namespace Common {
namespace Repository {
namespace Http {

class CCache
{
    //static unsigned kIgnoreSearch = 0x1;
    //static unsigned kIgnoreMethod = 0x2;
    //static unsigned kIgnoreVary   = 0x4;

public:
    typedef std::map<CRequest, CCachedResponse> index_t;

    CCache(const boost::filesystem::path &dir);

    virtual ~CCache();

    void
    Put(const CRequest &request, CCachedResponse &&response);

    boost::optional<CCachedResponse&>
    Match(const CRequest &request, unsigned options = 0);

    bool
    Delete(const CRequest &request, unsigned options = 0);

protected:
    inline boost::filesystem::path
    CacheIndexFilename() { return cache_dir_ / "cache.dat"; }

    std::string
    Store(const cpr::Response &response);

private:
    boost::filesystem::path cache_dir_;
    index_t                 index_;
};

}}}} // namespace Era::Common::Repository::Http

#endif // _ERA_REPOSITORY_HTTP_CACHE_H_
