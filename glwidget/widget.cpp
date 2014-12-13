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

	// UI: Light radiance
	connect(ui.checkBox_4, SIGNAL(clicked(bool)), this, SLOT(slotClicked4()));
	connect(ui.doubleSpinBox_6, SIGNAL(valueChanged(double)), this, SLOT(slotDoubleSpinbox_Slider6()));
	connect(ui.horizontalSlider_6, SIGNAL(valueChanged(int)), this, SLOT(slotSlider_DoubleSpinbox6()));

	// UI: Area box scene area light
	connect(ui.checkBox, SIGNAL(clicked(bool)), this, SLOT(slotClicked1HasArea()));
	connect(ui.checkBox_5, SIGNAL(clicked(bool)), this, SLOT(slotClicked5HasCornell()));
	// UI: Environment kind light choose
	connect(ui.checkBox_2, SIGNAL(clicked(bool)), this, SLOT(slotClicked2HasHDR()));

	ui.comboBox->setEditable(false); 
	ui.comboBox->addItem(  "Cedar City");  
	ui.comboBox->addItem(  "Grace LL");  
	ui.comboBox->addItem(  "Octane Studio 4"); 
	ui.comboBox->addItem(  "DH001 LL"); 
	ui.comboBox->addItem(  "DH037 LL"); 
	ui.comboBox->addItem(  "DH053 LL");  
	ui.comboBox->addItem(  "Ennis");  
	ui.comboBox->addItem(  "Grace Latlong");  
	ui.comboBox->addItem(  "Window Studio");  
	ui.comboBox->addItem(  "Studio 019");  
	connect( ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotComboBox()));

	SetDeafaultParamater();
}

void Widget::SetDeafaultParamater()
{
	UpdataParameterAndRefresh("sigma_t", ui.doubleSpinBox->value(), false);
	UpdataParameterAndRefresh("alpha_value", ui.doubleSpinBox_2->value() , false);
	UpdataParameterAndRefresh("g", ui.doubleSpinBox_3->value() , false);
	UpdataParameterAndRefresh("CloudCover", ui.doubleSpinBox_4->value(), false);
	UpdataParameterAndRefresh("CloudSharpness", ui.doubleSpinBox_5->value(), false);
	UpdataParameterAndRefresh("isCurve", ui.checkBox_3->isChecked()?1.f:0.f, false);
	UpdataParameterAndRefresh("hasBackground", ui.checkBox_4->isChecked()?1.f:0.f, false);
	UpdataParameterAndRefresh("radianceMultipler", ui.doubleSpinBox_6->value(), false);
	UpdataEnvironmentLight( ui.comboBox->currentIndex() , false);
	
	UpdataParameterAndRefresh("hasCornell", ui.checkBox_5->isChecked()?1.f:0.f, false);
	//UpdataHasHDR( ui.checkBox_2->isChecked(), false);
	UpdataParameterAndRefresh("hasArea", ui.checkBox->isChecked()?1.f:0.f, false);
	UpdataParameterAndRefresh("hasHDR", ui.checkBox_2->isChecked()?1.f:0.f, false);
}

void Widget::slotDoubleSpinbox_Slider()
{
	double scale = 10;
	if(int(ui.doubleSpinBox->value()*scale) == ui.horizontalSlider->value())
		return;
	ui.horizontalSlider->setValue((int)(ui.doubleSpinBox->value()*scale));
	UpdataParameterAndRefresh("sigma_t", ui.doubleSpinBox->value());
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
	UpdataParameterAndRefresh("alpha_value", ui.doubleSpinBox_2->value());
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
	UpdataParameterAndRefresh("g", ui.doubleSpinBox_3->value());
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
	UpdataParameterAndRefresh("CloudCover", ui.doubleSpinBox_4->value());
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
	ui.horizontalSlider_5->setValue((int)(ui.doubleSpinBox_5->value()*scale));
	UpdataParameterAndRefresh("CloudSharpness", ui.doubleSpinBox_5->value());
}

void Widget::slotSlider_DoubleSpinbox5()
{
	double scale = 100;
	if(int(ui.doubleSpinBox_5->value()*scale) == ui.horizontalSlider_5->value())
		return;
	ui.doubleSpinBox_5->setValue((double)(ui.horizontalSlider_5->value())/scale);
	UpdataParameterAndRefresh("CloudSharpness", ui.doubleSpinBox_5->value());
}

void Widget::slotDoubleSpinbox_Slider6()
{
	double scale = 10;
	if(int(ui.doubleSpinBox_6->value()*scale) == ui.horizontalSlider_6->value())
		return;
	ui.horizontalSlider_6->setValue((int)(ui.doubleSpinBox_6->value()*scale));
	UpdataParameterAndRefresh("radianceMultipler", ui.doubleSpinBox_6->value());
}

void Widget::slotSlider_DoubleSpinbox6()
{
	double scale = 10;
	if(int(ui.doubleSpinBox_6->value()*scale) == ui.horizontalSlider_6->value())
		return;
	ui.doubleSpinBox_6->setValue((double)(ui.horizontalSlider_6->value())/scale);
	UpdataParameterAndRefresh("radianceMultipler", ui.doubleSpinBox_6->value());
}

void Widget::slotClicked3()
{
	bool isEnabled = ui.checkBox_3->isChecked();
	ui.label_4->setEnabled(isEnabled);
	ui.label_5->setEnabled(isEnabled);
	ui.doubleSpinBox_4->setEnabled(isEnabled);
	ui.doubleSpinBox_5->setEnabled(isEnabled);
	ui.horizontalSlider_4->setEnabled(isEnabled);
	ui.horizontalSlider_5->setEnabled(isEnabled);
	UpdataParameterAndRefresh("isCurve", ui.checkBox_3->isChecked()?1.f:0.f);
}

void Widget::slotClicked4()
{
	UpdataParameterAndRefresh("hasBackground", ui.checkBox_4->isChecked()?1.f:0.f);
	PathTracerScene* scene= dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene);
	scene->updateGeometryInstance();
}


void Widget::slotComboBox()
{
	UpdataEnvironmentLight( ui.comboBox->currentIndex() );
}

Widget::~Widget()
{
}

float Widget::GetParameterValue(std::string str)
{
	PathTracerScene* scene= dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene);
	return scene->getParameter( str);
}

void Widget::UpdataParameterAndRefreshInt(std::string str, int value, bool refresh)
{
	PathTracerScene* scene= dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene);
	scene->updateParameter(str , value);
	if(refresh)
	{
		_glWidget->resizeGL(scene->m_width, scene->m_height);
	}
}

void Widget::UpdataParameterAndRefresh(std::string str, float value, bool refresh)
{
	PathTracerScene* scene= dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene);
	scene->updateParameter(str , value);
	if(refresh)
	{
		_glWidget->resizeGL(scene->m_width, scene->m_height);
	}
}

void Widget::UpdataEnvironmentLight(int idEnv, bool refresh)
{
	PathTracerScene* scene= dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene);
	scene->switchEnvironmentLight( idEnv);
	if(refresh)
	{
		_glWidget->resizeGL(scene->m_width, scene->m_height);
	}
}

void Widget::slotClicked2HasHDR()
{
	ui.groupBox->setEnabled(ui.checkBox_2->isChecked());
	UpdataHasHDR( ui.checkBox_2->isChecked());
}



void Widget::slotClicked5HasCornell()
{
	UpdataParameterAndRefresh("hasCornell", ui.checkBox_5->isChecked()?1.f:0.f, false);
	PathTracerScene* scene= dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene);
	scene->updateGeometryInstance();
	_glWidget->resizeGL(scene->m_width, scene->m_height);
}

//////////////////////////////////////////////////////////////////////////
// 3 - Area Light
void Widget::slotClicked1HasArea()
{
	UpdataParameterAndRefresh("hasArea", ui.checkBox->isChecked()?1.f:0.f, false);
	PathTracerScene* scene= dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene);
	scene->updateHasAreaBox( );
	scene->updateGeometryInstance();
	_glWidget->resizeGL(scene->m_width, scene->m_height);
}


void Widget::UpdataHasHDR(bool hasHDR, bool refresh)
{
	PathTracerScene* scene= dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene);
	scene->switchHasHDR( hasHDR);
	if(refresh)
	{
		_glWidget->resizeGL(scene->m_width, scene->m_height);
	}
}