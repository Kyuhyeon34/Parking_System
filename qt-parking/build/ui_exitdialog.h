/********************************************************************************
** Form generated from reading UI file 'exitdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXITDIALOG_H
#define UI_EXITDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ExitDialog
{
public:
    QVBoxLayout *mainLayout;
    QLabel *lblTitle;
    QHBoxLayout *bodyLayout;
    QLabel *lblCarImage;
    QVBoxLayout *infoLayout;
    QFormLayout *infoForm;
    QLabel *lblCarNumberCaption;
    QLabel *lblCarNumber;
    QLabel *lblEntryTimeCaption;
    QLabel *lblEntryTime;
    QLabel *lblExitTimeCaption;
    QLabel *lblExitTime;
    QLabel *lblParkingTimeCaption;
    QLabel *lblParkingTime;
    QLabel *lblFeeCaption;
    QLabel *lblFee;
    QSpacerItem *infoBottomSpacer;
    QHBoxLayout *buttonLayout;
    QSpacerItem *buttonSpacer;
    QPushButton *btnPay;
    QPushButton *btnCancel;

    void setupUi(QDialog *ExitDialog)
    {
        if (ExitDialog->objectName().isEmpty())
            ExitDialog->setObjectName(QString::fromUtf8("ExitDialog"));
        ExitDialog->resize(780, 480);
        ExitDialog->setWindowModality(Qt::ApplicationModal);
        ExitDialog->setModal(true);
        mainLayout = new QVBoxLayout(ExitDialog);
        mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
        lblTitle = new QLabel(ExitDialog);
        lblTitle->setObjectName(QString::fromUtf8("lblTitle"));
        lblTitle->setMinimumSize(QSize(0, 54));
        QFont font;
        font.setPointSize(18);
        font.setBold(true);
        lblTitle->setFont(font);
        lblTitle->setFrameShape(QFrame::StyledPanel);
        lblTitle->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(lblTitle);

        bodyLayout = new QHBoxLayout();
        bodyLayout->setObjectName(QString::fromUtf8("bodyLayout"));
        lblCarImage = new QLabel(ExitDialog);
        lblCarImage->setObjectName(QString::fromUtf8("lblCarImage"));
        lblCarImage->setMinimumSize(QSize(360, 270));
        lblCarImage->setStyleSheet(QString::fromUtf8("background-color: #1b1b1b; color: #cccccc; border: 1px solid #555;"));
        lblCarImage->setAlignment(Qt::AlignCenter);

        bodyLayout->addWidget(lblCarImage);

        infoLayout = new QVBoxLayout();
        infoLayout->setObjectName(QString::fromUtf8("infoLayout"));
        infoForm = new QFormLayout();
        infoForm->setObjectName(QString::fromUtf8("infoForm"));
        lblCarNumberCaption = new QLabel(ExitDialog);
        lblCarNumberCaption->setObjectName(QString::fromUtf8("lblCarNumberCaption"));
        QFont font1;
        font1.setPointSize(11);
        lblCarNumberCaption->setFont(font1);

        infoForm->setWidget(0, QFormLayout::LabelRole, lblCarNumberCaption);

        lblCarNumber = new QLabel(ExitDialog);
        lblCarNumber->setObjectName(QString::fromUtf8("lblCarNumber"));
        lblCarNumber->setMinimumSize(QSize(190, 0));
        QFont font2;
        font2.setPointSize(15);
        font2.setBold(true);
        lblCarNumber->setFont(font2);

        infoForm->setWidget(0, QFormLayout::FieldRole, lblCarNumber);

        lblEntryTimeCaption = new QLabel(ExitDialog);
        lblEntryTimeCaption->setObjectName(QString::fromUtf8("lblEntryTimeCaption"));
        lblEntryTimeCaption->setFont(font1);

        infoForm->setWidget(1, QFormLayout::LabelRole, lblEntryTimeCaption);

        lblEntryTime = new QLabel(ExitDialog);
        lblEntryTime->setObjectName(QString::fromUtf8("lblEntryTime"));
        lblEntryTime->setFont(font2);

        infoForm->setWidget(1, QFormLayout::FieldRole, lblEntryTime);

        lblExitTimeCaption = new QLabel(ExitDialog);
        lblExitTimeCaption->setObjectName(QString::fromUtf8("lblExitTimeCaption"));
        lblExitTimeCaption->setFont(font1);

        infoForm->setWidget(2, QFormLayout::LabelRole, lblExitTimeCaption);

        lblExitTime = new QLabel(ExitDialog);
        lblExitTime->setObjectName(QString::fromUtf8("lblExitTime"));
        lblExitTime->setFont(font2);

        infoForm->setWidget(2, QFormLayout::FieldRole, lblExitTime);

        lblParkingTimeCaption = new QLabel(ExitDialog);
        lblParkingTimeCaption->setObjectName(QString::fromUtf8("lblParkingTimeCaption"));
        lblParkingTimeCaption->setFont(font1);

        infoForm->setWidget(3, QFormLayout::LabelRole, lblParkingTimeCaption);

        lblParkingTime = new QLabel(ExitDialog);
        lblParkingTime->setObjectName(QString::fromUtf8("lblParkingTime"));
        lblParkingTime->setFont(font2);

        infoForm->setWidget(3, QFormLayout::FieldRole, lblParkingTime);

        lblFeeCaption = new QLabel(ExitDialog);
        lblFeeCaption->setObjectName(QString::fromUtf8("lblFeeCaption"));
        lblFeeCaption->setFont(font1);

        infoForm->setWidget(4, QFormLayout::LabelRole, lblFeeCaption);

        lblFee = new QLabel(ExitDialog);
        lblFee->setObjectName(QString::fromUtf8("lblFee"));
        lblFee->setFont(font);

        infoForm->setWidget(4, QFormLayout::FieldRole, lblFee);


        infoLayout->addLayout(infoForm);

        infoBottomSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        infoLayout->addItem(infoBottomSpacer);


        bodyLayout->addLayout(infoLayout);


        mainLayout->addLayout(bodyLayout);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        buttonSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonLayout->addItem(buttonSpacer);

        btnPay = new QPushButton(ExitDialog);
        btnPay->setObjectName(QString::fromUtf8("btnPay"));
        btnPay->setMinimumSize(QSize(170, 48));
        QFont font3;
        font3.setPointSize(13);
        font3.setBold(true);
        btnPay->setFont(font3);

        buttonLayout->addWidget(btnPay);

        btnCancel = new QPushButton(ExitDialog);
        btnCancel->setObjectName(QString::fromUtf8("btnCancel"));
        btnCancel->setMinimumSize(QSize(120, 48));
        QFont font4;
        font4.setPointSize(12);
        btnCancel->setFont(font4);

        buttonLayout->addWidget(btnCancel);


        mainLayout->addLayout(buttonLayout);


        retranslateUi(ExitDialog);

        QMetaObject::connectSlotsByName(ExitDialog);
    } // setupUi

    void retranslateUi(QDialog *ExitDialog)
    {
        ExitDialog->setWindowTitle(QCoreApplication::translate("ExitDialog", "\354\260\250\353\237\211 \354\266\234\354\260\250 \354\240\225\354\202\260", nullptr));
        lblTitle->setText(QCoreApplication::translate("ExitDialog", "\354\260\250\353\237\211 \354\266\234\354\260\250 \354\240\225\354\202\260", nullptr));
        lblCarImage->setText(QCoreApplication::translate("ExitDialog", "\354\272\241\354\263\220\353\220\234 \354\260\250\353\237\211 \354\235\264\353\257\270\354\247\200", nullptr));
        lblCarNumberCaption->setText(QCoreApplication::translate("ExitDialog", "\354\260\250\353\237\211 \353\262\210\355\230\270 :", nullptr));
        lblCarNumber->setText(QCoreApplication::translate("ExitDialog", "-------", nullptr));
        lblEntryTimeCaption->setText(QCoreApplication::translate("ExitDialog", "\354\236\205\354\260\250 \354\213\234\352\260\204 :", nullptr));
        lblEntryTime->setText(QCoreApplication::translate("ExitDialog", "HH : MM", nullptr));
        lblExitTimeCaption->setText(QCoreApplication::translate("ExitDialog", "\354\266\234\354\260\250 \354\213\234\352\260\204 :", nullptr));
        lblExitTime->setText(QCoreApplication::translate("ExitDialog", "HH : MM", nullptr));
        lblParkingTimeCaption->setText(QCoreApplication::translate("ExitDialog", "\354\243\274\354\260\250 \354\213\234\352\260\204 :", nullptr));
        lblParkingTime->setText(QCoreApplication::translate("ExitDialog", "--", nullptr));
        lblFeeCaption->setText(QCoreApplication::translate("ExitDialog", "\352\262\260\354\240\234 \352\270\210\354\225\241 :", nullptr));
        lblFee->setText(QCoreApplication::translate("ExitDialog", "0 \354\233\220", nullptr));
        btnPay->setText(QCoreApplication::translate("ExitDialog", "\352\262\260\354\240\234\355\225\230\352\270\260", nullptr));
        btnCancel->setText(QCoreApplication::translate("ExitDialog", "\354\267\250\354\206\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ExitDialog: public Ui_ExitDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXITDIALOG_H
