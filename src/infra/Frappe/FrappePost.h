//
// Created by Numan on 15.09.2024.
//

#ifndef FRAPPEPOST_H
#define FRAPPEPOST_H
#include <future>
#include <iostream>

#include "FrappeInstance.h"

class FrappePost {
private:
    FrappeInstance& frappe_instance;

public:
    explicit FrappePost(FrappeInstance& instance) : frappe_instance(instance) {}

    [[nodiscard]] std::future<std::string> sendAsyncRequest(const std::string& endpoint, const std::string& data) const {
        return std::async(std::launch::async, [this, endpoint, data]() {
            CURL* curl = frappe_instance.createRequest("POST", endpoint, data);
            CURLcode res;
            if (curl) {
                res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                    std::cerr << "POST request failed: " << curl_easy_strerror(res) << std::endl;
                }
                curl_easy_cleanup(curl);
            }
            return std::string("POST request completed");
        });
    }
};
#endif //FRAPPEPOST_H
