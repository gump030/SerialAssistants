#include "MainWindow.h"
#include <qdebug.h>
#include <qscrollbar.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qsettings.h>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), totalSend(0), totalReceive(0)
{	
	ui.setupUi(this);
	setWindowIcon(QIcon("./image/globe.png"));
	SetCurrent();									//读取配置文件信息
	my_serial = new QSerialPort(this);				//new串口信息对象
	currentTime = QTime::currentTime();				//当前时间
	sendTimer = new QTimer(this);					//创建定时器
	CreateActions();
	on_action_triggered();							//启动搜索端口号
	//SetSerial();
	SetStatusBar();									//设置底部状态栏
	connect(my_serial, SIGNAL(readyRead()), this, SLOT(ShowData()));
	connect(sendTimer, SIGNAL(timeout()), this, SLOT(on_SendButton_clicked()));
}

MainWindow::~MainWindow()
{
	QString iniFilePath = "./ini/self.ini";
	QSettings settings(iniFilePath, QSettings::IniFormat);

	settings.setValue("SerialSetting/Baudrate", ui.baudrateComboBox->currentIndex());
	settings.setValue("SerialSetting/Databit", ui.databitComboBox->currentIndex());
	settings.setValue("SerialSetting/Checkbit", ui.checkbitComboBox->currentIndex());
	settings.setValue("SerialSetting/Stopbit", ui.stopbitComboBox->currentIndex());
	settings.setValue("SerialSetting/Flowcontrol", ui.flowcontrolComboBox->currentIndex());

	settings.setValue("Receive/Ascii", ui.AsciiButton_1->isChecked());
	settings.setValue("Receive/Hex", ui.HexButton_1->isChecked());
	settings.setValue("Receive/Auto", ui.AutocheckBox->isChecked());
	settings.setValue("Receive/Showsend", ui.ShowsendcheckBox->isChecked());
	settings.setValue("Receive/Showtime", ui.ShowtimecheckBox->isChecked());

	settings.setValue("Send/Ascii", ui.AsciiButton_2->isChecked());
	settings.setValue("Send/Hex", ui.HexButton_2->isChecked());
	settings.setValue("Send/Send", ui.sendCheckBox->isChecked());
	settings.setValue("Send/Timespin", ui.timespinBox->text());
}

//设置状态栏控件
void MainWindow::SetStatusBar()
{
	receiveLabel = new QLabel("Rx：0 Bytes");
	receiveLabel->setFixedWidth(200);
	ui.statusBar->addPermanentWidget(receiveLabel); //现实永久信息	
	sendLabel = new QLabel("Tx：0 Bytes");
	sendLabel->setFixedWidth(200);
	ui.statusBar->addPermanentWidget(sendLabel); //现实永久信息
}

void MainWindow::SetCurrent()
{
	QString iniFilePath = "./ini/self.ini";
	QSettings settings(iniFilePath, QSettings::IniFormat);

	ui.baudrateComboBox->setCurrentIndex(settings.value("SerialSetting/Baudrate").toInt());
	ui.databitComboBox->setCurrentIndex(settings.value("SerialSetting/Databit").toInt());
	ui.checkbitComboBox->setCurrentIndex(settings.value("SerialSetting/Checkbit").toInt());
	ui.stopbitComboBox->setCurrentIndex(settings.value("SerialSetting/Stopbit").toInt());
	ui.flowcontrolComboBox->setCurrentIndex(settings.value("SerialSetting/Flowcontrol").toInt());

	ui.AsciiButton_1->setChecked(settings.value("Receive/Ascii").toBool());
	ui.HexButton_1->setChecked(settings.value("Receive/Hex").toBool());
	ui.AutocheckBox->setChecked(settings.value("Receive/Auto").toBool());
	ui.ShowsendcheckBox->setChecked(settings.value("Receive/Showsend").toBool());
	ui.ShowtimecheckBox->setChecked(settings.value("Receive/Showtime").toBool());

	ui.AsciiButton_2->setChecked(settings.value("Send/Ascii").toBool());
	ui.HexButton_2->setChecked(settings.value("Send/Hex").toBool());
	ui.sendCheckBox->setChecked(settings.value("Send/Send").toBool());
	ui.timespinBox->setValue(settings.value("Send/Timespin").toInt());
}

//开始暂停关闭按钮设成同一个组
void MainWindow::CreateActions()
{
	switchgroup = new QActionGroup(this);
	switchgroup->addAction(ui.startAction);
	switchgroup->addAction(ui.pauseAction);
	switchgroup->addAction(ui.stopAction);
	connect(switchgroup, SIGNAL(triggered(QAction*)), this, SLOT(ShowAlignment(QAction*)));
}

//设置串口参数选中状态
void MainWindow::SetSerialEnabled(bool state)
{
	ui.SerialComboBox->setEnabled(state);
	ui.baudrateComboBox->setEnabled(state);
	ui.checkbitComboBox->setEnabled(state);
	ui.databitComboBox->setEnabled(state);
	ui.flowcontrolComboBox->setEnabled(state);
	ui.stopbitComboBox->setEnabled(state);
	ui.action->setEnabled(state); 
}

//显示版本相关信息
void MainWindow::on_aboutAction_triggered()
{
	aboutDialog.setModal(true);
	aboutDialog.show();
}

//保存log相关信息dialog
void MainWindow::on_saveAction_triggered()
{
	log.show();
}

//退出按钮按下时关闭窗口
void MainWindow::on_quitAction_triggered()
{
	this->close();
}

//搜索串口槽函数
void MainWindow::on_action_triggered()
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
	ui.SerialComboBox->addItem(tr("TCP Server"));
	ui.SerialComboBox->addItem(tr("TCP Client"));
	ui.SerialComboBox->addItem(tr("UDP"));
}

void MainWindow::on_SerialComboBox_currentIndexChanged()
{
	QString name = ui.SerialComboBox->currentText();
	if (name == "TCP Server")
	{	
		//ui.verticalLayout->removeWidget(ui.baudrateLabel);
		//delete ui.baudrateLabel;
		//ui.verticalLayout_2->removeWidget(ui.baudrateComboBox);
	}
}

//设置串口信息
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
	my_serial->setRequestToSend(true);
	my_serial->setDataTerminalReady(true);
	my_serial->clearError();
	my_serial->clear();
}

//开始暂停关闭按钮槽函数
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
				SetSerialEnabled(false);
				ui.statusBar->setStyleSheet("QStatusBar{ color:green }");
				ui.statusBar->showMessage(my_serial->portName() + " 打开成功");
			}
			else
			{
				ui.statusBar->setStyleSheet("QStatusBar{ color:red }");
				ui.statusBar->showMessage(my_serial->portName() + " 打开失败", 3000);
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
			SetSerialEnabled(true);
			ui.statusBar->setStyleSheet("QStatusBar{ color:red }");
			ui.statusBar->showMessage(my_serial->portName() + " 已关闭", 3000);
		}
	}
}

//发送信息槽函数
void MainWindow::on_SendButton_clicked()
{
	if (my_serial->isOpen())
	{
		if (!ui.SendTextEdit->toPlainText().isEmpty())
		{
			QString sendstr = ui.SendTextEdit->toPlainText();
			int j = 0;
			for (int i = 0; i < ui.comboBox->count(); i++)
			{
				if (ui.comboBox->itemText(i) == sendstr)
				{
					break;
				}
				else
					++j;
			}
			if (j == ui.comboBox->count())
			{
				ui.comboBox->insertItem(0, sendstr);
				ui.comboBox->setCurrentIndex(0);
			}
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

//显示接收到的信息槽函数
void MainWindow::ShowData()
{
	//qDebug() << "11111";
	QString strTime = "[" + currentTime.currentTime().toString("hh:mm:ss:zzz") + "] ";
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
		
		ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText() + strTime + show);
	}
	else
		ui.ReceiveTextEdit->setText(ui.ReceiveTextEdit->toPlainText()  + show);
	ui.ReceiveTextEdit->verticalScrollBar()->setValue(ui.ReceiveTextEdit->verticalScrollBar()->maximum());
	totalReceive += showdata.length();
	receiveLabel->setText("Rx：" + QString::number(totalReceive, 10) + " Bytes");
	if (log.IsLog())
	{
		qDebug() << "isopen";
		QFile file(log.getFileName());//文件命名  
		if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Append))     //检测文件是否打开  
		{
			QMessageBox::information(this, "Error Message", "Please Select a Text File!");
			return;
		}
		QTextStream out(&file);
		out << strTime + show;
		file.close();
	}
}

//定时重发ON/OFF
void MainWindow::on_sendCheckBox_clicked(bool state)
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

//定时重发间隔时间改变时，改变定时器的时间间隔
void MainWindow::on_timespinBox_valueChanged(int value)
{
	qDebug() << "2212";
	if (ui.sendCheckBox->isChecked())
	{
		sendTimer->start(value);
	}
		
}

//选择历史发送记录，并填充到发送TextEdit中
void MainWindow::on_comboBox_currentIndexChanged()
{
	ui.SendTextEdit->setText(ui.comboBox->currentText());
}

//清空按钮按下时，清除receive textedit中的内容及计数
void MainWindow::on_clearAction_triggered()
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