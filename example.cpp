#include <iostream>

#include <cpr/cpr.h>
#include <json.hpp>

#include "cache/Cache.h"
#include "cache/CacheUtils.h"

#include <boost/filesystem.hpp>

template<typename T>
void print_response(const T &response)
{
    std::cout << "> HTTP/1.1 " << response.status_code << std::endl;
    std::for_each(response.header.cbegin(),
                  response.header.cend(),
                  [](const std::pair<std::string, std::string> &val) {
        std::cout << "> " << val.first << ": " << val.second << std::endl;
    });
    std::cout << ">" << std::endl;

    auto &hdr = response.header;
    if (hdr.find("Content-Type") != hdr.end() &&
        boost::iequals(hdr.at("Content-Type"), "application/json"))
    {
        auto json = nlohmann::json::parse(response.text);
        std::cout <<  json.dump(4) << std::endl;
    }
    else
    {
        std::cout <<  response.text << std::endl;
    }
}

namespace Era {
namespace Common {
namespace Repository {
namespace Http {

Era::Common::Repository::Http::CRequest
make_validation_request(const Era::Common::Repository::Http::CRequest &request,
                        const Era::Common::Repository::Http::CCachedResponse &cached_response)
{
    using namespace Era::Common::Repository;

    auto validation_request(request);

    auto etag = Http::CacheUtils::get_header_value(cached_response.header, "ETag");
    if (etag) {
        validation_request << cpr::Header{ {"If-None-Match", etag.get()} };
    }

    auto last_modified = Http::CacheUtils::get_header_value(cached_response.header, "Last-Modified");
    if (last_modified) {
        validation_request << cpr::Header{ {"If-Modified-Since", last_modified.get()} };
    }

    return validation_request;
}

cpr::Response
Get(cpr::Session &session, const CRequest &request)
{
    cpr::priv::set_option(session,
                          request.url,
                          request.header,
                          request.parameters);
    return session.Get();
}

cpr::Response
validate(cpr::Session &session,
         Era::Common::Repository::Http::CCache &cache,
         const Era::Common::Repository::Http::CRequest &request,
         Era::Common::Repository::Http::CCachedResponse &cached_response)
{
    using namespace Era::Common::Repository;

    auto validation_request = make_validation_request(request, cached_response);
    validation_request.header["Connection"] = "Keep-Alive";

    auto validation_response = Http::Get(session, validation_request);

    if ( 304 == validation_response.status_code ) {
        std::cout << "Resource not modified: " << request.url << std::endl;
        //print_response(validation_response);

        cached_response.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        std::vector<std::string> hdrs = {"Date", "Expires", "Cache-Control", "ETag"};
        for (const auto &hdr : hdrs)
        {
            auto value = Http::CacheUtils::get_header_value(validation_response.header, hdr);
            if (value)
            {
                cached_response.header[hdr] = std::move(value.get());
            }
        }
    }
    else if ( 200 == validation_response.status_code ) {
        std::cout << "Resource has changed: " << request.url << std::endl;
        //print_response(validation_response);

        cache.Delete(request);
        cache.Put(request, validation_response);
    }
    else {
        std::cout << "Invalid cached resource:" << request.url << std::endl;
        //print_response(validation_response);

        cache.Delete(request);
    }
}

cpr::Response
Get(cpr::Session &session, CCache &cache, const CRequest &request)
{
    using namespace Era::Common::Repository;

    auto cached_response_opt = cache.Match(request);
    if (cached_response_opt)
    {
        auto &cached_response = cached_response_opt.get();

        if (Http::CacheUtils::is_response_stale(cached_response))
        {
            return validate(session, cache, request, cached_response);
        }
        else
        {
            std::cout << "Cached resource: " << request.url << std::endl;
            return cached_response.original();
        }
    }
    else
    {
        auto new_request(request);
        new_request.header["Connection"] = "Keep-Alive";

        auto response = Http::Get(session, new_request);

        if ( 200 == response.status_code ) {
            std::cout << "New resource: " << new_request.url << std::endl;
            //print_response(response);

            if (Http::CacheUtils::is_response_cacheable(response))
            {
                cache.Put(new_request, response);
            }
        }
        else {
            std::cout << "Invalid resource: " << request.url << std::endl;
            //print_response(response);
        }

        return response;
    }
}

}}}} // namespace Era::Common::Repository::Http;

int main(int argc, char** argv) {
    using namespace Era::Common::Repository;

    std::vector<Http::CRequest> requests = {
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/info.meta"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/eea/linux/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/eea/mac/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/eea/windows/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/ees/android/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/ees/mac/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/ees/windows/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/efs/windows/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/efs/windows_core/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/ems/exchange/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/ems/kerio/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/ems/lotus/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/era/agent/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/era/proxy/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/era/rdsensor/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/era/server/windows/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/era/webconsole/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/es/linux/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/es/ms_sharepoint/metadata"} ),
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/eslc/linux/metadata"} )/*,
        Http::MakeRequest( cpr::Url{"http://repository.eset.com/v1/com/eset/apps/business/era/vah/metadata"} )*/
    };

    boost::filesystem::path path("/tmp/httpcache");

    boost::system::error_code ec;
    boost::filesystem::remove_all(path, ec);

    Http::CCache cache(path);

    cpr::Session session;

    auto start = std::chrono::system_clock::now();
    for (auto &request : requests) Http::Get(session, cache, request);

    auto fresh_sync = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);

    start = std::chrono::system_clock::now();
    for (auto &request : requests) Http::Get(session, cache, request);

    auto cached_sync = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);

    auto ratio = 100. * cached_sync.count() / fresh_sync.count();

    std::cout << "Fresh sync: " << fresh_sync.count() << " ms; ";
    std::cout << "Cached sync: " << cached_sync.count() << " ms (" << ratio << "%)" << std::endl;
}
