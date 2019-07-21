#ifndef LANDINGPAGE_H
#define LANDINGPAGE_H

#include <QMainWindow>
#include <QDialog>
#include <QLabel>

#include "Client.h"


namespace Ui {
	class LandingPage;
}

class LandingPage : public QMainWindow
{
	Q_OBJECT

public:
	LandingPage(Client* client, QWidget * parent = nullptr);
	~LandingPage();

	void openEditor(int mode, QString path = nullptr);
	enum openMode {uri, newfile};

private slots:
	void pushButtonLoginClicked();
	void pushButtonNewClicked();
	void pushButtonRegisterClicked();
	void pushButtonBrowseClicked();
	void pushButtonConfirmRegistrationClicked();
	void pushButtonBackClicked();
	void pushButtonOpenClicked();
	void pushButtonOpenUriClicked();
	void enablePushButtonOpen();
	void tryToLoginOrRegister();


	void showUserIcon(QString path);

private:
	Ui::LandingPage* ui;
	Client* client;

	QLabel* loading;

private:
	void setupFileList();
	void centerAndResize();
	void startLoadingAnimation();
	void stopLoadingAnimation();
	
};

#endif // LANDINGPAGE_H