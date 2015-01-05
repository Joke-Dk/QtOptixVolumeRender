#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QWidget>
#include "ui_widget.h"
#include "optix/QTGLUTDisplay.h"
class Widget : public QWidget
{
	Q_OBJECT

	friend class QTGLUTDisplay;
public:
	Widget( QTGLUTDisplay* glWidget, QWidget *parent = 0, Qt::WFlags flags = 0);
	~Widget();

	//////////////////////////////////////////////////////////////////////////
	// Tab: Parameter
	void UpdataParameterAndRefresh(std::string str, float value, bool refresh=true);
	void UpdataParameterAndRefreshInt(std::string str, int value, bool refresh=true);
	void UpdataParameterAndRefreshUInt(std::string str, unsigned int value, bool refresh=true);
	float GetParameterValue(std::string str);
	void SetDeafaultParamater();
	void UpdataEnvironmentLight(int idEnv, bool refresh=true);
	void UpdataHasHDR(bool hasHDR, bool refresh=true);
	void Refresh();
	QTGLUTDisplay* _glWidget;
	public slots:
	void slotDoubleSpinbox_Slider();
	void slotSlider_DoubleSpinbox();
	void slotDoubleSpinbox_Slider2();
	void slotSlider_DoubleSpinbox2();
	void slotDoubleSpinbox_Slider3();
	void slotSlider_DoubleSpinbox3();

	void slotClicked3();
	void slotClicked4();
	void slotDoubleSpinbox_Slider4();
	void slotSlider_DoubleSpinbox4();
	void slotDoubleSpinbox_Slider5();
	void slotSlider_DoubleSpinbox5();
	void slotDoubleSpinbox_Slider6();
	void slotSlider_DoubleSpinbox6();

	void slotClicked1HasArea();
	void slotClicked2HasHDR();
	void slotClicked5HasCornell();
	void slotComboBox();

	void slotRadioButton();
	void slotRadioButton3();
	void slotRadioButton4();
	void slotRadioButton5();

	void slotPushButton();
	void slotPushButton2SaveImage();

	//////////////////////////////////////////////////////////////////////////
	// Tab: Rendering
	void slotPushButton3VolumePath();
	void slotSpinbox1MaxSample();
	void slotSpinbox2MinID();
	void slotSpinbox3MaxID();
	void slotPushButtonRenderingSequence();
private:
	Ui::WidgetClass ui;
};

#endif // WIDGET_H
