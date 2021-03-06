#include "sendtoserver.h"
#include <QMessageBox>
#include <QDebug>
#include <QAbstractSocket>
#include <sstream>
#include <QHostInfo>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QNetworkInterface>
const int MINPORTNUM = 50000;
const int MAXPORTNUM = 53000;
// receive user list from server, used for chat and transfer files.
string userList = "";
string port = "";
string ip = "";
QString serverIP = "127.0.0.1"; // modify this ip of server
typedef struct
{
    string fromName;
    string msg;
}OfflineMessage;

vector<OfflineMessage>offlinev;

void Split(const string& s, vector<string>& v, const string& c);

int SendtoServer(string str, string option)
{
    QString localHostName = QHostInfo::localHostName();

    QHostInfo info = QHostInfo::fromName(localHostName);
    foreach(QHostAddress address,info.addresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            ip = address.toString().toStdString();
    }

    char sendBuf[BUFLEN], recvBuf[BUFLEN];
    string tmp = option;

    GeneratePort();
    option = option + "!" + str;
    memset(sendBuf, '\0', BUFLEN);
    strcpy(sendBuf, option.c_str());
    memset(recvBuf, '\0', BUFLEN);

    QTcpSocket *clientsocket = new QTcpSocket();
    clientsocket->connectToHost(QHostAddress(serverIP), 60000);
    clientsocket->waitForConnected();

    if(tmp == "login")
    {
        memset(sendBuf, '\0', BUFLEN);
        option = option + "+" + ip + "+" + port;
        strcpy(sendBuf, option.c_str());

        clientsocket->write(sendBuf, strlen(sendBuf)+1);
        clientsocket->waitForBytesWritten();

        clientsocket->waitForReadyRead();
        clientsocket->read(recvBuf, BUFLEN);

        if(strcmp(recvBuf, "LOGIN_SUCCESS") == 0) // login success
        {
            clientsocket->waitForReadyRead();
            clientsocket->read(recvBuf, BUFLEN);

            // recvBuf: name+-.-.-.-+xxxx+false!name+-.-.-.-+xxxxx+false!%fromName+msg!fromName+msg!
            // recvBuf: name+-.-.-.-+xxxx+false!name+-.-.-.-+xxxxx+false!%none
            string recv;
            vector<string>v;
            recv = recvBuf;
            Split(recv, v, "%");
            userList.clear();
            userList = v[0];
            if(v[1] == "none")
            {
                return LOGIN_SUCCESS;
            }
            else
            {
                offlinev.clear();
                vector<string>fromName_msg;
                Split(v[1], fromName_msg, "!");
                for(auto it = fromName_msg.begin(); it!=fromName_msg.end(); it++)
                {
                    vector<string>e;
                    Split(*it, e, "+");
                    OfflineMessage tmp = { e[0], e[1] };
                    offlinev.push_back(tmp);
                }
                return LOGIN_SUCCESS;
            }
        }
        else if(strcmp(recvBuf, "INVALID_USERNAME") == 0)
        {
            return INVALID_USERNAME;
        }
        else if(strcmp(recvBuf, "PSW_ERROR") == 0)
        {
            return PSW_ERROR;
        }
        else if(strcmp(recvBuf, "FILE_ERROR") == 0)
        {
            return FILE_ERROR;
        }
    }
    else if(tmp == "register") // test OK
    {
        clientsocket->write(sendBuf, strlen(sendBuf)+1);
        clientsocket->waitForBytesWritten();

        clientsocket->waitForReadyRead();
        clientsocket->read(recvBuf, BUFLEN);

        if(strcmp(recvBuf, "REGISTER_SUCCESS") == 0) // login success
        {
            return REGISTER_SUCCESS;
        }
        else if(strcmp(recvBuf, "REGISTER_FAILURE") == 0)
        {
            return REGISTER_FAILURE;
        }
    }
    else if(tmp == "getpasswdback")
    {
        clientsocket->write(sendBuf, strlen(sendBuf)+1);
        clientsocket->waitForBytesWritten();

        clientsocket->waitForReadyRead();
        clientsocket->read(recvBuf, BUFLEN);

        if(strcmp(recvBuf, "GETPASSWDBACK_SUCCESS") == 0) // login success
        {
            return GETPASSWDBACK_SUCCESS;
        }
        else if(strcmp(recvBuf, "GETPASSWDBACK_FAILURE") == 0)
        {
            return GETPASSWDBACK_FAILURE;
        }
    }
    clientsocket->disconnectFromHost();
    if (clientsocket->state() == QAbstractSocket::UnconnectedState || clientsocket->waitForDisconnected(1000))
        qDebug("Disconnected!");
    else
        qDebug("Disconnect faild!");
    return 0;
}

void GeneratePort()
{
    srand((unsigned)time(0));
    int randomport = rand() % (MAXPORTNUM - MINPORTNUM + 1) + MINPORTNUM;
    std::stringstream ss;
    ss << randomport;
    ss >> port;
}

void Split(const string& s, vector<string>& v, const string& c)
{
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));
}
