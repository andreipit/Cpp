#include "DefaultFactory.h"
#include "SocketConnectionPoint.h"
#include "DefaultUI.h"
#include "DefaultLogger.h"
#include "PipeConnectionPoint.h"

IConnectionPointPtr DefaultFactory::createConnectionPoint(std::string msg, ILoggerPtr logger)
{
    return std::make_shared<PipeConnectionPoint>(logger);
}

ILoggerPtr DefaultFactory::createLogger(std::string logFilePath)
{
    return std::make_shared<DefaultLogger>(logFilePath);
}

IUserInterfacePtr DefaultFactory::createUserInterface(std::vector<std::string> cmdLines, ILoggerPtr logger)
{
    return std::make_shared<DefaultUI>(cmdLines, logger);
}
