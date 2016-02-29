#include <iostream>

#include <cpr/cpr.h>
#include <json.hpp>

#include "cache/Cache.h"
#include "cache/CacheUtils.h"

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
validate(cpr::Session &session,
         Era::Common::Repository::Http::CCache &cache,
         const Era::Common::Repository::Http::CRequest &request,
         Era::Common::Repository::Http::CCachedResponse &cached_response)
{
    using namespace Era::Common::Repository;

    auto validation_request = make_validation_request(request, cached_response);
    validation_request.header["Connection"] = "Keep-Alive";

    cpr::priv::set_option(session,
                          validation_request.url,
                          validation_request.header,
                          validation_request.parameters);

    auto validation_response = session.Get();

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

cpr::Response get_with_cache(cpr::Session &session,
                             Era::Common::Repository::Http::CCache &cache,
                             const Era::Common::Repository::Http::CRequest &request)
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
    else {
        cpr::priv::set_option(session, request.url,
                                       cpr::Header{ { "Connection", "Keep-Alive" } } );
        auto response = session.Get();

        if ( 200 == response.status_code ) {
            std::cout << "New resource: " << request.url << std::endl;
            //print_response(response);

            cache.Put(request, response);
        }
        else {
            std::cout << "Invalid resource: " << request.url << std::endl;
            //print_response(response);
        }

        return response;
    }
}

int main(int argc, char** argv) {
    using namespace Era::Common::Repository;

    Http::CCache cache("/tmp/httpcache");

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

    cpr::Session session;

    auto start_stamp = std::chrono::system_clock::now();
    for (auto &request : requests)
    {
        get_with_cache(session, cache, request);
    }
    auto end_stamp = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_stamp - start_stamp);

    std::cout << "Sync took " << duration.count() << " milliseconds" << std::endl;
}
