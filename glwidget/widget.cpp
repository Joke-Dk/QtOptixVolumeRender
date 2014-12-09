#include "widget.h"

Widget::Widget(QWidget *parent, Qt::WFlags flags)
: QWidget(parent, flags)
{
	ui.setupUi(this);
	connect(ui.doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotDoubleSpinbox_Slider()));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSlider_DoubleSpinbox()));
	connect(ui.doubleSpinBox_2, SIGNAL(valueChanged(double)), this, SLOT(slotDoubleSpinbox_Slider2()));
	connect(ui.horizontalSlider_2, SIGNAL(valueChanged(int)), this, SLOT(slotSlider_DoubleSpinbox2()));
	connect(ui.doubleSpinBox_3, SIGNAL(valueChanged(double)), this, SLOT(slotDoubleSpinbox_Slider3()));
	connect(ui.horizontalSlider_3, SIGNAL(valueChanged(int)), this, SLOT(slotSlider_DoubleSpinbox3()));
}

void Widget::slotDoubleSpinbox_Slider()
{
	ui.horizontalSlider->setValue((int)(ui.doubleSpinBox->value()*10));
}

void Widget::slotSlider_DoubleSpinbox()
{
	ui.doubleSpinBox->setValue((double)(ui.horizontalSlider->value())/10);
}

void Widget::slotDoubleSpinbox_Slider2()
{
	ui.horizontalSlider_2->setValue((int)(ui.doubleSpinBox_2->value()*100));
}

void Widget::slotSlider_DoubleSpinbox2()
{
	ui.doubleSpinBox_2->setValue((double)(ui.horizontalSlider_2->value())/100);
}

void Widget::slotDoubleSpinbox_Slider3()
{
	ui.horizontalSlider_3->setValue((int)(ui.doubleSpinBox_3->value()*100));
}

void Widget::slotSlider_DoubleSpinbox3()
{
	ui.doubleSpinBox_3->setValue((double)(ui.horizontalSlider_3->value())/100);
}

Widget::~Widget()
{

}
