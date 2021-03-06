#include "LockIcon.h"



LockIcon::LockIcon(const KeychainWarningMessage wMessage, QWidget * parent)
	: QFrame (parent)
{
	this->setFixedHeight(25);
	this->setFixedWidth(25);
	if (wMessage.isWarn()==false)
		this->setStyleSheet("background:transparent;background-image:url(:/keychain_gui_win/lock.png);border-style:outset;border-width:0px;height:22px;width:22px");
	else
		this->setStyleSheet("background:transparent;background-image:url(:/keychain_gui_win/unsecure_lock.png);border-style:outset;border-width:0px;height:22px;width:22px");
}


LockIcon::~LockIcon()
{
}

void LockIcon::setSourceDialog(PopupWindow * popup)
{
	_popup = popup;
}

void LockIcon::mouseMoveEvent(QMouseEvent *event) 
{
	if (_popup != Q_NULLPTR) {
		_popup->move(this->x() - 320, this->y() + 30);
		_popup->setVisible(true);
	}
}

void LockIcon::leaveEvent(QEvent *event) 
{
	if (_popup != Q_NULLPTR) {
		_popup->setVisible(false);
	}
}