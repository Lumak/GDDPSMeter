#include "IPCMessage.h"
#include "proc.h"
#include "Logger.h"

//=============================================================================
//=============================================================================
void IPCMessage::SendShortData(DataMsgType msg, unsigned int val)
{
    IPCData ipc;
    ipc.msg_ = msg;
    ipc.value_ = val;

    MessageHandler::GetQueue().QueueMessage(ipc);
}

void IPCMessage::SendData(IPCData& data)
{
    MessageHandler::GetQueue().QueueMessage(data);
}

void IPCMessage::SendData(std::vector<IPCData>& msgList)
{
    MessageHandler::GetQueue().QueueMessage(msgList);
}

void IPCMessage::ClearQueue()
{
    MessageHandler::GetQueue().ClearQueue();
}

//=============================================================================
//=============================================================================
MessageHandler MessageHandler::sMessageHandler_;

MessageHandler& MessageHandler::GetQueue()
{
    return sMessageHandler_;
}

void MessageHandler::ClearQueue()
{
    MutexLock lock(mutexMessageLock_);
    messageQueue_.clear();
}

void MessageHandler::QueueMessage(IPCData& data)
{
    MutexLock lock(mutexMessageLock_);
    messageQueue_.push_back(data);
}

void MessageHandler::QueueMessage(std::vector<IPCData> &msgList)
{
    MutexLock lock(mutexMessageLock_);

    for (unsigned i = 0; i < msgList.size(); ++i)
    {
        messageQueue_.push_back(msgList[i]);
    }
}

void MessageHandler::GetMessages(std::vector<IPCData> &veclist)
{
    MutexLock lock(mutexMessageLock_);

    for (unsigned i = 0; i < messageQueue_.size(); ++i)
    {
        veclist.push_back(messageQueue_[i]);
    }
    messageQueue_.clear();
}

