// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_NETWORK_CLIENT_HPP
#define OUZEL_NETWORK_CLIENT_HPP

#include "Socket.hpp"

namespace ouzel
{
    namespace network
    {
        class Network;

        class Client final
        {
        public:
            Client(Network& initNetwork);
            ~Client();

            Client(const Client&) = delete;
            Client& operator=(const Client&) = delete;

            Client(Client&& other);
            Client& operator=(Client&& other);

            void disconnect();

        private:
            Network* network = nullptr;
            Socket sock;
        };
    } // namespace network
} // namespace ouzel

#endif // OUZEL_NETWORK_CLIENT_HPP
