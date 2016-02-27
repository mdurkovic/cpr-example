/**
 * @file Cache.cpp
 * @author created by: Michal Durkovic
 * @author created on: 2016/02/22
 * @author \n
 * @author Copyright (c) 2016 ESET, spol. s r. o.
 * @note current owner: Michal Durkovic (michal.durkovic@eset.sk)
 * @note IMPORTANT: Before doing any significant change to this file check your plan with the current owner to avoid unexpected behaviour.
 */


#include "Cache.h"

#include <fstream>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/filesystem.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#include <cpr/cpr.h>

namespace Era {
namespace Common {
namespace Repository {
namespace Http {

CCache::CCache(const boost::filesystem::path &dir)
    : cache_dir_(dir)
{
    if (!boost::filesystem::exists(cache_dir_))
    {
        boost::filesystem::create_directory(cache_dir_);
    }

    if (!boost::filesystem::is_directory(cache_dir_))
    {
        throw std::runtime_error(cache_dir_.string() + " is not a directory");
    }

    auto cache_index = CacheIndexFilename();
    if (boost::filesystem::is_regular_file(cache_index))
    {
        std::ifstream ifs(cache_index.string());
        boost::archive::binary_iarchive ia(ifs);
        ia >> index_;
    }
}

CCache::~CCache()
{
    auto cache_index = CacheIndexFilename();
    std::ofstream ofs(cache_index.string());
    boost::archive::binary_oarchive oa(ofs);
    oa << index_;
}

void
CCache::Put(const CRequest &request, CCachedResponse &&response)
{
    response.path = Store(response.original());
    index_.insert( {request, response} );
}

boost::optional<CCachedResponse&>
CCache::Match(const CRequest &request, unsigned options)
{
    auto match = index_.find(request);
    if (match != index_.end()) return match->second;
    else return boost::none;
}

bool CCache::Delete(const CRequest &request, unsigned options)
{
    auto match = index_.find(request);
    if (match != index_.end())
    {
        boost::system::error_code ec;
        boost::filesystem::remove(match->second.path, ec);

        index_.erase(match);

        return true;
    }
    else return false;
}

std::string CCache::Store(const cpr::Response &response)
{
    auto uuid = boost::uuids::random_generator()();
    auto path = cache_dir_ / (boost::uuids::to_string(uuid) + ".dat");

    path.normalize();

    std::ofstream ofs(path.string());
    boost::archive::binary_oarchive oa(ofs);
    oa << response;

    return path.string();
}

}
}
}
}
