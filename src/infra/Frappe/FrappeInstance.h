//
// Created by Numan on 15.09.2024.
//

#ifndef FRAPPEINSTANCE_H
#define FRAPPEINSTANCE_H
#include <future>
#include <iostream>
#include <string>
#include <curl/curl.h>

class FrappeInstance {
    std::string base_url;
    std::string api_key;
    std::string api_secret;

    static size_t writeCallback(void* contents, size_t size, const size_t nmemb, std::string* s) {
        s->append(static_cast<char*>(contents), size * nmemb);
        return size * nmemb;
    }

public:
    FrappeInstance(std::string base_url, std::string api_key, std::string api_secret)
        : base_url(std::move(base_url)), api_key(std::move(api_key)), api_secret(std::move(api_secret)) {}


    [[nodiscard]] CURL* createRequest(const std::string& method, const std::string& endpoint, const std::string& data = "") const {
        if (CURL* curl = curl_easy_init()) {
            const std::string url = base_url + "/api/method/numabo.api." + endpoint;
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

    [[nodiscard]] std::future<std::string> asyncGet(const std::string& endpoint) const {
        return std::async(std::launch::async, [this, endpoint]() {
            CURL* curl = createRequest("GET", endpoint);
            if (!curl) {
                Log::getInstance().logError("Failed to initialize CURL");
                throw std::runtime_error("Failed to initialize CURL");
            }

            std::string response_string;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

            if (const CURLcode res = curl_easy_perform(curl); res != CURLE_OK) {
                curl_easy_cleanup(curl);
                Log::getInstance().logError(curl_easy_strerror(res));
                throw std::runtime_error(curl_easy_strerror(res));
            }

            curl_easy_cleanup(curl);
            return response_string;
        });
    }

    [[nodiscard]] std::future<std::string> asyncPost(const std::string& endpoint, const std::string& jsonData) const {
        return std::async(std::launch::async, [this, endpoint, jsonData]() {
            CURL* curl = createRequest("POST", endpoint, jsonData);
            if (!curl) {
                Log::getInstance().logError("Failed to initialize CURL");
                throw std::runtime_error("Failed to initialize CURL");
            }

            std::string response_string;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

            if (const CURLcode res = curl_easy_perform(curl); res != CURLE_OK) {
                curl_easy_cleanup(curl);
                Log::getInstance().logError(curl_easy_strerror(res));
                throw std::runtime_error(curl_easy_strerror(res));
            }

            curl_easy_cleanup(curl);
            return response_string;
        });
    }


    [[nodiscard]] std::future<std::string> asyncPut(const std::string& endpoint, const std::string& jsonData) const {
        return std::async(std::launch::async, [this, endpoint, jsonData]() {
            CURL* curl = createRequest("PUT", endpoint, jsonData);
            if (!curl) {
                Log::getInstance().logError("Failed to initialize CURL");
                throw std::runtime_error("Failed to initialize CURL");
            }

            std::string response_string;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                curl_easy_cleanup(curl);
                Log::getInstance().logError(curl_easy_strerror(res));
                throw std::runtime_error(curl_easy_strerror(res));
            }

            curl_easy_cleanup(curl);
            return response_string;
        });
    }


    [[nodiscard]] std::future<std::string> asyncDelete(const std::string& endpoint) const {
        return std::async(std::launch::async, [this, endpoint]() {
            CURL* curl = createRequest("DELETE", endpoint);
            if (!curl) {
                Log::getInstance().logError("Failed to initialize CURL");
                throw std::runtime_error("Failed to initialize CURL");
            }

            std::string response_string;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                curl_easy_cleanup(curl);
                Log::getInstance().logError(curl_easy_strerror(res));
                throw std::runtime_error(curl_easy_strerror(res));
            }

            curl_easy_cleanup(curl);
            return response_string;
        });
    }
};

#endif //FRAPPEINSTANCE_H