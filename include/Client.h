#ifndef CLIENT_H
#define CLIENT_H

#include <utility> // Include this line before Boost.Asio headers
#include <boost/asio.hpp>
#include <stdint.h>
#include <thread>
#include <atomic>
#include <memory>
#include <iostream>

namespace SN_Client
{
    class Client : public std::enable_shared_from_this<Client>
    {
    private:
        //* Create io_context for Client
        boost::asio::io_context io_context;

        // Strand For I/O 
        boost::asio::io_context::strand strand;

        // The main Client_socket to send/get data to Server
        std::shared_ptr<boost::asio::ip::tcp::socket> client_socket;

        // Server Endpoint Object
        boost::asio::ip::tcp::endpoint server_endpoint;

        // To Concurrently shutdown
        std::atomic<bool> is_running;

        // Chunk Size of Data to Send/Get
        std::size_t CHUNK_SIZE = 255;

        // End Signal of the Text
        std::string_view end_signal = "|end";
    public:
        Client(std::string server_address = "127.0.0.1", std::uint16_t server_port = 5000);
        ~Client();

        // Simple I/O To Start and Stop
        void Start();
        void Stop();

        bool IsRunning();

        // Set-Get The Chunk of data
        void SetChunkData(std::size_t new_chunk_size);
        std::size_t GetChunkData() const;

        // Set-Get End Signal of the data
        void SetEndSignal(const std::string_view& end_signal);
        std::string_view GetEndSignal() const;

        //========================================================================================================================
        //! IMPORTANT: Send End Signal
        void SendEndSignal();
        
        // Simple I/O Send Protocol
        void SendText(const std::string_view &text);

        // For Sending Text-Based Formats Files
        void SendTextBasedFile(const std::string &file_to_send);

        // For Sending Binary Formats Files
        void SendBinaryFile(const std::string &file_to_send);

        //========================================================================================================================
        // Simple I/O Get Protocol
        void GetText(std::string &received_text);

        // For Receiving Text-Based Formats Files
        void GetTextBasedFile(const std::string &file_to_store);

        // For Receiving Binary Formats Files
        void GetBinaryFile(const std::string &file_to_store);
    };
} // namespace SN_Client

#endif