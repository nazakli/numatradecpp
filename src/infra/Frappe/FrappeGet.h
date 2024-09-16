//
// Created by Numan on 15.09.2024.
//

#ifndef FRAPPEGET_H
#define FRAPPEGET_H
#include <future>
#include <string>
#include <iostream>

#include "FrappeInstance.h"

class FrappeGet {
private:
    FrappeInstance& frappe_instance;

public:
    explicit FrappeGet(FrappeInstance& instance) : frappe_instance(instance) {}

    [[nodiscard]] std::future<std::string> sendAsyncRequest(const std::string& endpoint) const {
        return std::async(std::launch::async, [this, endpoint]() {
            CURL* curl = frappe_instance.createRequest("GET", endpoint);
            CURLcode res;
            if (curl) {
                res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                    std::cerr << "GET request failed: " << curl_easy_strerror(res) << std::endl;
                }
                curl_easy_cleanup(curl);
            }
            return std::string("GET request completed");
        });
    }
};

#endif //FRAPPEGET_H
