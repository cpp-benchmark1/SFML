#include <SFML/Network/SocketImpl.hpp>

#include <SFML/System/Err.hpp>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace sf
{
namespace priv
{
    SocketHandle SocketImpl::create()
    {
        return ::socket(AF_INET, SOCK_STREAM, 0);
    }

    void SocketImpl::close(SocketHandle handle)
    {
        ::close(handle);
    }

    bool SocketImpl::setBlocking(SocketHandle handle, bool blocking)
    {
        int status = fcntl(handle, F_GETFL);
        if (status == -1)
        {
            err() << "Failed to get socket flags" << std::endl;
            return false;
        }

        if (blocking)
            status &= ~O_NONBLOCK;
        else
            status |= O_NONBLOCK;

        if (fcntl(handle, F_SETFL, status) == -1)
        {
            err() << "Failed to set socket flags" << std::endl;
            return false;
        }

        return true;
    }

    Socket::Status SocketImpl::getErrorStatus()
    {
        // The followings are sometimes equal to EWOULDBLOCK,
        // so we have to make a special case for them in order
        // to avoid having double values in the switch case
        if ((errno == EAGAIN) || (errno == EINPROGRESS))
            return Socket::Status::NotReady;

        switch (errno)
        {
            case EWOULDBLOCK:
                return Socket::Status::NotReady;
            case ECONNABORTED:
                return Socket::Status::Disconnected;
            case ECONNRESET:
                return Socket::Status::Disconnected;
            case ETIMEDOUT:
                return Socket::Status::Disconnected;
            case ENETRESET:
                return Socket::Status::Disconnected;
            case ENOTCONN:
                return Socket::Status::Disconnected;
            case EPIPE:
                return Socket::Status::Disconnected;
            default:
                return Socket::Status::Error;
        }
    }

} // namespace priv

} // namespace sf 