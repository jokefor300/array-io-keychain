#include "menu_toolbar.hpp"

menu_toolbar::menu_toolbar(QWidget *parent)
	: QToolBar(parent)
{
    //create File menu actions for slots
    QAction *actionExport = new QAction("&Export", this);
    QAction *actionImport = new QAction("&Import", this);
    QAction *actionExit = new QAction("&Exit", this);

    QMenu *menuFile = new QMenu("&File");
    menuFile->setStyleSheet("QMenu {background-color: #fafafc;}");
    menuFile->setStyleSheet(menuFile->styleSheet() + "QMenu::item:selected {background-color: #9faec5;}");
    menuFile->addAction(actionExport);
    menuFile->addAction(actionImport);
    menuFile->addAction(actionExit);

    //create About menu actions for slots
    QAction *actionAbout = new QAction("&About", this);
    QAction *actionStatus = new QAction("&Status", this);

    //create menu and add actions to it
    QMenu *menuAbout = new QMenu("&About");
    menuAbout->setStyleSheet("QMenu {background-color: #fafafc;}");
    menuAbout->setStyleSheet(menuAbout->styleSheet() + "QMenu::item:selected {background-color: #9faec5;}");
    menuAbout->addAction(actionAbout);
    menuAbout->addAction(actionStatus);

    //create file button
    QPushButton *FileButton = new QPushButton;
    FileButton->setStyleSheet("QWidget > QPushButton {background-color: #fafafc; \
                               min-width: 4em; \
                               padding: 4px; \
                               border: 4px solid #fafafc; \
                               border-radius: 5px;}");
    FileButton->setStyleSheet(FileButton->styleSheet() + "QPushButton::menu-indicator{image: none;}");

    //create file button text label
    QLabel *FileButtonLabel = new QLabel(FileButton);
    FileButtonLabel->setText("File");
    FileButtonLabel->setAlignment(Qt::AlignCenter);
    FileButtonLabel->setStyleSheet("QWidget > QLabel {background-color: #fafafc; \
                                    text-align: center; \
                                    min-width: 4em; \
                                    border: 4px solid #fafafc; \
                                    border-radius: 5px;}");
    FileButtonLabel->setFont(QFont("Segoe UI Semibold", 10, QFont::Normal, false));
    
    //create about button text label
    QPushButton *AboutButton = new QPushButton("About");
    AboutButton->setStyleSheet("QWidget > QPushButton {background-color: #fafafc; \
                                min-width: 4em; \
                                padding: 4px; \
                                margin-left: 4px; \
                                border: 4px solid #fafafc; \
                                border-radius: 5px;}");
    AboutButton->setStyleSheet(AboutButton->styleSheet() + "QPushButton::menu-indicator{image: none;}");

    //create about button text label
    QLabel *AboutButtonLabel = new QLabel(AboutButton);
    AboutButtonLabel->setText("About");
    AboutButtonLabel->setAlignment(Qt::AlignCenter);
    AboutButtonLabel->setStyleSheet("QWidget > QLabel {background-color: #fafafc; \
                                    text-align: center; \
                                    min-width: 4em; \
                                    margin-left: 4px; \
                                    border: 4px solid #fafafc; \
                                    border-radius: 5px;}");
    AboutButtonLabel->setFont(QFont("Segoe UI Semibold", 10, QFont::Normal, false));
    
    //change buttons to flat style
    FileButton->setFlat(true);
    AboutButton->setFlat(true);

    //add menu to buttons
    FileButton->setMenu(menuFile);
    AboutButton->setMenu(menuAbout);

    //add buttons to toolbar
    QToolBar *toolBarFile = new QToolBar();
    toolBarFile->addWidget(FileButton);
    toolBarFile->addWidget(AboutButton);
    
    //add toolbar to main layout
    this->addWidget(toolBarFile);

    //temporary use textedit for debug output
    text_edit_form = new QTextEdit();
    text_edit_form->setFixedSize(100, 30);
    text_edit_form->setStyleSheet("font:10px \"Segoe UI\";");
    this->addWidget(text_edit_form);

    //connect actions to slots via qt signals
    connect(actionExit, SIGNAL(triggered()), this, SLOT(Exit()));
    connect(actionExport, SIGNAL(triggered()), this, SLOT(Export()));
    connect(actionImport, SIGNAL(triggered()), this, SLOT(Import()));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(About()));
    connect(actionStatus, SIGNAL(triggered()), this, SLOT(Status()));
}

//call export dialog
void menu_toolbar::Export()
{
    text_edit_form->setText("Export was clicked!");
}

//call import dialog
void menu_toolbar::Import()
{
    text_edit_form->setText("Import was clicked!");
}

//handle exit event
void menu_toolbar::Exit()
{
    text_edit_form->setText("Exit was clicked!");
    QApplication::quit();
}

//print version info and other information 
void menu_toolbar::About()
{
    text_edit_form->setText("About was clicked!");
}

//print current keychain status ok / failed
void menu_toolbar::Status()
{
    text_edit_form->setText("Status was clicked!");
}