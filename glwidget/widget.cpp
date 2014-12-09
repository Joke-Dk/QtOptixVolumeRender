#include "widget.h"

Widget::Widget(QWidget *parent, Qt::WFlags flags)
: QWidget(parent, flags)
{
	ui.setupUi(this);
}

Widget::~Widget()
{

}
