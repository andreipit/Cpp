#include "SocketConnectionPoint.h"
#include "SocketWrapperLib/SocketWrapperShared.h"
#include "ChatException.hpp"
#include <mutex>

struct SocketConnectionPoint::impl {
    TCPSocketPtr clientSocket;
    std::mutex socketMutex;
};

const int GOOD_SEGMENT_SIZE = 1300;

SocketConnectionPoint::SocketConnectionPoint(ILoggerPtr logger)
{
    SocketUtil::StaticInit();
    m_pimpl = std::make_unique<impl>();
    m_logger = logger;
}

SocketConnectionPoint::~SocketConnectionPoint()
{
    disconnect();
    SocketUtil::CleanUp();
}

void SocketConnectionPoint::accept(std::string connectInfo)
{
    m_logger->LogInfo("SocketConnectionPoint::accept");
    TCPSocketPtr listenSocket = SocketUtil::CreateTCPSocket(INET);
    SocketAddress receivingAddress(INADDR_ANY, 48000);
    if (listenSocket->Bind(receivingAddress) != NO_ERROR)
    {
        m_status = CpStatus::ConnectionError;
        m_logger->LogError("Error: listenSocket->Bind()");
        return;
    }
    if(listenSocket->Listen() != NO_ERROR)
    {
        m_status = CpStatus::ConnectionError;
        m_logger->LogError("Error: listenSocket->Listen()");
        return;
    }

    m_logger->LogInfo("Waiting client to connection");
    SocketAddress newClientAddress;
    m_pimpl->clientSocket = listenSocket->Accept(newClientAddress);
    m_logger->LogInfo(std::string("Connected client") + newClientAddress.ToString());
    m_pimpl->clientSocket->SetNonBlockingMode(true); // TODO
    m_status = CpStatus::Connected;
}

void SocketConnectionPoint::connect(std::string connectInfo)
{
    m_logger->LogInfo("SocketConnectionPoint::connect");
    SocketAddressPtr newClientAddress = SocketAddressFactory::CreateIPv4FromString(connectInfo);
    if(!newClientAddress)
    {
        throw ChatException("Error CreateIPv4FromString in SocketConnectionPoint::connect");
    }
    m_pimpl->clientSocket = SocketUtil::CreateTCPSocket(INET);
    m_logger->LogInfo("start connection");
    m_pimpl->clientSocket->Connect(*newClientAddress);
    m_pimpl->clientSocket->SetNonBlockingMode(true); // TODO
    m_status = CpStatus::Connected;
    m_logger->LogInfo("connection successful");
}


void SocketConnectionPoint::send(std::string msg)
{
    std::lock_guard<std::mutex> lg(m_pimpl->socketMutex);
    msg.resize(GOOD_SEGMENT_SIZE-1);
    msg += '\0';
    const char * segment = msg.c_str();
    int bytesSent = m_pimpl->clientSocket->Send(segment, GOOD_SEGMENT_SIZE);
    //if(bytesSent < 0)
    //{
    //    //m_status = CpStatus::ConnectionError;
    //    m_logger->LogError("Connection error during SocketConnectionPoint::send. Error=" + std::to_string(-bytesSent));
    //}
}

std::string SocketConnectionPoint::receive()
{
    std::lock_guard<std::mutex> lg(m_pimpl->socketMutex);
    char segment[GOOD_SEGMENT_SIZE] = {};
    int bytesReceived = m_pimpl->clientSocket->Receive(segment, GOOD_SEGMENT_SIZE);
    if (bytesReceived < 0)
    {
        //m_status = CpStatus::ConnectionError;
        m_logger->LogError("Connection error during SocketConnectionPoint::receive. Error=" + std::to_string(-bytesReceived));
    }
    return segment;
}

void SocketConnectionPoint::disconnect()
{
    m_status = CpStatus::Disconnected;
    m_pimpl->clientSocket.reset();
}

CpStatus SocketConnectionPoint::getStatus()
{
    return m_status;
}