#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QMutex>

#include <Document.h>
#include "Client.h"
#include "MessageHandler.h"
#include "ServerException.h"

#define DOCUMENT_SAVE_TIMEOUT 5000	/* ms */


class TcpServer;

class WorkSpace : public QObject
{
	Q_OBJECT

	friend class MessageHandler;

private:

	QSharedPointer<Document> doc;
	QSharedPointer<QThread> workThread;
	QMap<QTcpSocket*, QSharedPointer<Client>> editors;

	QTimer timer;

	MessageHandler messageHandler;

	QMutex& users_mutex;

public:

	WorkSpace(QSharedPointer<Document> d, QMutex& m, QObject* parent = 0);
	~WorkSpace();

public slots:

	void newClient(QSharedPointer<Client> client);
	void clientDisconnection();
	void readMessage();
	
	void documentSave();
	void documentInsertSymbol(Symbol& symbol);
	void documentDeleteSymbol(QVector<qint32> position);

	void dispatchMessage(MessageCapsule message, QTcpSocket* sender);

	MessageCapsule updateAccount(QTcpSocket* clientSocket, QString nickname, QImage icon, QString password);
	void clientQuit(QTcpSocket* clientSocket);

signals: void noEditors(URI documentURI);
signals: void returnClient(QSharedPointer<Client> client);
signals: void restoreUserAvaiable(QString username);
};

