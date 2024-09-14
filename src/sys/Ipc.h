//
// Created by Numan on 10.09.2024.
//

#ifndef IPC_H
#define IPC_H
#include <string>
#include <glaze/glaze.hpp>  // Glaze library
#include <iostream>
#include <string>
#include <atomic>
#include <functional>
#include <utility>
#include <glaze/glaze.hpp>  // Glaze library
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>

// Base struct for JSON-based messages
struct IpcMsg {
    std::string prefix;  // The message prefix (e.g., "ALRT", "INFO")
    std::string content;  // The message content in JSON format as a string

    // Default constructor (this allows empty IpcMsg objects to be created)
    IpcMsg() : prefix(""), content("") {}

    // Constructor to set the prefix and content
    IpcMsg(std::string  pfx, std::string  cnt) : prefix(std::move(pfx)), content(std::move(cnt)) {}

    // Function to print the message (for debugging)
    void printMessage() const {
        std::cout << "Prefix: " << prefix << std::endl;
        std::cout << "Content: " << content << std::endl;
    }
};

// Glaze reflection to enable JSON serialization/deserialization for IpcMsg
template <>
struct glz::meta<IpcMsg> {
    using T = IpcMsg;
    static constexpr auto value = object("prefix", &T::prefix, "content", &T::content);
};

// External functions for JSON serialization/deserialization
inline std::string toJSON(const IpcMsg& msg) {
    std::string jsonStr;
    glz::write_json(msg, jsonStr);  // Glaze serialization
    return jsonStr;
}

inline IpcMsg fromJSON(const std::string& jsonStr) {
    IpcMsg msg;  // Default constructor allows empty initialization
    glz::read_json(msg, jsonStr);  // Glaze deserialization
    return msg;
}

// Base class for NNG Pub/Sub with Glaze-based JSON messages
class IpcBase {
protected:
    nng_socket pubSocket;     // Socket for publishing
    nng_socket subSocket;     // Socket for subscribing
    nng_aio* pubAio;          // AIO for publishing
    nng_aio* subAio;          // AIO for subscribing
    std::atomic<bool> running;
    bool pubInitialized;      // Track if the publisher is initialized
    bool subInitialized;      // Track if the subscriber is initialized

public:
    IpcBase() : running(true), pubAio(nullptr), subAio(nullptr), pubInitialized(false), subInitialized(false) {}

    virtual ~IpcBase() {
        stop();
    }

    // Initialize the publisher socket and setup AIO
    bool initPublisher() {
        if (pubInitialized) return true;

        int rv = nng_pub0_open(&pubSocket);
        if (rv != 0) {
            std::cerr << "Failed to open PUB socket: " << nng_strerror(rv) << std::endl;
            return false;
        }

        rv = nng_listen(pubSocket, "inproc://global", nullptr, 0);
        if (rv != 0) {
            std::cerr << "Failed to listen on PUB socket: " << nng_strerror(rv) << std::endl;
            return false;
        }

        if (!setupPubAIO()) {
            std::cerr << "Failed to setup Publisher AIO." << std::endl;
            return false;
        }

        pubInitialized = true;
        return true;
    }

    // Initialize the subscriber socket and setup AIO
    bool initSubscriber() {
        if (subInitialized) return true;

        int rv = nng_sub0_open(&subSocket);
        if (rv != 0) {
            std::cerr << "Failed to open SUB socket: " << nng_strerror(rv) << std::endl;
            return false;
        }

        rv = nng_dial(subSocket, "inproc://global", nullptr, 0);
        if (rv != 0) {
            std::cerr << "Failed to dial SUB socket: " << nng_strerror(rv) << std::endl;
            return false;
        }

        if (!setupSubAIO()) {
            std::cerr << "Failed to setup Subscriber AIO." << std::endl;
            return false;
        }

        subInitialized = true;
        return true;
    }

    // Setup Publisher AIO
    bool setupPubAIO() {
        int rv = nng_aio_alloc(&pubAio, nullptr, nullptr);
        return rv == 0;
    }

    // Setup Subscriber AIO
    bool setupSubAIO() {
        int rv = nng_aio_alloc(&subAio, nullptr, nullptr);
        return rv == 0;
    }

    // Stop the sockets
    void stop() {
        running = false;
        if (pubAio) {
            nng_aio_stop(pubAio);
            nng_aio_free(pubAio);
        }
        if (subAio) {
            nng_aio_stop(subAio);
            nng_aio_free(subAio);
        }
        if (nng_socket_id(pubSocket) != -1) {
            nng_close(pubSocket);
        }
        if (nng_socket_id(subSocket) != -1) {
            nng_close(subSocket);
        }
    }

    // Publish a JSON-formatted message using Glaze
    bool pubIpc(const IpcMsg& msg) {
        if (!pubInitialized && !initPublisher()) {
            std::cerr << "Error: Failed to initialize publisher" << std::endl;
            return false;
        }

        std::string jsonMessage = toJSON(msg);

        nng_msg* nmsg;
        if (nng_msg_alloc(&nmsg, 0) != 0) {
            return false;
        }

        // Append the JSON message as a string
        nng_msg_append(nmsg, jsonMessage.c_str(), jsonMessage.size());

        nng_aio_set_msg(pubAio, nmsg);
        nng_send_aio(pubSocket, pubAio);

        return true;
    }

    // Subscribe to a specific prefix and receive messages using Glaze
    void subIpc(const std::string& prefix, std::function<void(const IpcMsg&)> callback) {
        if (!subInitialized && !initSubscriber()) {
            std::cerr << "Error: Failed to initialize subscriber" << std::endl;
            return;
        }

        // Subscribe to the prefix (this adds a prefix filter)
        nng_setopt(subSocket, NNG_OPT_SUB_SUBSCRIBE, prefix.c_str(), prefix.size());

        // Wait for messages
        nng_recv_aio(subSocket, subAio);
        nng_aio_wait(subAio);

        nng_msg* msg = nng_aio_get_msg(subAio);
        if (msg) {
            // Extract the message content as a string
            std::string received_json((char*)nng_msg_body(msg), nng_msg_len(msg));

            // Deserialize the JSON string into a IpcMsg object using Glaze
            IpcMsg baseMsg = fromJSON(received_json);

            // Invoke the callback with the deserialized message
            callback(baseMsg);

            nng_msg_free(msg);
        }
    }
};

/*
int main() {
    // Create an instance of IpcBase
    IpcBase pubSub;

    // Create a JSON message with a prefix and content
    IpcMsg alertMsg("ALRT", "{\"type\": \"alert\", \"message\": \"This is an alert message\"}");

    // Publish the message
    pubSub.publish(alertMsg);

    // Subscribe to messages with the prefix "ALRT" and process them
    pubSub.subscribe("ALRT", [](const IpcMsg& msg) {
        msg.printMessage();
    });

    // Wait for a while to let messages be processed
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Cleanup
    pubSub.stop();

    return 0;
}*/
#endif //IPC_H
