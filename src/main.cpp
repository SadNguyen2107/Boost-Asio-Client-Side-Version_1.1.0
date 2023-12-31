#include "../include/Client.h"

using namespace SN_Client;

// Handle The Signal
void HandleSignal(int signal);

// The Client
std::shared_ptr<Client> client;

int main(int argc, char const *argv[])
{
    // Make A Signal To Turn of the Client
    std::signal(SIGINT, HandleSignal);

    // Make A Client Object
    client = std::make_shared<Client>("172.29.163.214", 6969);

    // Start The Client
    client->Start();

    // Send something
    // client->SendTextBasedFile("decode_temp.txt");
    // client->SendBinaryFile("hello.png");
    client->SendText("Hello World");

    while (client->IsRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Client is Running!" << std::endl
                  << "In PID: " << getpid() << std::endl;
    }

    // Stop the Client
    client->Stop();

    return 0;
}

void HandleSignal(int signal)
{
     std::string statement;
    switch (signal)
    {
    case SIGINT:
        statement = std::format("Received signal {0}. Initiating graceful shutdown.\n", signal);
        write(STDOUT_FILENO, statement.c_str(), statement.size());
        
        // Call The Server To Stop
        client->Stop();
        break;

    case SIGTERM:
        write(STDOUT_FILENO, "Terminate Called!\n", 19);
        break;

    case SIGTSTP:
        write(STDOUT_FILENO, "Stop Process!\n", 15);
        break;

    case SIGSTOP:
        write(STDOUT_FILENO, "Stop Process!\n", 15);
        break;

    case SIGCONT:
        write(STDOUT_FILENO, "Continue Process\n", 18);
        break;

    default:
        break;
    }
}