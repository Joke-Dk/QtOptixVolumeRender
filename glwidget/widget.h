#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QWidget>
#include "ui_widget.h"

class Widget : public QWidget
{
	Q_OBJECT

public:
	Widget(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Widget();
	public slots:
	void slotDoubleSpinbox_Slider();
	void slotSlider_DoubleSpinbox();
	void slotDoubleSpinbox_Slider2();
	void slotSlider_DoubleSpinbox2();
	void slotDoubleSpinbox_Slider3();
	void slotSlider_DoubleSpinbox3();
private:
	Ui::WidgetClass ui;
};

#endif // WIDGET_H
