#pragma once

#include <QObject>
#include <list>
#include <queue>
#include <mutex>

#include "Document.h"
#include "Client.h"
#include "Message.h"
#include "TextEditMessage.h"
#include "ServerException.h"


class TcpServer;

class WorkSpace : public QObject
{
	Q_OBJECT
private:
	QSharedPointer<Document> doc;
	QSharedPointer<TcpServer> server;
	QMap<QTcpSocket *, QSharedPointer<Client>> editors;

	void handleMessage(std::unique_ptr<Message>&& msg, QTcpSocket* socket);

public:
	WorkSpace(QSharedPointer<Document> d, QSharedPointer<TcpServer> server);
	~WorkSpace();

public slots:
	void newSocket(qint64 handle);
	void clientDisconnection();
	void readMessage();

signals:
	void notWorking(QString document);
signals:
	void deleteClient(qint64 handle);
};

