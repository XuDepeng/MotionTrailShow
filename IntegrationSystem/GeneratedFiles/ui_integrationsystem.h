/********************************************************************************
** Form generated from reading UI file 'integrationsystem.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INTEGRATIONSYSTEM_H
#define UI_INTEGRATIONSYSTEM_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_IntegrationSystemClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *IntegrationSystemClass)
    {
        if (IntegrationSystemClass->objectName().isEmpty())
            IntegrationSystemClass->setObjectName(QStringLiteral("IntegrationSystemClass"));
        IntegrationSystemClass->resize(600, 400);
        menuBar = new QMenuBar(IntegrationSystemClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        IntegrationSystemClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(IntegrationSystemClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        IntegrationSystemClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(IntegrationSystemClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        IntegrationSystemClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(IntegrationSystemClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        IntegrationSystemClass->setStatusBar(statusBar);

        retranslateUi(IntegrationSystemClass);

        QMetaObject::connectSlotsByName(IntegrationSystemClass);
    } // setupUi

    void retranslateUi(QMainWindow *IntegrationSystemClass)
    {
        IntegrationSystemClass->setWindowTitle(QApplication::translate("IntegrationSystemClass", "IntegrationSystem", 0));
    } // retranslateUi

};

namespace Ui {
    class IntegrationSystemClass: public Ui_IntegrationSystemClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INTEGRATIONSYSTEM_H
