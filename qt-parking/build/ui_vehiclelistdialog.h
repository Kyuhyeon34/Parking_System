/********************************************************************************
** Form generated from reading UI file 'vehiclelistdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VEHICLELISTDIALOG_H
#define UI_VEHICLELISTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_VehicleListDialog
{
public:
    QVBoxLayout *mainLayout;
    QLabel *lblTitle;
    QLabel *lblSummary;
    QTableWidget *tblVehicles;
    QLabel *lblNote;
    QHBoxLayout *buttonLayout;
    QSpacerItem *buttonSpacer;
    QPushButton *btnClose;

    void setupUi(QDialog *VehicleListDialog)
    {
        if (VehicleListDialog->objectName().isEmpty())
            VehicleListDialog->setObjectName(QString::fromUtf8("VehicleListDialog"));
        VehicleListDialog->resize(1000, 560);
        VehicleListDialog->setWindowModality(Qt::ApplicationModal);
        VehicleListDialog->setModal(true);
        mainLayout = new QVBoxLayout(VehicleListDialog);
        mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
        lblTitle = new QLabel(VehicleListDialog);
        lblTitle->setObjectName(QString::fromUtf8("lblTitle"));
        lblTitle->setMinimumSize(QSize(0, 54));
        QFont font;
        font.setPointSize(18);
        font.setBold(true);
        lblTitle->setFont(font);
        lblTitle->setFrameShape(QFrame::StyledPanel);
        lblTitle->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(lblTitle);

        lblSummary = new QLabel(VehicleListDialog);
        lblSummary->setObjectName(QString::fromUtf8("lblSummary"));
        QFont font1;
        font1.setPointSize(11);
        font1.setBold(true);
        lblSummary->setFont(font1);

        mainLayout->addWidget(lblSummary);

        tblVehicles = new QTableWidget(VehicleListDialog);
        tblVehicles->setObjectName(QString::fromUtf8("tblVehicles"));
        QFont font2;
        font2.setPointSize(10);
        tblVehicles->setFont(font2);
        tblVehicles->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblVehicles->setAlternatingRowColors(true);
        tblVehicles->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblVehicles->setSortingEnabled(false);

        mainLayout->addWidget(tblVehicles);

        lblNote = new QLabel(VehicleListDialog);
        lblNote->setObjectName(QString::fromUtf8("lblNote"));
        QFont font3;
        font3.setPointSize(9);
        lblNote->setFont(font3);

        mainLayout->addWidget(lblNote);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        buttonSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonLayout->addItem(buttonSpacer);

        btnClose = new QPushButton(VehicleListDialog);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));
        btnClose->setMinimumSize(QSize(130, 44));
        QFont font4;
        font4.setPointSize(12);
        font4.setBold(true);
        btnClose->setFont(font4);

        buttonLayout->addWidget(btnClose);


        mainLayout->addLayout(buttonLayout);


        retranslateUi(VehicleListDialog);

        QMetaObject::connectSlotsByName(VehicleListDialog);
    } // setupUi

    void retranslateUi(QDialog *VehicleListDialog)
    {
        VehicleListDialog->setWindowTitle(QCoreApplication::translate("VehicleListDialog", "\354\243\274\354\260\250 \354\260\250\353\237\211 \354\241\260\355\232\214", nullptr));
        lblTitle->setText(QCoreApplication::translate("VehicleListDialog", "\354\243\274\354\260\250 \354\260\250\353\237\211 \354\241\260\355\232\214", nullptr));
        lblSummary->setText(QCoreApplication::translate("VehicleListDialog", "\354\243\274\354\260\250 \354\260\250\353\237\211 : 0 / 10 \353\214\200", nullptr));
        lblNote->setText(QCoreApplication::translate("VehicleListDialog", "\342\200\273 \352\262\260\354\240\234 \352\270\210\354\225\241\354\235\200 \354\241\260\355\232\214 \354\213\234\354\240\220 \352\270\260\354\244\200\354\235\264\353\251\260, \354\202\254\354\240\204\354\240\225\354\202\260\353\220\234 \354\260\250\353\237\211\354\235\200 \355\231\225\354\240\225 \352\270\210\354\225\241\354\236\205\353\213\210\353\213\244.", nullptr));
        btnClose->setText(QCoreApplication::translate("VehicleListDialog", "\353\213\253\352\270\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class VehicleListDialog: public Ui_VehicleListDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VEHICLELISTDIALOG_H
