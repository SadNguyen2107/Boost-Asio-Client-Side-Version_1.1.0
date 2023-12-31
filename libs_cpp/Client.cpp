#include "../include/Client.h"
#include "../include/encode_decode_base64.h"
#include <boost/filesystem.hpp>
#include <algorithm>
#include <fstream>


using namespace JB_Encode_Decode_Base64;

namespace SN_Client
{
    Client::Client(std::string server_address, std::uint16_t server_port) : strand(io_context)
    {
        // Setup Phase
        // Create A Client Socket
        this->client_socket = std::make_shared<boost::asio::ip::tcp::socket>(this->io_context);

        // Make A Server Endpoint to connect to Where
        this->server_endpoint = boost::asio::ip::tcp::endpoint(
            boost::asio::ip::address_v4::from_string(server_address),
            server_port
        );
    }

    Client::~Client()
    {
        this->Stop();
    }

    void Client::Start()
    {
        // Start The Client
        this->is_running = true;

        // Start The I/O
        this->io_context.run();

        // Connect The client_socket with the Server_endpoint
        this->client_socket->connect(server_endpoint);

        // Show A Log Of Set up Client
        std::cout << "Connect to Server: "
                  << server_endpoint.address() << ":"
                  << server_endpoint.port() << std::endl;
    }

    void Client::Stop()
    {
        // Change The Status of the Client
        this->is_running = false;

        // Close The Connection
        this->client_socket->close();

        // Stop the I/O
        this->io_context.stop();
        this->io_context.reset();

        std::cout << "Shutdown The Client!" << std::endl;
    }

    bool Client::IsRunning()
    {
        return this->is_running && client_socket->is_open();
    }

    void Client::SetChunkData(std::size_t new_chunk_size)
    {
        if (new_chunk_size > 0)
        {
            this->CHUNK_SIZE = new_chunk_size;
        }
    }

    std::size_t Client::GetChunkData() const
    {
        return this->CHUNK_SIZE;
    }

    /**
     * @brief Change A New End Signal to send to the Client
     * Default: |end
     * 
     * @param end_signal new end_signal
     */
    void Client::SetEndSignal(const std::string_view& end_signal)
    {
        if (end_signal != "")
        {
            this->end_signal = end_signal;
        }
    }

    /**
     * @brief Get the current end_signal
     * 
     * @return std::string_view return the end_signal
     */
    std::string_view Client::GetEndSignal() const
    {
        return this->end_signal;
    }

    /**
     * @brief Call This Send A End Signal to the Server
     * 
     */
    void Client::SendEndSignal()
    {
        // Error If Thrown
        boost::system::error_code error;

        //! Send Signal To end
        boost::asio::write(
            *client_socket,
            boost::asio::buffer(this->end_signal),
            error
        );

        // Check Whether Error Happen
        if (error)
        {
            std::cerr << "Error: " << error.message() << std::endl;
        }
    }

    /**
     * @brief Send Text Synchrounsly to Server
     * 
     * @param text the Text to send
     */
    void Client::SendText(const std::string_view &text)
    {
        // The Variable To check For The Bytes Have Send
        std::size_t total_sent = 0;

        // Error If Thrown
        boost::system::error_code error;

        // Send every CHUNK_SIZE character per send
        for (size_t index = 0; index < text.size(); index += this->CHUNK_SIZE)
        {
            std::string_view chunk = text.substr(index, this->CHUNK_SIZE);

            // Synchronous write
            std::size_t bytes_sent = boost::asio::write(
                *client_socket, 
                boost::asio::buffer(chunk), 
                error
            );

            // Check Whether Error Happen
            if (!error)
            {
                total_sent += bytes_sent;
                std::cout << "Message sent successfully to "
                          << client_socket->remote_endpoint().address() << ":"
                          << client_socket->remote_endpoint().port()
                          << std::endl;
            }
            else
            {
                std::cerr << "Error: " << error.message() << std::endl;
                break; // Break The Loop On Error
            }
        }

        // Check If All data has been sent
        if (total_sent == text.size())
        {
            std::cout << " All data sent successfully!" << std::endl;
        }
        else
        {
            std::cerr << "Not all data sent. Total sent: " << total_sent << " bytes out of " << text.size() << " bytes." << std::endl;
        }

        //! Send an end signal
        this->SendEndSignal();
    }

    /**
     * @brief Call This Function within client object to send Synchrously a Text-Based Formats \n
     * For Sending Files ends with: \n
     * 1. Plain Text Files (.txt) \n
     * 2. JSON Files (.json) \n
     * 3. XML Files (.xml) \n
     * 4. CSV Files (.csv) \n
     * 5. TCV Files (.tsx) \n
     * And Many More...
     *
     * @param file_to_send The file directory to send
     */
    void Client::SendTextBasedFile(const std::string &file_to_send)
    {
        // Open The File JSON and Load every 255 Character To Send
        std::ifstream text_file(file_to_send, std::ios::binary);

        // Check If the File Open Successfully
        if (!text_file.is_open())
        {
            std::cerr << "Error: Unable to open TEXT file " << file_to_send << std::endl;
            return;
        }

        // Cut Into Chunk of Data to send
        std::string chunk;

        // The Variable To check For The Bytes Have Send
        std::size_t total_sent = 0;

        // Error if Thrown
        boost::system::error_code error;

        while (std::getline(text_file, chunk, static_cast<char>(EOF)) && !chunk.empty())
        {
            // Synchronous write
            std::size_t bytes_sent = boost::asio::write(*client_socket, boost::asio::buffer(chunk), error);

            // Check Whether Error Happen
            if (!error)
            {
                total_sent += bytes_sent;
                std::cout << "Message chunk sent successfully to "
                          << client_socket->remote_endpoint().address() << ":"
                          << client_socket->remote_endpoint().port()
                          << std::endl;
            }
            else
            {
                std::cerr << "Error: " << error.message() << std::endl;
                break; // Break the loop on error
            }
        }

        // Check for end-of-file or error
        if (text_file.eof())
        {
            std::cout << "All data sent successfully!" << std::endl;
        }
        else if (text_file.fail())
        {
            std::cerr << "Error reading binary file: " << file_to_send << std::endl;
        }

        // Close the File after Sending Successfully
        text_file.close();

        // Check If All data has been sent
        if (total_sent == boost::filesystem::file_size(file_to_send))
        {
            std::cout << " All data sent successfully!" << std::endl;
        }
        else
        {
            std::cerr << "Not all data sent. Total sent: " << total_sent << " bytes out of "
                      << boost::filesystem::file_size(file_to_send) << " bytes." << std::endl;
        }

        //! Send an end signal
        this->SendEndSignal();
    }

    /**
     * @brief Call This Function within client object to send Synchrously a Binary File Formats \n
     * For Sending Files likes: \n
     * 1. Image Files (.jpg, .png, .gif, etc.) \n
     * 2. Binary Documents (.pdf, .docx, .xlsx, etc.) \n
     * 3. Executable Files (.exe) \n
     * 4. Compressed Archives (.zip, .tar, .gz, etc.) \n
     *
     * @param client_socket The client_socket to send the File
     * @param file_to_send The file directory to send
     */
    void Client::SendBinaryFile(const std::string &file_to_send)
    {
        //! Approach 1: Encoding Base 64
        // Create A temp file with subfix .txt
        const std::string temp_file_directory = createTempFile(file_to_send);

        // Encode Binary File into temp file
        encodeFileToFile(file_to_send, temp_file_directory);

        // Then send that temp file
        this->SendTextBasedFile(temp_file_directory);

        // Delete that temp file afterward
        int result = std::remove(temp_file_directory.c_str());

        // Check If delete success
        if (result == 0)
        {
            std::cout << "File '" << temp_file_directory << "' deleted successfully." << std::endl;
        }
        else
        {
            std::cerr << "Error deleting file" << std::endl;
        }
    }
} // namespace SN_Client
