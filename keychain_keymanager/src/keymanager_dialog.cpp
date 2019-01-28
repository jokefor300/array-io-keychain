#include "keymanager_dialog.hpp"

keymanager_dialog::keymanager_dialog(QWidget * parent)
	: QDialog(parent) 
{
    //define common dialog properties
    QMetaObject::connectSlotsByName(this);
    setWindowTitle(QApplication::translate("keychain_gui_winClass", "keychain_gui_win", nullptr));
    resize(1100, 760);
    setWindowFlags(Qt::FramelessWindowHint);
    setStyleSheet("background-color:rgb(242,243,246);");
	//QFontDatabase

	QFontDatabase::addApplicationFont(":/fonts/Cartograph Mono CF/Cartograph Mono CF Regular.ttf");
}

//process export dialog signal call
void keymanager_dialog::ProcessExport(const QString &text)
{
    text_edit_form->setText("Process Export!");
}

void keymanager_dialog::init()
{
	//
	//define header properties
	header = new QLabel(this);
	header->setFixedSize(this->width(), 50);
	header->setStyleSheet("background-color:rgb(255,255,255);");
	//define logo picture
	QPixmap logo(":/keymanager/keychain_logo.png");
	logo = logo.scaled(100, 38, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	logoLabel = new QLabel(this);
	logoLabel->setStyleSheet("background:transparent;");
	logoLabel->setPixmap(logo);
	logoLabel->move(811, 4);
	//create logo title "KeyManager
	logoTitle = new QLabel("KeyManager", this);
	logoTitle->move(884, 20);
	QFont font("Cartograph Mono CF");
	font.setPixelSize(15);
	logoTitle->setFont(font);
	logoTitle->setStyleSheet("background:transparent;");
	//end dialog initialization

	keys_table = new keychain_table(this);
	keys_table->move(32, 90);
	keys_table->init();
	keys_table->load_records();

	//create menu toolbar 
	toolbar = new menu_toolbar(this);
	toolbar->setStyleSheet("font:\"Segoe UI\";background:transparent;");
	toolbar->move(27, 10);

	//create text-edit as temporary GUI element
	text_edit_form = new QTextEdit(this);
	text_edit_form->setFixedSize(100, 30);
	text_edit_form->setStyleSheet("font:10px \"Segoe UI\";");
	text_edit_form->move(210, 10);

	QObject::connect(toolbar, SIGNAL(ExportSelected(QString)), this, SLOT(ProcessExport(QString)));
	QObject::connect(toolbar, SIGNAL(ImportSelected(QString)), this, SLOT(ProcessImport(QString)));
	QObject::connect(toolbar, SIGNAL(AboutSelected(QString)), this, SLOT(ProcessAbout(QString)));
	QObject::connect(toolbar, SIGNAL(StatusSelected(QString)), this, SLOT(ProcessStatus(QString)));
	QObject::connect(toolbar, SIGNAL(ExitSelected(QString)), this, SLOT(ProcessExit(QString)));
}

//process import dialog signal call
void keymanager_dialog::ProcessImport(const QString &text)
{
    text_edit_form->setText("Process Import!");
}

//process about dialog signal call
void keymanager_dialog::ProcessAbout(const QString &text)
{
    text_edit_form->setText("Process About!");
}

//process status dialog signal call
void keymanager_dialog::ProcessStatus(const QString &text)
{
    text_edit_form->setText("Process Status!");
}

//process exit dialog signal call
void keymanager_dialog::ProcessExit(const QString &text)
{
    text_edit_form->setText("Process Exit!");
    QApplication::quit();
}

//process minimize signal call
void keymanager_dialog::ProcessMinimize(const QString &text)
{
    text_edit_form->setText("Process Minimize!");
    showMinimized();
}

//process maximize signal call
void keymanager_dialog::ProcessMaximize(const QString &text)
{
    text_edit_form->setText("Process Maximize!");
    showMaximized();
}

//process hint events
void keymanager_dialog::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent* event = static_cast<QWindowStateChangeEvent*>(e);

        if (this->windowState() & Qt::WindowMaximized)
        {
            if (event->oldState() & Qt::WindowNoState)
            {
                showMaximized();
            }
            else if (event->oldState() == Qt::WindowMaximized)
            {
                showNormal();
            }
        }
    }
}
