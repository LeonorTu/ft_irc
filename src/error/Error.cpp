#include <Error.hpp>
#include <ConnectionManager.hpp>

void Error::catchError()
{
    try
    {
        throw;
    }
    catch(const ChannelNotCreated &e)
    {
        std::cerr << "ChannelNotCreated: " << e.what() << std::endl;
    }
    catch(const ChannelNotFound &e)
    {
        std::cerr << "ChannelNotFound: " << e.what() << std::endl;
    }
    catch(const std::out_of_range &e)
    {
        std::cerr << "Out_of_range error: " << e.what() << std::endl;
    }
    catch(const BrokenPipe &e)
    {
        std::cerr << "BrokenPipe: " << e.what() << std::endl;
    }
    catch(const SocketError &e)
    {
        std::cerr << "SocketError: " << e.what() << std::endl;
    }
    catch(const EventError &e)
    {
        std::cerr << "EventError: " << e.what() << std::endl;
    }
    catch(const MessageError &e)
    {
        std::cerr << "MessageError: " << e.what() << std::endl;
    }
    catch(const ServerError &e)
    {
        std::cerr << "ServerError: " << e.what() << std::endl;
    }
    catch(const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}