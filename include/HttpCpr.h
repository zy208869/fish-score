// HttpCpr.h - 基于 cpr 的 HTTP 客户端封装
#pragma once

#include <string>
#include <optional>
#include <cpr/cpr.h>

namespace FishScore {

class HttpCpr {
public:
    HttpCpr();
    ~HttpCpr() = default;

    // 设置超时（毫秒）
    void setTimeout(int ms);
    // 设置请求头
    void setDefaultHeader(const std::string& key, const std::string& value);

    // GET 请求
    cpr::Response get(const std::string& url,
                      const cpr::Parameters& params = {},
                      int timeoutMs = 10000);

    // POST 请求
    cpr::Response post(const std::string& url,
                       const cpr::Body& body = {},
                       const cpr::Header& headers = {},
                       int timeoutMs = 10000);

    // PUT 请求
    cpr::Response put(const std::string& url,
                      const cpr::Body& body = {},
                      int timeoutMs = 10000);

private:
    cpr::Header mDefaultHeaders;
    int mDefaultTimeout;
};

} // namespace FishScore
