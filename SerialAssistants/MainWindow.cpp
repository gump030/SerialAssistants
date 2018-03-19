#include "MainWindow.h"
#include <qdebug.h>
#include <qscrollbar.h>
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), totalSend(0), totalReceive(0)
{	
	ui.setupUi(this);
	setWindowIcon(QIcon("./image/globe.png"));
	my_serial = new QSerialPort(this);
	currentTime = QTime::currentTime();
	sendTimer = new QTimer(this);
	CreateActions();
	SearchPort();//启动搜索端口号
	SetSerial();
	SetStatusBar();
	CreateSignal();

}

void MainWindow::SetStatusBar()
{
	pa1.setColor(QPalette::WindowText, Qt::red);
	pa2.setColor(QPalette::WindowText, Qt::darkGreen);
	messageLabel = new QLabel;
	messageLabel->setPalette(pa1);
	messageLabel->setText(my_serial->portName() + " 已关闭");
	ui.statusBar->addWidget(messageLabel,3); //现实永久信息
	receiveLabel = new QLabel("Rx：0 Bytes");
	ui.statusBar->addWidget(receiveLabel, 3); //现实永久信息	
	sendLabel = new QLabel("Tx：0 Bytes");
	ui.statusBar->addWidget(sendLabel,3); //现实永久信息
}

void MainWindow::CreateSignal()
{
	connect(ui.aboutAction, SIGNAL(triggered()), this, SLOT(ShowAboutDialog()));//显示相关信息Dialog 
	connect(ui.quitAction, SIGNAL(triggered()), this, SLOT(CloseWindow()));		//关闭窗口
	connect(ui.action, SIGNAL(triggered()), this, SLOT(SearchPort()));			//按下刷新按钮刷新端口
	connect(ui.SendButton, SIGNAL(clicked()), this, SLOT(SendData()));
	connect(ui.clearAction, SIGNAL(triggered()), this, SLOT(Clear()));
	connect(my_serial, SIGNAL(readyRead()), this, SLOT(ShowData()));
	connect(sendTimer, SIGNAL(timeout()), this, SLOT(SendData()));
	connect(ui.sendCheckBox, SIGNAL(clicked(bool)), this, SLOT(AutoSend(bool))); \
	connect(ui.timespinBox, SIGNAL(valueChanged(int)), this, SLOT(ChangeSendTime(int)));
}

void MainWindow::CreateActions()
{
	switchgroup = new QActionGroup(this);
	switchgroup->addAction(ui.startAction);
	switchgroup->addAction(ui.pauseAction);
	switchgroup->addAction(ui.stopAction);
	connect(switchgroup, SIGNAL(triggered(QAction*)), this, SLOT(ShowAlignment(QAction*)));
}

void MainWindow::ShowAboutDialog()
{
	aboutDialog.show();
}

void MainWindow::CloseWindow()
{
	this->close();
}

void MainWindow::SearchPort()
{
	ui.SerialComboBox->clear();
	foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
	{
		qDebug() << "Name        : " << info.portName();
		qDebug() << "Description : " << info.description();
		qDebug() << "Manufacturer: " << info.manufacturer();
		QSerialPort serial;
		serial.setPort(info);
		if(serial.open(QIODevice::ReadWrite))
		{
			ui.SerialComboBox->addItem(info.description() + "(" + info.portName() + ")");
		}
	}
}

void MainWindow::SetSerial()
{
	/*设置端口号*/
	QString comname = ui.SerialComboBox->currentText();
	foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
	{
		if (info.description() + "(" + info.portName() + ")" == comname)
		{
			my_serial->setPort(info);
		}
	}
	/*设置波特率*/
	int baudrate_index = ui.baudrateComboBox->currentIndex();
	switch (baudrate_index)
	{
	case 0 :
		my_serial->setBaudRate(QSerialPort::Baud1200, QSerialPort::AllDirections);
		break;
	case 1:
		my_serial->setBaudRate(QSerialPort::Baud2400, QSerialPort::AllDirections);
		break;
	case 2:
		my_serial->setBaudRate(QSerialPort::Baud4800, QSerialPort::AllDirections);
		break;
	case 3:
		my_serial->setBaudRate(QSerialPort::Baud9600, QSerialPort::AllDirections);
		break;
	case 4:
		my_serial->setBaudRate(QSerialPort::Baud19200, QSerialPort::AllDirections);
		break;
	case 5:
		my_serial->setBaudRate(QSerialPort::Baud38400, QSerialPort::AllDirections);
		break;
	case 6:
		my_serial->setBaudRate(QSerialPort::Baud57600, QSerialPort::AllDirections);
		break;
	case 7:
		my_serial->setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
		break;
	default:
		my_serial->setBaudRate(QSerialPort::UnknownBaud, QSerialPort::AllDirections);
		break;
	}
	/*设置数据位*/
	int databits_index = ui.databitComboBox->currentIndex();
	switch (databits_index) {
	case 0:
		my_serial->setDataBits(QSerialPort::Data8);
		break;
	case 1:
		my_serial->setDataBits(QSerialPort::Data7);
		break;
	case 2:
		my_serial->setDataBits(QSerialPort::Data6);
		break;
	case 3:
		my_serial->setDataBits(QSerialPort::Data5);
		break;
	default:
		my_serial->setDataBits(QSerialPort::UnknownDataBits);
		break;
	}
	/*设置校验位*/
	int parity_index = ui.checkbitComboBox->currentIndex();
	switch (parity_index) {
	case 0:
		my_serial->setParity(QSerialPort::NoParity);
		break;
	case 1:
		my_serial->setParity(QSerialPort::EvenParity);
		break;
	case 2:
		my_serial->setParity(QSerialPort::OddParity);
		break;
	case 3:
		my_serial->setParity(QSerialPort::MarkParity);
		break;
	case 4:
		my_serial->setParity(QSerialPort::SpaceParity);
		break;
	default:
		my_serial->setParity(QSerialPort::UnknownParity);
		break;
	}
	/*设置停止位*/
	int stopbit_index = ui.stopbitComboBox->currentIndex();
	switch (stopbit_index) {
	case 0:
		my_serial->setStopBits(QSerialPort::OneStop);
		break;
	case 1:
		my_serial->setStopBits(QSerialPort::OneAndHalfStop);
		break;
	case 2:
		my_serial->setStopBits(QSerialPort::TwoStop);
		break;
	default:
		my_serial->setStopBits(QSerialPort::UnknownStopBits);
		break;
	}
	int flowcontrol_index = ui.flowcontrolComboBox->currentIndex();
	switch (flowcontrol_index)
	{
	case 0:
		my_serial->setFlowControl(QSerialPort::NoFlowControl);
		break;
	case 1:
		my_serial->setFlowControl(QSerialPort::HardwareControl);
		break;
	case 2:
		my_serial->setFlowControl(QSerialPort::SoftwareControl);
		break;
	}
	//my_serial->setDataTerminalReady(true);
	//my_serial->setRequestToSend(true);
	my_serial->clearError();
	my_serial->clear();
}

void MainWindow::ShowAlignment(QAction* act)
{
	if (act == ui.startAction)
	{
		SetSerial();
		if (!my_serial->isOpen())
		{
			ui.stopAction->setChecked(false);
			ui.pauseAction->setChecked(false);
			bool com = my_serial->open(QIODevice::ReadWrite);
			if (com)
			{
				messageLabel->setPalette(pa2);
				messageLabel->setText(my_serial->portName() + " 打开成功");
			}
			else
			{
				messageLabel->setPalette(pa1);
				messageLabel->setText(my_serial->portName() + " 打开失败");
			}
		}
	}
	else if (act == ui.pauseAction)
	{
		ui.startAction->setChecked(false);
		ui.stopAction->setChecked(false);
		if (my_serial->isOpen())
		{
			
		}
	}
	else if (act == ui.stopAction)
	{
		ui.startAction->setChecked(false);
		ui.pauseAction->setChecked(false);
		if (my_serial->isOpen())
		{
			my_serial->close();
			messageLabel->setPalette(pa1);
			messageLabel->setText(my_serial->portName() + " 打开失败");
		}
	}
}

void MainWindow::SendData()
{
	if (my_serial->isOpen())
	{
		if (!ui.SendTextEdit->toPlainText().isEmpty())
		{
			QString sendstr = ui.SendTextEdit->toPlainText();
			QString show = "";
			QByteArray sdata;
			if (ui.HexButton_2->isChecked())
			{
				int len = sendstr.length();
				if (len % 2 == 1)   //如果发送的数据个数为奇数的，则在前面最后落单的字符前添加一个字符0
				{
					sendstr = sendstr.insert(len - 1, '0'); //insert(int position, const QString & str)
				}
				StringToHex(sendstr, sdata);//将str字符串转换为16进制的形式
				if (ui.ShowsendcheckBox->isChecked())
				{
					for (int i = 0; i < sdata.length(); i++)
					{
						qint8 outChar = sdata[i];
						QString str = QString("%1").arg(outChar & 0xFF, 2, 16, QLatin1Char('0')) + " ";
						show += str.toUpper();
					}
					if (ui.ShowtimecheckBox->isChecked())
					{
						QString strTime = "[" + currentTime.toString("hh:mm:ss:zzz") + "] ";
						if (ui.AutocheckBox->isChecked())
							ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText() + strTime + "发送：" + show + "\n");
						else
							ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText() + strTime + "发送：" + show);
					}
					else
					{
						if (ui.AutocheckBox->isChecked())
							ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText() + "发送：" + show + "\n");
						else
							ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText() + "发送：" + show);
					}
					ui.ReceiveTextEdit->verticalScrollBar()->setValue(ui.ReceiveTextEdit->verticalScrollBar()->maximum());
				}
			}
			else
			{
				//sdata.append(sendstr);
				sdata.append(sendstr).toHex();
				if (ui.ShowsendcheckBox->isChecked())
				{
					if (ui.ShowtimecheckBox->isChecked())
					{
						QString strTime = "[" + currentTime.toString("hh:mm:ss:zzz") + "] ";
						if (ui.AutocheckBox->isChecked())
							ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText() + strTime + "发送：" + sendstr + "\n");
						else
							ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText() + strTime + "发送：" + sendstr);
					}
					else
					{
						if (ui.AutocheckBox->isChecked())
							ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText() + "发送：" + sendstr + "\n");
						else
							ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText() + "发送：" + sendstr);
					}
					ui.ReceiveTextEdit->verticalScrollBar()->setValue(ui.ReceiveTextEdit->verticalScrollBar()->maximum());
				}
			}
			my_serial->write(sdata, sdata.length());
			totalSend += sdata.length();
			sendLabel->setText("Tx：" + QString::number(totalSend, 10) + " Bytes");
		}
		else
		{
			qDebug() << "请输入要发送的值";
		}
	}
}

void MainWindow::ShowData()
{
	//qDebug() << "11111";
	QByteArray showdata = my_serial->readAll();
	qDebug() << "show data is " << showdata;
	QString show = "接收：";
	if (ui.HexButton_1->isChecked())
	{
		for (int i = 0; i < showdata.length(); i++)
		{
			qint8 outChar = showdata[i];
			QString str = QString("%1").arg(outChar & 0xFF, 2, 16, QLatin1Char('0')) + " ";
			show += str.toUpper();
		}
	}
	else
	{
		show += QString(showdata);
	}	
	if(ui.AutocheckBox->isChecked())
		show += "\n";
	if (ui.ShowtimecheckBox->isChecked())
	{
		QString strTime = "[" + currentTime.toString("hh:mm:ss:zzz") + "] ";
		ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText() + strTime + show);
	}
	else
		ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText()  + show);
	ui.ReceiveTextEdit->verticalScrollBar()->setValue(ui.ReceiveTextEdit->verticalScrollBar()->maximum());
	totalReceive += showdata.length();
	receiveLabel->setText("Rx：" + QString::number(totalReceive, 10) + " Bytes");
}

void MainWindow::AutoSend(bool state)
{
	qDebug() << "111";
	if(state)
	{
		sendTimer->start(ui.timespinBox->value());
	}
	else
	{
		sendTimer->stop();
	}
}

void MainWindow::ChangeSendTime(int value)
{
	qDebug() << "2212";
	if (ui.sendCheckBox->isChecked())
	{
		sendTimer->start(value);
	}
		
}

void MainWindow::Clear()
{
	ui.ReceiveTextEdit->clear();
	totalSend = 0;
	totalReceive = 0;
	sendLabel->setText("Tx：0 Bytes");
	receiveLabel->setText("Rx：0 Bytes");
}

void MainWindow::StringToHex(QString str, QByteArray &senddata) //字符串转换为十六进制数据0-F
{
	int hexdata, lowhexdata;
	int hexdatalen = 0;
	int len = str.length();
	senddata.resize(len / 2);
	char lstr, hstr;

	for (int i = 0; i<len; )
	{
		//char lstr,
		hstr = str[i].toLatin1();
		if (hstr == ' ')
		{
			i++;
			continue;
		}
		i++;
		if (i >= len)
			break;
		lstr = str[i].toLatin1();
		hexdata = ConvertHexChar(hstr);
		lowhexdata = ConvertHexChar(lstr);
		if ((hexdata == 16) || (lowhexdata == 16))
			break;
		else
			hexdata = hexdata * 16 + lowhexdata;
		i++;
		senddata[hexdatalen] = (char)hexdata;
		hexdatalen++;
	}
	senddata.resize(hexdatalen);
}

char MainWindow::ConvertHexChar(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - 0x30;
	else if ((ch >= 'A') && (ch <= 'F'))
		return ch - 'A' + 10;
	else if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	else return ch - ch;//不在0-f范围内的会发送成0
}