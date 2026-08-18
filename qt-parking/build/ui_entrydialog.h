/********************************************************************************
** Form generated from reading UI file 'entrydialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ENTRYDIALOG_H
#define UI_ENTRYDIALOG_H

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

class Ui_EntryDialog
{
public:
    QVBoxLayout *mainLayout;
    QLabel *lblTitle;
    QHBoxLayout *bodyLayout;
    QLabel *lblCarImage;
    QVBoxLayout *infoLayout;
    QSpacerItem *infoTopSpacer;
    QFormLayout *infoForm;
    QLabel *lblCarNumberCaption;
    QLabel *lblCarNumber;
    QLabel *lblEntryTimeCaption;
    QLabel *lblEntryTime;
    QLabel *lblHint;
    QSpacerItem *infoBottomSpacer;
    QHBoxLayout *buttonLayout;
    QSpacerItem *buttonSpacer;
    QPushButton *btnConfirm;
    QPushButton *btnCancel;

    void setupUi(QDialog *EntryDialog)
    {
        if (EntryDialog->objectName().isEmpty())
            EntryDialog->setObjectName(QString::fromUtf8("EntryDialog"));
        EntryDialog->resize(740, 420);
        EntryDialog->setWindowModality(Qt::ApplicationModal);
        EntryDialog->setModal(true);
        mainLayout = new QVBoxLayout(EntryDialog);
        mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
        lblTitle = new QLabel(EntryDialog);
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
        lblCarImage = new QLabel(EntryDialog);
        lblCarImage->setObjectName(QString::fromUtf8("lblCarImage"));
        lblCarImage->setMinimumSize(QSize(360, 270));
        lblCarImage->setStyleSheet(QString::fromUtf8("background-color: #1b1b1b; color: #cccccc; border: 1px solid #555;"));
        lblCarImage->setAlignment(Qt::AlignCenter);

        bodyLayout->addWidget(lblCarImage);

        infoLayout = new QVBoxLayout();
        infoLayout->setObjectName(QString::fromUtf8("infoLayout"));
        infoTopSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        infoLayout->addItem(infoTopSpacer);

        infoForm = new QFormLayout();
        infoForm->setObjectName(QString::fromUtf8("infoForm"));
        lblCarNumberCaption = new QLabel(EntryDialog);
        lblCarNumberCaption->setObjectName(QString::fromUtf8("lblCarNumberCaption"));
        QFont font1;
        font1.setPointSize(12);
        lblCarNumberCaption->setFont(font1);

        infoForm->setWidget(0, QFormLayout::LabelRole, lblCarNumberCaption);

        lblCarNumber = new QLabel(EntryDialog);
        lblCarNumber->setObjectName(QString::fromUtf8("lblCarNumber"));
        lblCarNumber->setMinimumSize(QSize(170, 0));
        QFont font2;
        font2.setPointSize(16);
        font2.setBold(true);
        lblCarNumber->setFont(font2);

        infoForm->setWidget(0, QFormLayout::FieldRole, lblCarNumber);

        lblEntryTimeCaption = new QLabel(EntryDialog);
        lblEntryTimeCaption->setObjectName(QString::fromUtf8("lblEntryTimeCaption"));
        lblEntryTimeCaption->setFont(font1);

        infoForm->setWidget(1, QFormLayout::LabelRole, lblEntryTimeCaption);

        lblEntryTime = new QLabel(EntryDialog);
        lblEntryTime->setObjectName(QString::fromUtf8("lblEntryTime"));
        lblEntryTime->setFont(font2);

        infoForm->setWidget(1, QFormLayout::FieldRole, lblEntryTime);


        infoLayout->addLayout(infoForm);

        lblHint = new QLabel(EntryDialog);
        lblHint->setObjectName(QString::fromUtf8("lblHint"));
        QFont font3;
        font3.setPointSize(10);
        lblHint->setFont(font3);
        lblHint->setWordWrap(true);

        infoLayout->addWidget(lblHint);

        infoBottomSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        infoLayout->addItem(infoBottomSpacer);


        bodyLayout->addLayout(infoLayout);


        mainLayout->addLayout(bodyLayout);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        buttonSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonLayout->addItem(buttonSpacer);

        btnConfirm = new QPushButton(EntryDialog);
        btnConfirm->setObjectName(QString::fromUtf8("btnConfirm"));
        btnConfirm->setMinimumSize(QSize(130, 44));
        QFont font4;
        font4.setPointSize(12);
        font4.setBold(true);
        btnConfirm->setFont(font4);

        buttonLayout->addWidget(btnConfirm);

        btnCancel = new QPushButton(EntryDialog);
        btnCancel->setObjectName(QString::fromUtf8("btnCancel"));
        btnCancel->setMinimumSize(QSize(130, 44));
        btnCancel->setFont(font1);

        buttonLayout->addWidget(btnCancel);


        mainLayout->addLayout(buttonLayout);


        retranslateUi(EntryDialog);

        QMetaObject::connectSlotsByName(EntryDialog);
    } // setupUi

    void retranslateUi(QDialog *EntryDialog)
    {
        EntryDialog->setWindowTitle(QCoreApplication::translate("EntryDialog", "\354\260\250\353\237\211 \354\236\205\354\260\250 \355\231\225\354\235\270", nullptr));
        lblTitle->setText(QCoreApplication::translate("EntryDialog", "\354\260\250\353\237\211 \354\236\205\354\260\250 \355\231\225\354\235\270", nullptr));
        lblCarImage->setText(QCoreApplication::translate("EntryDialog", "\354\272\241\354\263\220\353\220\234 \354\260\250\353\237\211 \354\235\264\353\257\270\354\247\200", nullptr));
        lblCarNumberCaption->setText(QCoreApplication::translate("EntryDialog", "\354\260\250\353\237\211 \353\262\210\355\230\270 :", nullptr));
        lblCarNumber->setText(QCoreApplication::translate("EntryDialog", "-------", nullptr));
        lblEntryTimeCaption->setText(QCoreApplication::translate("EntryDialog", "\354\236\205\354\260\250 \354\213\234\352\260\204 :", nullptr));
        lblEntryTime->setText(QCoreApplication::translate("EntryDialog", "HH : MM", nullptr));
        lblHint->setText(QCoreApplication::translate("EntryDialog", "\355\231\225\354\235\270\354\235\204 \353\210\204\353\245\264\353\251\264 \354\260\250\353\213\250\352\270\260\352\260\200 \354\227\264\353\246\275\353\213\210\353\213\244.", nullptr));
        btnConfirm->setText(QCoreApplication::translate("EntryDialog", "\355\231\225\354\235\270", nullptr));
        btnCancel->setText(QCoreApplication::translate("EntryDialog", "\354\267\250\354\206\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class EntryDialog: public Ui_EntryDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ENTRYDIALOG_H
