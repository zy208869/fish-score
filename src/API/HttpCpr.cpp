// HttpCpr.cpp
#include "HttpCpr.h"

namespace FishScore {

HttpCpr::HttpCpr()
    : mDefaultHeaders{{"User-Agent", "FishScore-LeviMina/1.0"}, {"Accept", "application/json"}}
    , mDefaultTimeout(10000) {
}

void HttpCpr::setTimeout(int ms) {
    mDefaultTimeout = ms;
}

void HttpCpr::setDefaultHeader(const std::string& key, const std::string& value) {
    mDefaultHeaders[key] = value;
}

cpr::Response HttpCpr::get(const std::string& url, const cpr::Parameters& params, int timeoutMs) {
    return cpr::Get(
        cpr::Url{url},
        params,
        mDefaultHeaders,
        cpr::Timeout{timeoutMs > 0 ? timeoutMs : mDefaultTimeout}
    );
}

cpr::Response HttpCpr::post(const std::string& url, const cpr::Body& body, const cpr::Header& headers, int timeoutMs) {
    cpr::Header merged = mDefaultHeaders;
    merged.insert(headers.begin(), headers.end());
    return cpr::Post(
        cpr::Url{url},
        body,
        merged,
        cpr::Timeout{timeoutMs > 0 ? timeoutMs : mDefaultTimeout}
    );
}

cpr::Response HttpCpr::put(const std::string& url, const cpr::Body& body, int timeoutMs) {
    return cpr::Put(
        cpr::Url{url},
        body,
        mDefaultHeaders,
        cpr::Timeout{timeoutMs > 0 ? timeoutMs : mDefaultTimeout}
    );
}

} // namespace FishScore
