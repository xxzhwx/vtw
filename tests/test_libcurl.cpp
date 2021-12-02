#include "tests/test.h"
#include <map>
#include <vector>
#include <string>

extern "C" {
#include "curl/curl.h"
}

enum class HttpVersion
{
    Http1_0 = CURL_HTTP_VERSION_1_0,
    Http1_1 = CURL_HTTP_VERSION_1_1,
    Http2_0 = CURL_HTTP_VERSION_2_0,
    Http2_0_TLS = CURL_HTTP_VERSION_2TLS,
};

struct HttpResponse
{
    long statusCode;
    long httpVersion;
    std::vector<std::string> headers;
    std::string content;

    HttpResponse()
        : statusCode(0)
        , httpVersion(0)
        , headers()
        , content()
    {
    }

    std::string ToString()
    {
        std::string str;

        str.append("Status Code:");
        str.append(std::to_string(statusCode));
        str.append("\n");
        str.append("Http Version:");
        str.append(std::to_string(httpVersion));
        str.append("\n");

        str.append("Headers:\n");
        for (auto v : headers)
        {
            str.append(v);
        }
        str.append("\n");

        str.append("Content length:");
        str.append(std::to_string(content.size()));
        str.append("\n");
        str.append(content);
        return str;
    }
};

static bool checkCurlError(const char* opDesc, CURLcode code)
{
    if (code == CURLE_OK)
        return false; // no error

    printf("%s, curl code: %d, error desc: %s\n", opDesc, code, curl_easy_strerror(code));
    return true;
}

static size_t onHttpHeaderArrival(char *ptr, size_t size, size_t nitems, void *userdata)
{
    printf("onHttpHeaderArrival size:%zd, nitems:%zd\n", size, nitems);

    size_t numBytes = size * nitems;
    if (numBytes == 2 && (ptr[0] == '\r' && ptr[1] == '\n'))
        return numBytes;

    auto res = reinterpret_cast<HttpResponse *>(userdata);

    static std::string buf;
    buf.clear();
    buf.append(ptr, numBytes);

    //printf("%s\n", std::string(ptr, numBytes).c_str());
    res->headers.emplace_back(ptr, numBytes);

    return numBytes;
}

static size_t onHttpContentArrival(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    printf("onHttpContentArrival size:%zd, nmemb:%zd\n", size, nmemb);

    size_t numBytes = size * nmemb;
    auto res = reinterpret_cast<HttpResponse *>(userdata);
    res->content.append(ptr, numBytes);
    return numBytes;
}

void Test_LibCurl()
{
    auto ret = curl_global_init(CURL_GLOBAL_ALL); //curl_global_init(CURL_GLOBAL_ALL & (~CURL_GLOBAL_WIN32));
    if (checkCurlError("Initialize libcurl library", ret))
        return;

    CURL* client = curl_easy_init();

    //curl_easy_reset(client);

    // https settings
    curl_easy_setopt(client, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(client, CURLOPT_SSL_VERIFYHOST, 0);

    // set Header and Content callback
    HttpResponse res;
    curl_easy_setopt(client, CURLOPT_HEADERFUNCTION, onHttpHeaderArrival);
    curl_easy_setopt(client, CURLOPT_HEADERDATA, &res);
    curl_easy_setopt(client, CURLOPT_WRITEFUNCTION, onHttpContentArrival);
    curl_easy_setopt(client, CURLOPT_WRITEDATA, &res);

    // set request params
    curl_easy_setopt(client, CURLOPT_URL, "http://www.baidu.com");

    ret = curl_easy_perform(client);
    if (checkCurlError("Perform", ret))
        return;

    // get http status code
    long statusCode = 0;
    curl_easy_getinfo(client, CURLINFO_RESPONSE_CODE, &statusCode);
    res.statusCode = statusCode;

    // get actually used http version
    long httpVersion = 0;
    curl_easy_getinfo(client, CURLINFO_HTTP_VERSION, &httpVersion);
    res.httpVersion = httpVersion;

    printf("%s\n", res.ToString().c_str());
    FILE* file = fopen("vtw_libcurl_out.txt", "w");
    std::string str = res.ToString();
    fwrite(str.c_str(), 1, str.size(), file);
    fclose(file);

    curl_easy_cleanup(client);

    curl_global_cleanup();
}
