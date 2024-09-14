#ifndef IPC_H
#define IPC_H

#include <atomic>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <glaze/glaze.hpp>  // Glaze library
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/protocol/reqrep0/rep.h>

// Base struct for JSON-based messages
struct IpcMsg {
    std::string prefix;
    std::string content;

    IpcMsg() : prefix(""), content("") {}

    IpcMsg(std::string pfx, std::string cnt) : prefix(std::move(pfx)), content(std::move(cnt)) {}

    void printMessage() const {
        std::cout << "Prefix: " << prefix << std::endl;
        std::cout << "Content: " << content << std::endl;
    }
};

// Glaze reflection for IpcMsg
template <>
struct glz::meta<IpcMsg> {
    using T = IpcMsg;
    static constexpr auto value = object("prefix", &T::prefix, "content", &T::content);
};

// External JSON serialization/deserialization functions
inline std::string toJSON(const IpcMsg& msg) {
    std::string jsonStr;
    glz::write_json(msg, jsonStr);
    return jsonStr;
}

inline IpcMsg fromJSON(const std::string& jsonStr) {
    IpcMsg msg;
    glz::read_json(msg, jsonStr);
    return msg;
}

// Base class for NNG Pub/Sub and Req/Rep with Glaze-based JSON messages
class IpcBase {
protected:
    nng_socket pubSocket;
    nng_socket subSocket;
    nng_socket reqSocket;
    nng_socket repSocket;
    std::atomic<bool> running;
    bool pubInitialized;
    bool subInitialized;
    bool reqInitialized;
    bool repInitialized;

public:
    IpcBase() : running(true), pubInitialized(false), subInitialized(false), reqInitialized(false), repInitialized(false) {}

    virtual ~IpcBase() {
        stop();
    }

    // Stop all sockets
    void stop() {
        running = false;
        if (nng_socket_id(pubSocket) != -1) nng_close(pubSocket);
        if (nng_socket_id(subSocket) != -1) nng_close(subSocket);
        if (nng_socket_id(reqSocket) != -1) nng_close(reqSocket);
        if (nng_socket_id(repSocket) != -1) nng_close(repSocket);
    }

    // Initialize Publisher
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

        pubInitialized = true;
        return true;
    }

    // Initialize Subscriber
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

        subInitialized = true;
        return true;
    }

    // Initialize Requester
    bool initRequester() {
        if (reqInitialized) return true;

        int rv = nng_req0_open(&reqSocket);
        if (rv != 0) {
            std::cerr << "Failed to open REQ socket: " << nng_strerror(rv) << std::endl;
            return false;
        }

        rv = nng_dial(reqSocket, "inproc://reqrep", nullptr, 0);
        if (rv != 0) {
            std::cerr << "Failed to dial REQ socket: " << nng_strerror(rv) << std::endl;
            return false;
        }

        reqInitialized = true;
        return true;
    }

    // Initialize Replier
    bool initReplier() {
        if (repInitialized) return true;

        int rv = nng_rep0_open(&repSocket);
        if (rv != 0) {
            std::cerr << "Failed to open REP socket: " << nng_strerror(rv) << std::endl;
            return false;
        }

        rv = nng_listen(repSocket, "inproc://reqrep", nullptr, 0);
        if (rv != 0) {
            std::cerr << "Failed to listen on REP socket: " << nng_strerror(rv) << std::endl;
            return false;
        }

        repInitialized = true;
        return true;
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

        nng_msg_append(nmsg, jsonMessage.c_str(), jsonMessage.size());
        nng_sendmsg(pubSocket, nmsg, 0);

        return true;
    }

    // Subscribe to a specific prefix and receive messages using Glaze
    void subIpc(const std::string& prefix, std::function<void(const IpcMsg&)> callback) {
        if (!subInitialized && !initSubscriber()) {
            std::cerr << "Error: Failed to initialize subscriber" << std::endl;
            return;
        }

        nng_setopt(subSocket, NNG_OPT_SUB_SUBSCRIBE, prefix.c_str(), prefix.size());

        std::thread([this, callback]() {
            while (running) {
                nng_msg* msg;
                if (nng_recvmsg(subSocket, &msg, 0) == 0) {
                    std::string received_json((char*)nng_msg_body(msg), nng_msg_len(msg));
                    IpcMsg baseMsg = fromJSON(received_json);
                    callback(baseMsg);
                    nng_msg_free(msg);
                }
            }
        }).detach();
    }

    // Send request and receive reply (Requester)
    bool reqIpc(const IpcMsg& msg, std::function<void(const IpcMsg&)> callback) {
        if (!reqInitialized && !initRequester()) {
            std::cerr << "Error: Failed to initialize requester" << std::endl;
            return false;
        }

        std::string jsonMessage = toJSON(msg);

        nng_msg* nmsg;
        if (nng_msg_alloc(&nmsg, 0) != 0) {
            return false;
        }

        nng_msg_append(nmsg, jsonMessage.c_str(), jsonMessage.size());
        nng_sendmsg(reqSocket, nmsg, 0);

        // Separate thread for receiving the reply
        std::thread([this, callback]() {
            nng_msg* rep_msg;
            if (nng_recvmsg(reqSocket, &rep_msg, 0) == 0) {
                std::string reply_json((char*)nng_msg_body(rep_msg), nng_msg_len(rep_msg));
                IpcMsg replyMsg = fromJSON(reply_json);
                callback(replyMsg);
                nng_msg_free(rep_msg);
            }
        }).detach();

        return true;
    }

    // Listen for requests and send replies (Replier)
    void repIpc(std::function<IpcMsg(const IpcMsg&)> callback) {
        if (!repInitialized && !initReplier()) {
            std::cerr << "Error: Failed to initialize replier" << std::endl;
            return;
        }

        // Separate thread for listening to requests
        std::thread([this, callback]() {
            while (running) {
                nng_msg* req_msg;
                if (nng_recvmsg(repSocket, &req_msg, 0) == 0) {
                    std::string request_json((char*)nng_msg_body(req_msg), nng_msg_len(req_msg));
                    IpcMsg requestMsg = fromJSON(request_json);
                    IpcMsg replyMsg = callback(requestMsg);

                    nng_msg* nmsg;
                    nng_msg_alloc(&nmsg, 0);
                    std::string reply_json = toJSON(replyMsg);
                    nng_msg_append(nmsg, reply_json.c_str(),

 reply_json.size());
                    nng_sendmsg(repSocket, nmsg, 0);

                    nng_msg_free(req_msg);
                }
            }
        }).detach();
    }
};

#endif //IPC_H