#include "optix/PathTracerScene.h"
#include "widget.h"
#include <QFileDialog>

Widget::Widget(QTGLUTDisplay* glWidget, QWidget *parent, Qt::WindowFlags flags)
: QWidget(parent, flags)
{
	//////////////////////////////////////////////////////////////////////////
	// Tab: Parameter
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
	ui.comboBox->addItem(  "rnl"); 
	ui.comboBox->addItem(  "stpeters"); 
	ui.comboBox->addItem(  "Octane Studio 4"); 
	ui.comboBox->addItem(  "DH001 LL"); 
	ui.comboBox->addItem(  "DH037 LL"); 
	ui.comboBox->addItem(  "DH053 LL");  
	ui.comboBox->addItem(  "Ennis");  
	ui.comboBox->addItem(  "Grace Latlong");  
	ui.comboBox->addItem(  "Window Studio");  
	ui.comboBox->addItem(  "Studio 019");  
	connect( ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotComboBox()));

	// UI: Method selection: Monte Carlo and FLD
	connect(ui.radioButton, SIGNAL(toggled(bool)), this, SLOT(slotRadioButton()));
	connect(ui.radioButton_3, SIGNAL(toggled(bool)), this, SLOT(slotRadioButton3()));	
	connect(ui.radioButton_4, SIGNAL(toggled(bool)), this, SLOT(slotRadioButton4()));	
	connect(ui.radioButton_5, SIGNAL(toggled(bool)), this, SLOT(slotRadioButton5()));	
	//SetDeafaultParamater();

	// UI: FLD Precompution button
	connect(ui.pushButton, SIGNAL(clicked(bool)), this, SLOT(slotPushButton()));

	// UI: Monte Carlo Method
	connect(ui.radioButton_woodcock, SIGNAL(toggled(bool)), this, SLOT(slotRadioButtonWoodcockSwith()));

	// UI: Save Image button
	connect(ui.pushButton_2, SIGNAL(clicked(bool)), this, SLOT(slotPushButton2SaveImage()));

	//////////////////////////////////////////////////////////////////////////
	// Tab: Rendering
	connect(ui.pushButton_VolumePath, SIGNAL(clicked(bool)), this, SLOT(slotPushButton3VolumePath()));
	connect(ui.spinBox, SIGNAL(valueChanged( int)), this, SLOT(slotSpinbox1MaxSample()));
	connect(ui.spinBox_2, SIGNAL(valueChanged( int)), this, SLOT(slotSpinbox2MinID()));
	connect(ui.spinBox_3, SIGNAL(valueChanged( int)), this, SLOT(slotSpinbox3MaxID()));
	connect(ui.pushButton_3Rendering, SIGNAL(clicked(bool)), this, SLOT(slotPushButtonRenderingSequence()));

	connect(ui.pushButton_Pause, SIGNAL(clicked(bool)), this, SLOT(slotPushButtonPause()));
	
	// UI: Rendering Parameter
	connect(ui.spinBox_AntiAlias, SIGNAL(valueChanged( int)), this, SLOT(slotSpinboxAntiAlias()));
	connect(ui.spinBox_MaxDepth, SIGNAL(valueChanged( int)), this, SLOT(slotSpinboxMaxDepth()));
	connect(ui.spinBox_SampleLaunchId, SIGNAL(valueChanged( int)), this, SLOT(slotSpinboxSampleLaunchId()));

	SetDeafaultParamater();
	dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene)->_widget = this;
}

optix::Context& Widget::getContext()
{
	return QTGLUTDisplay::_scene->getContext();
}

void Widget::SetDeafaultParamater()
{
	dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene)->initScensor();
	//m_context = QTGLUTDisplay::_scene->getContext();

	ui.doubleSpinBox->setValue( getContext()["sigmaT"]->getFloat());
	ui.doubleSpinBox_2->setValue( getContext()["alpha"]->getFloat());
	ui.doubleSpinBox_3->setValue( getContext()["g"]->getFloat());
	ui.doubleSpinBox_4->setValue( getContext()["CloudCover"]->getFloat());
	ui.doubleSpinBox_5->setValue( getContext()["CloudSharpness"]->getFloat());
	ui.doubleSpinBox_6->setValue( getContext()["radianceMultipler"]->getFloat());
	ui.checkBox->setChecked( getContext()["hasArea"]->getInt());
	ui.checkBox_2->setChecked( getContext()["hasHDR"]->getInt());
	ui.checkBox_3->setChecked( getContext()["isCurve"]->getInt());
	ui.checkBox_4->setChecked( getContext()["hasBackground"]->getInt());
	ui.checkBox_5->setChecked( getContext()["hasCornell"]->getInt());
	UpdataEnvironmentLight( ui.comboBox->currentIndex() , false);
	//UpdataEnvironmentLight( 1, false);
	
	//UpdataParameterAndRefresh("hasCornell", ui.checkBox_5->isChecked()?1.f:0.f, false);
	//UpdataHasHDR( ui.checkBox_2->isChecked(), false);


	UpdataParameterAndRefresh("isFLDMethod", ui.radioButton->isChecked()||ui.radioButton_4->isChecked()?0.f:1.f , false);
	UpdataParameterAndRefresh("isFLDSingle", ui.radioButton_3->isChecked()?1.f:ui.radioButton_5->isChecked()?-1.f:0.f, false);
	UpdataParameterAndRefreshUInt("max_depth", ui.radioButton_4->isChecked()?unsigned int(3):unsigned int(100), false);
	UpdataParameterAndRefresh("isSingle", 0.f, false);
	UpdataParameterAndRefreshInt("MCWoodcock", ui.radioButton_woodcock->isChecked()?1:0, false);
	ui.comboBox->setEnabled(ui.checkBox_2->isChecked());

	//////////////////////////////////////////////////////////////////////////
	// Tab: Rendering
	slotSpinbox1MaxSample();
	slotSpinbox2MinID();
	slotSpinbox3MaxID();
	UpdataParameterAndRefreshInt("doRendering", 1, false);

	UpdataParameterAndRefreshUInt( "sqrt_num_samples", ui.spinBox_AntiAlias->value(), false);
	UpdataParameterAndRefreshUInt( "max_depth", ui.spinBox_MaxDepth->value(), false);
	UpdataParameterAndRefreshInt("sampleLaunchId", ui.spinBox_SampleLaunchId->value(), false);
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
	UpdataParameterAndRefresh("alpha", ui.doubleSpinBox_2->value());
}

void Widget::slotSlider_DoubleSpinbox2()
{
	double scale = 100;
	if(int(ui.doubleSpinBox_2->value()*scale) == ui.horizontalSlider_2->value())
		return;
	ui.doubleSpinBox_2->setValue((double)(ui.horizontalSlider_2->value())/scale);
	UpdataParameterAndRefresh("alpha", ui.doubleSpinBox_2->value());
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
	double scale = 1000;
	if(int(ui.doubleSpinBox_5->value()*scale) == ui.horizontalSlider_5->value())
		return;
	ui.horizontalSlider_5->setValue((int)(ui.doubleSpinBox_5->value()*scale));
	UpdataParameterAndRefresh("CloudSharpness", ui.doubleSpinBox_5->value());
}

void Widget::slotSlider_DoubleSpinbox5()
{
	double scale = 1000;
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
	UpdataParameterAndRefreshInt("isCurve", ui.checkBox_3->isChecked()?1:0);
}

void Widget::slotClicked4()
{
	UpdataParameterAndRefreshInt("hasBackground", ui.checkBox_4->isChecked()?1:0);
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



void Widget::UpdataParameterAndRefresh(std::string str, float value, bool refresh)
{
	getContext()[str.c_str()]->setFloat( value);
	if(refresh)
	{
		getContext()["frame_number"]->setUint( 1);
	}
}

float Widget::GetParameterValue(std::string str)
{
	return getContext()[str.c_str()]->getFloat();
}

int Widget::GetParameterValueInt(std::string str)
{
	return getContext()[str.c_str()]->getInt();
}

void Widget::UpdataParameterAndRefreshInt(std::string str, int value, bool refresh)
{
	getContext()[str.c_str()]->setInt( value);
	if(refresh)
	{
		getContext()["frame_number"]->setUint( 1);
	}
}

void Widget::UpdataParameterAndRefreshUInt(std::string str, unsigned int value, bool refresh)
{
	getContext()[str.c_str()]->setUint( value);
	if(refresh)
	{
		getContext()["frame_number"]->setUint( 1);
	}
}

void Widget::Refresh()
{
	getContext()["frame_number"]->setUint( 1);
}

void Widget::UpdataEnvironmentLight(int idEnv, bool refresh)
{
	PathTracerScene* scene= dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene);
	scene->switchEnvironmentLight( idEnv);
	if(refresh)
	{
		_glWidget->resizeGL(scene->m_width, scene->m_height);
		//scene->PreCompution( );
	}
}

void Widget::slotClicked2HasHDR()
{
	ui.comboBox->setEnabled(ui.checkBox_2->isChecked());
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

//////////////////////////////////////////////////////////////////////////
// 4 - Method Selection
void Widget::slotRadioButton()
{
	UpdataParameterAndRefresh("isFLDMethod", ui.radioButton->isChecked()?0.f:1.f);
}

void Widget::slotRadioButton3()
{
	UpdataParameterAndRefresh("isFLDSingle", ui.radioButton_3->isChecked()?1.f:0.f);
}

void Widget::slotRadioButton4()
{
	UpdataParameterAndRefresh("isFLDMethod", ui.radioButton_4->isChecked()?0.f:1.f, false);
	UpdataParameterAndRefreshUInt("max_depth", ui.radioButton_4->isChecked()?unsigned int(3):unsigned int(100), false);
	UpdataParameterAndRefresh("isSingle", ui.radioButton_4->isChecked()?1.f:0.f);
}

void Widget::slotRadioButton5()
{
	UpdataParameterAndRefresh("isFLDSingle", ui.radioButton_5->isChecked()?-1.f:0.f);
}


void Widget::slotPushButton()
{
	dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene)->PreCompution();
	ui.radioButton_2->setChecked( 1);
}

void Widget::slotRadioButtonWoodcockSwith()
{
	UpdataParameterAndRefreshInt("MCWoodcock", ui.radioButton_woodcock->isChecked()?1:0);
}

void Widget::slotPushButton2SaveImage()
{
	dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene)->SaveImageButton();
}

//////////////////////////////////////////////////////////////////////////
// Tab: Rendering
void Widget::slotPushButton3VolumePath()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load Volume Data"),
		"../../VolumeData", QString("Volume Data(*.dat)"));
	if (fileName=="")
	{
		return;
	}
	ui.lineEdit_4->setText( fileName);
	std::string filePath = dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene)->updateVolumeFilename( fileName.toStdString());
	ui.lineEdit_5->setText( QString::fromStdString(filePath));
	ui.pushButton_3Rendering->setEnabled(1);
	Refresh();
}

void Widget::slotSpinbox1MaxSample()
{
	UpdataParameterAndRefreshInt("SequenceMaxSample", ui.spinBox->value(), false);
}

void Widget::slotSpinbox2MinID()
{
	UpdataParameterAndRefreshInt("SequenceMinID", ui.spinBox_2->value(), false);
	UpdataParameterAndRefreshInt("SequenceCurID", ui.spinBox_2->value(), false);
	if (ui.spinBox_3->value()<ui.spinBox_2->value())
	{
		ui.spinBox_3->setValue( ui.spinBox_2->value());
		slotSpinbox3MaxID();
	}
}

void Widget::slotSpinbox3MaxID()
{
	UpdataParameterAndRefreshInt("SequenceMaxID", ui.spinBox_3->value(), false);
}



void Widget::slotPushButtonRenderingSequence()
{
	//dynamic_cast<PathTracerScene*>(QTGLUTDisplay::_scene)->UpdateID( ui.spinBox_2->value());
	UpdataParameterAndRefreshInt("SequenceCurID", ui.spinBox_2->value(), false);
	UpdataParameterAndRefreshInt("_init_", 1, false);
	ui.pushButton_Pause->setText( "Pause");
	UpdataParameterAndRefreshInt("doRendering", 1, false);
	Refresh();
}

void Widget::addProgressBar( int value)
{
	ui.progressBar->setValue(value);
}

void Widget::slotPushButtonPause()
{
	int doRendering = 1-GetParameterValueInt("doRendering");
	if (doRendering)
	{
		ui.pushButton_Pause->setText( "Pause");
	}
	else
	{
		ui.pushButton_Pause->setText( "Go on");
	}
	UpdataParameterAndRefreshInt("doRendering", doRendering, false);
}

void Widget::slotSpinboxAntiAlias()
{
	UpdataParameterAndRefreshUInt( "sqrt_num_samples", ui.spinBox_AntiAlias->value());
}

void Widget::slotSpinboxMaxDepth()
{
	UpdataParameterAndRefreshUInt( "max_depth", ui.spinBox_MaxDepth->value());
}

void Widget::slotSpinboxSampleLaunchId()
{
	UpdataParameterAndRefreshInt("sampleLaunchId", ui.spinBox_SampleLaunchId->value());
}