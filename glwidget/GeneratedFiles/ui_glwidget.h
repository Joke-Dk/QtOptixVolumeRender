/********************************************************************************
** Form generated from reading UI file 'glwidget.ui'
**
** Created by: Qt User Interface Compiler version 4.8.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GLWIDGET_H
#define UI_GLWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GLWidgetClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *GLWidgetClass)
    {
        if (GLWidgetClass->objectName().isEmpty())
            GLWidgetClass->setObjectName(QString::fromUtf8("GLWidgetClass"));
        GLWidgetClass->resize(600, 400);
        menuBar = new QMenuBar(GLWidgetClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        GLWidgetClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(GLWidgetClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        GLWidgetClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(GLWidgetClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        GLWidgetClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(GLWidgetClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        GLWidgetClass->setStatusBar(statusBar);

        retranslateUi(GLWidgetClass);

        QMetaObject::connectSlotsByName(GLWidgetClass);
    } // setupUi

    void retranslateUi(QMainWindow *GLWidgetClass)
    {
        GLWidgetClass->setWindowTitle(QApplication::translate("GLWidgetClass", "GLWidget", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class GLWidgetClass: public Ui_GLWidgetClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GLWIDGET_H
