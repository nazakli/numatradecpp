//
// Created by Numan on 15.09.2024.
//

#ifndef FRAPPEINSTANCE_H
#define FRAPPEINSTANCE_H
#include <string>
#include <memory>
#include <mutex>
#include <utility>
#include <curl/curl.h>

class FrappeInstance {
private:
    std::string base_url;
    std::string api_key;
    std::string api_secret;

public:
    FrappeInstance(std::string  base_url, std::string  api_key, std::string  api_secret)
        : base_url(std::move(base_url)), api_key(std::move(api_key)), api_secret(std::move(api_secret)) {}

    CURL* createRequest(const std::string& method, const std::string& endpoint, const std::string& data = "") const {
        CURL* curl = curl_easy_init();
        if (curl) {
            std::string url = base_url + "/" + endpoint;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, ("Authorization: token " + api_key + ":" + api_secret).c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            if (method == "POST") {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            } else if (method == "PUT") {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            } else if (method == "DELETE") {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            }

            return curl;
        }
        return nullptr;
    }
};
#endif //FRAPPEINSTANCE_H
