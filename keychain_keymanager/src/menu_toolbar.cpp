#include "menu_toolbar.hpp"

menu_toolbar::menu_toolbar(QWidget *parent)
	: QToolBar(parent)
{
    //create file button
    QPushButton *FileButton = new QPushButton(this);
    FileButton->setObjectName("filebtn");
    FileButton->setStyleSheet("QPushButton#filebtn {margin-left:3px; margin-right: 2px;}");
    FileButton->setStyleSheet(FileButton->styleSheet() + "QPushButton#filebtn:hover:pressed{background-color:#e7e9ef;\
                                                          border-style:inset; \
                                                          border-radius:2px 2px 2px 2px; \
                                                          border: 2px solid #e7e9ef;}");
    FileButton->setStyleSheet(FileButton->styleSheet() + "QPushButton::menu-indicator{image: none;}");
    FileButton->setFixedSize(45, 24);

    //create file button text label
    custom_qlabel *FileButtonLabel = new custom_qlabel(FileButton);
    FileButtonLabel->setText("File");
    FileButtonLabel->setAlignment(Qt::AlignCenter);
    FileButtonLabel->setFont(QFont("Segoe UI Semibold", 12, QFont::Bold, false));
    FileButtonLabel->setFixedSize(40, 24);

    custom_menu *menuFile = new custom_menu(FileButton, this);
    menuFile->setObjectName("menufile");
    menuFile->setStyleSheet("QMenu#menufile {background-color:#ffffff; \
                                             padding: 5px; \
                                             font-family:\"Segoe UI Normal\"; \
                                             border-style:inset; \
                                             border-radius:2px; \
                                             border: 1px solid #e7e9ef; \
                                             font-size:14px;}");
    menuFile->setStyleSheet(menuFile->styleSheet() + "QMenu#menufile::item:selected {background-color:#e7e9ef; \
                                                      border-style:inset; \
                                                      border-radius:2px; \
                                                      border: 0px solid #e7e9ef; }");

    //create File menu actions for slots
    QAction *actionCreate = new QAction("&Create key", this);
    QAction *actionExport = new QAction("&Export file", this);
    QAction *actionImport = new QAction("&Import file", this);
    QAction *actionExit = new QAction("&Exit", this);
    menuFile->addAction(actionCreate);
    menuFile->addAction(actionExport);
    menuFile->addAction(actionImport);
    menuFile->addAction(actionExit);
    
    //create about button text label
    QPushButton *AboutButton = new QPushButton(this);
    AboutButton->setObjectName("aboutbtn");
    AboutButton->setStyleSheet("QPushButton#aboutlabel{margin-right: 2px;}");
    AboutButton->setStyleSheet(AboutButton->styleSheet() + "QPushButton#aboutbtn:hover:pressed{background-color:#e7e9ef; \
                                                            border-style:inset; \
                                                            border-radius:2px 2px 2px 2px; \
                                                            border: 2px solid #e7e9ef;}");
    AboutButton->setStyleSheet(AboutButton->styleSheet() + "QPushButton::menu-indicator{image: none;}");
    AboutButton->setFixedSize(55, 24);

    //create about button text label
    custom_qlabel *Aboutbuttonlabel = new custom_qlabel(AboutButton);
    Aboutbuttonlabel->setText("About");
    Aboutbuttonlabel->setAlignment(Qt::AlignCenter);
    Aboutbuttonlabel->setFont(QFont("Segoe UI Semibold", 12, QFont::Bold, false));
    Aboutbuttonlabel->setFixedSize(52, 24);

    //create menu and add actions to it
    custom_menu *menuAbout = new custom_menu(AboutButton, this);
    menuAbout->setObjectName("menuabout");
    menuAbout->setStyleSheet("QMenu#menuabout {background-color:#ffffff; \
                                               font-family:\"Segoe UI Normal\"; \
                                               padding: 5px; \
                                               border-style:inset; \
                                               border-radius:2px; \
                                               border: 1px solid #e7e9ef; \
                                               font-size:14px;}");
    menuAbout->setStyleSheet(menuAbout->styleSheet() + "QMenu#menuabout::item:selected {background-color:#e7e9ef; \
                                                        border-style:inset; \
                                                        border-radius:2px; \
                                                        border: 0px solid #e7e9ef;}");

    //create About menu actions for slots
    QAction *actionAbout = new QAction("&About", this);
    QAction *actionStatus = new QAction("&Status", this);
    menuAbout->addAction(actionAbout);
    menuAbout->addAction(actionStatus);

    //create key button
    QPushButton *KeyButton = new QPushButton(this);
    KeyButton->setObjectName("keybtn");
    KeyButton->setFixedSize(40, 18);

    //create key button text label
    custom_qlabel *Keybuttonlabel = new custom_qlabel(KeyButton);
    Keybuttonlabel->setAlignment(Qt::AlignCenter);

    //set flexy transformed pixmap for key button
    QPixmap keyBtnLogo(":/keymanager/tbr_key_icon.png");
    keyBtnLogo = keyBtnLogo.scaled(22, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    KeyButton->setIcon(keyBtnLogo);
    KeyButton->setIconSize(QSize(22, 18));
    
    //change buttons to flat style
    FileButton->setFlat(true);
    AboutButton->setFlat(true);
    KeyButton->setFlat(true);

    //add menu to buttons
    FileButton->setMenu(menuFile);
    AboutButton->setMenu(menuAbout);

    QHBoxLayout* elementsLayout = new QHBoxLayout(this);
    elementsLayout->setSizeConstraint(QLayout::SetFixedSize);
    elementsLayout->addWidget(FileButton, 1);
    elementsLayout->addWidget(AboutButton, 1);
    elementsLayout->addWidget(KeyButton, 1);

    QGridLayout* frameLayout = new QGridLayout(this);
    frameLayout->setSizeConstraint(QLayout::SetFixedSize);

    QWidget *extension = new QWidget(this);
    extension->setLayout(frameLayout);
    frameLayout->addLayout(elementsLayout, 0, 0);
    frameLayout->setSpacing(0);
    extension->setLayout(frameLayout);

    //add toolbar to main layout
    this->addWidget(extension);

    //connect actions to slots via qt signals
    QObject::connect(actionCreate, SIGNAL(triggered()), this, SLOT(Create()));
    QObject::connect(actionExport, SIGNAL(triggered()), this, SLOT(Export()));
    QObject::connect(actionImport, SIGNAL(triggered()), this, SLOT(Import()));
    QObject::connect(actionAbout, SIGNAL(triggered()), this, SLOT(About()));
    QObject::connect(actionStatus, SIGNAL(triggered()), this, SLOT(Status()));
    QObject::connect(actionExit, SIGNAL(triggered()), this, SLOT(Exit()));
}

//call create dialog
void menu_toolbar::Create()
{
    emit CreateSelected("Create was clicked!");
}

//call export dialog
void menu_toolbar::Export()
{
    emit ExportSelected("Export was clicked!");
}

//call import dialog
void menu_toolbar::Import()
{
    emit ImportSelected("Import was clicked!");
}

//call about dialog
void menu_toolbar::About()
{
    emit AboutSelected("About was clicked!");
}

//call status dialog
void menu_toolbar::Status()
{
    emit StatusSelected("Status was clicked!");
}

//call exit dialog
void menu_toolbar::Exit()
{
    emit ExitSelected("Exit was clicked!");
}