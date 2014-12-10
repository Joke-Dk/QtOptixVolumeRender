#include "optix/PathTracerScene.h"
#include "widget.h"


Widget::Widget( QTGLUTDisplay* glWidget,  QWidget *parent, Qt::WFlags flags)
: QWidget(parent, flags)
{
	_glWidget = glWidget;
	ui.setupUi(this);
	connect(ui.doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotDoubleSpinbox_Slider()));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSlider_DoubleSpinbox()));
	connect(ui.doubleSpinBox_2, SIGNAL(valueChanged(double)), this, SLOT(slotDoubleSpinbox_Slider2()));
	connect(ui.horizontalSlider_2, SIGNAL(valueChanged(int)), this, SLOT(slotSlider_DoubleSpinbox2()));
	connect(ui.doubleSpinBox_3, SIGNAL(valueChanged(double)), this, SLOT(slotDoubleSpinbox_Slider3()));
	connect(ui.horizontalSlider_3, SIGNAL(valueChanged(int)), this, SLOT(slotSlider_DoubleSpinbox3()));

	// UI: Volume curve
	connect(ui.checkBox_3, SIGNAL(clicked(bool)), this, SLOT(slotClicked3()));
	
	connect(ui.doubleSpinBox_4, SIGNAL(valueChanged(double)), this, SLOT(slotDoubleSpinbox_Slider4()));
	connect(ui.horizontalSlider_4, SIGNAL(valueChanged(int)), this, SLOT(slotSlider_DoubleSpinbox4()));
	connect(ui.doubleSpinBox_5, SIGNAL(valueChanged(double)), this, SLOT(slotDoubleSpinbox_Slider5()));
	connect(ui.horizontalSlider_5, SIGNAL(valueChanged(int)), this, SLOT(slotSlider_DoubleSpinbox5()));
}

void Widget::slotDoubleSpinbox_Slider()
{
	double scale = 10;
	if(int(ui.doubleSpinBox->value()*scale) == ui.horizontalSlider->value())
		return;
	ui.horizontalSlider->setValue((int)(ui.doubleSpinBox->value()*scale));
}

void Widget::slotSlider_DoubleSpinbox()
{
	double scale = 10;
	if(int(ui.doubleSpinBox->value()*scale) == ui.horizontalSlider->value())
		return;
	ui.doubleSpinBox->setValue((double)(ui.horizontalSlider->value())/scale);
	UpdataParameterAndRefresh("sigma_t", ui.doubleSpinBox->value());

}

void Widget::slotDoubleSpinbox_Slider2()
{
	double scale = 100;
	if(int(ui.doubleSpinBox_2->value()*scale) == ui.horizontalSlider_2->value())
		return;
	ui.horizontalSlider_2->setValue((int)(ui.doubleSpinBox_2->value()*scale));
}

void Widget::slotSlider_DoubleSpinbox2()
{
	double scale = 100;
	if(int(ui.doubleSpinBox_2->value()*scale) == ui.horizontalSlider_2->value())
		return;
	ui.doubleSpinBox_2->setValue((double)(ui.horizontalSlider_2->value())/scale);
	UpdataParameterAndRefresh("alpha_value", ui.doubleSpinBox_2->value());
}

void Widget::slotDoubleSpinbox_Slider3()
{
	double scale = 100;
	if(int(ui.doubleSpinBox_3->value()*scale) == ui.horizontalSlider_3->value())
		return;
	ui.horizontalSlider_3->setValue((int)(ui.doubleSpinBox_3->value()*scale));
}

void Widget::slotSlider_DoubleSpinbox3()
{
	double scale = 100;
	if(int(ui.doubleSpinBox_3->value()*scale) == ui.horizontalSlider_3->value())
		return;
	ui.doubleSpinBox_3->setValue((double)(ui.horizontalSlider_3->value())/scale);
	UpdataParameterAndRefresh("g", ui.doubleSpinBox_3->value());
}

void Widget::slotDoubleSpinbox_Slider4()
{
	double scale = 100;
	if(int(ui.doubleSpinBox_4->value()*scale) == ui.horizontalSlider_4->value())
		return;
	ui.horizontalSlider_4->setValue((int)(ui.doubleSpinBox_4->value()*scale));
}

void Widget::slotSlider_DoubleSpinbox4()
{
	double scale = 100;
	if(int(ui.doubleSpinBox_4->value()*scale) == ui.horizontalSlider_4->value())
		return;
	ui.doubleSpinBox_4->setValue((double)(ui.horizontalSlider_4->value())/scale);
	UpdataParameterAndRefresh("CloudCover", ui.doubleSpinBox_4->value());
}

void Widget::slotDoubleSpinbox_Slider5()
{
	double scale = 100;
	if(int(ui.doubleSpinBox_5->value()*scale) == ui.horizontalSlider_5->value())
		return;
	ui.horizontalSlider_3->setValue((int)(ui.doubleSpinBox_5->value()*scale));
}

void Widget::slotSlider_DoubleSpinbox5()
{
	double scale = 100;
	if(int(ui.doubleSpinBox_5->value()*scale) == ui.horizontalSlider_5->value())
		return;
	ui.doubleSpinBox_5->setValue((double)(ui.horizontalSlider_5->value())/scale);
	UpdataParameterAndRefresh("CloudSharpness", ui.doubleSpinBox_5->value());
}

void Widget::slotClicked3()
{
	UpdataParameterAndRefresh("isCurve", 1.f-GetParameterValue("isCurve"));
}

Widget::~Widget()
{

}

float Widget::GetParameterValue(std::string str)
{
	PathTracerScene* scene= dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene);
	return scene->getParameter( str);
}

void Widget::UpdataParameterAndRefresh(std::string str, float value)
{
	PathTracerScene* scene= dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene);
	scene->updateParameter(str , value);
	_glWidget->resizeGL(scene->m_width, scene->m_height);
	//_glWidget->resizeGL(width(), height());
	//_glWidget->setCurContinuousMode(QTGLUTDisplay::CDProgressive);

}