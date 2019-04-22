#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtServer.h"


class QtServer : public QMainWindow
{
	Q_OBJECT

public:
	QtServer(QWidget *parent = Q_NULLPTR);

public slots:
	void start();

private:
	Ui::QtServerClass ui;
};
