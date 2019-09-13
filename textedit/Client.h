#pragma once

#include <qobject.h>
#include <qtcpsocket.h>
#include <QtNetwork>
#include <QObject>

#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <csignal>
#include <stdio.h>

#include <string>

#include <fstream>
#include <iostream>
#include <sstream>

#include <vector>

// File to handle Message with the server
#include <AccountMessage.h>
#include <Message.h>
#include <LoginMessage.h>
#include <LogoutMessage.h>
#include <PresenceMessage.h>
#include <DocumentMessage.h>
#include <MessageFactory.h>
#include <TextEditMessage.h>
#include <SocketBuffer.h>


//File for DataStructure
#include <User.h>
#include <Symbol.h>
#include <Document.h>


#define READYREAD_TIMEOUT 10000

class Client : public QObject
{
	Q_OBJECT

private:

	QSslSocket* socket;
	QString username;
	QString nickname;
	QString password;
	SocketBuffer socketBuffer;
	QImage image;
	bool login;

	enum qint16{
		LoginMessage,
		RegisterMessage,
		OpenFileMessage,
		CreateFileMessage,
		DeleteMessage
	};

signals:

	void connectionEstablished();
	void impossibleToConnect();

	// Login, Logout & Register
	void loginSuccess(User user);
	void loginFailed(QString errorType);
	void registrationCompleted(User user);
	void registrationFailed(QString errorType);
	void logoutCompleted();
	void logoutFailed(QString errorType);

	// Presence Signals
	void cursorMoved(qint32 position, qint32 user);
	void userPresence(qint32 userId, QString username, QImage image);	
	void cancelUserPresence(qint32 userId);
	void accountModified(qint32 userId, QString username, QImage image);

	//Account signals
	void personalAccountModified(User user);
	void accountModificationFail(QString error);
	
	//Document Signals
	void removeFileFailed(QString errorType);
	void openFileCompleted(Document document);
	void openFileFailed(QString error);
	void documentDismissed(URI URI);
	
	// Symbol Signals
	void recivedSymbol(Symbol character);
	void removeSymbol(QVector<int> position);

public:

	Client(QObject* parent = 0);
	~Client();

	void messageHandler(MessageType typeOfMeassage, QDataStream& in);
	MessageCapsule readMessage(QDataStream& stream, qint16 typeOfMessage);

public slots:

	// signals handler
	void serverConnection();
	void readBuffer();
	void serverDisconnection();
	void errorHandler();
	void writeOnServer();
	void ready();
	void handleSslErrors(const QList<QSslError>& sslErrors);
	// User connection
	void Login();
	void Register();
	void Logout();
	// Data Exchange
	void sendCursor(qint32 userId, qint32 position);
	void receiveCursor(QDataStream& in);
	void sendChar(Symbol character);
	void removeChar(QVector<int> position);
	void receiveChar(QDataStream& in);
	void deleteChar(QDataStream& in);
	// Document handler
	void openDocument(URI URI);
	void createDocument(QString name);
	void deleteDocument(URI URI);
	// Server connection
	void Connect(QString ipAddress, quint16 port);
	void Disconnect();
	// Setter & Getter
	void setUsername(QString username);
	void setPassword(QString password);
	void setLogin(bool flag);
	void setNickname(QString nickname);
	void setImage(QImage image);
	bool getLogin();
	// Account handler
	void newUserPresence(QDataStream& in);
	void accountUpdate(QDataStream& in);
	void sendAccountUpdate(QString nickname, QImage image, QString password);
	void deleteUserPresence(QDataStream& in);
	void removeFromFile(qint32 myId);
};

