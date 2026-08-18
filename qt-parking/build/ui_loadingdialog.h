/********************************************************************************
** Form generated from reading UI file 'loadingdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOADINGDIALOG_H
#define UI_LOADINGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_LoadingDialog
{
public:
    QVBoxLayout *mainLayout;
    QLabel *lblTitle;
    QSpacerItem *topSpacer;
    QLabel *lblPreview;
    QProgressBar *progressBar;
    QLabel *lblMessage1;
    QLabel *lblMessage2;
    QSpacerItem *bottomSpacer;
    QHBoxLayout *testButtonLayout;
    QPushButton *btnTestEntry;
    QPushButton *btnTestExit;
    QPushButton *btnCancel;

    void setupUi(QDialog *LoadingDialog)
    {
        if (LoadingDialog->objectName().isEmpty())
            LoadingDialog->setObjectName(QString::fromUtf8("LoadingDialog"));
        LoadingDialog->resize(560, 360);
        LoadingDialog->setWindowModality(Qt::ApplicationModal);
        LoadingDialog->setModal(true);
        mainLayout = new QVBoxLayout(LoadingDialog);
        mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
        lblTitle = new QLabel(LoadingDialog);
        lblTitle->setObjectName(QString::fromUtf8("lblTitle"));
        lblTitle->setMinimumSize(QSize(0, 54));
        QFont font;
        font.setPointSize(18);
        font.setBold(true);
        lblTitle->setFont(font);
        lblTitle->setFrameShape(QFrame::StyledPanel);
        lblTitle->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(lblTitle);

        topSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        mainLayout->addItem(topSpacer);

        lblPreview = new QLabel(LoadingDialog);
        lblPreview->setObjectName(QString::fromUtf8("lblPreview"));
        lblPreview->setMinimumSize(QSize(0, 130));
        lblPreview->setStyleSheet(QString::fromUtf8("background-color: #1b1b1b; color: #cccccc; border: 1px solid #555;"));
        lblPreview->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(lblPreview);

        progressBar = new QProgressBar(LoadingDialog);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setMinimumSize(QSize(0, 28));
        progressBar->setMaximum(100);
        progressBar->setValue(0);
        progressBar->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(progressBar);

        lblMessage1 = new QLabel(LoadingDialog);
        lblMessage1->setObjectName(QString::fromUtf8("lblMessage1"));
        QFont font1;
        font1.setPointSize(13);
        font1.setBold(true);
        lblMessage1->setFont(font1);
        lblMessage1->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(lblMessage1);

        lblMessage2 = new QLabel(LoadingDialog);
        lblMessage2->setObjectName(QString::fromUtf8("lblMessage2"));
        QFont font2;
        font2.setPointSize(11);
        font2.setBold(false);
        lblMessage2->setFont(font2);
        lblMessage2->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(lblMessage2);

        bottomSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        mainLayout->addItem(bottomSpacer);

        testButtonLayout = new QHBoxLayout();
        testButtonLayout->setObjectName(QString::fromUtf8("testButtonLayout"));
        btnTestEntry = new QPushButton(LoadingDialog);
        btnTestEntry->setObjectName(QString::fromUtf8("btnTestEntry"));
        btnTestEntry->setMinimumSize(QSize(0, 44));
        QFont font3;
        font3.setPointSize(11);
        font3.setBold(true);
        btnTestEntry->setFont(font3);

        testButtonLayout->addWidget(btnTestEntry);

        btnTestExit = new QPushButton(LoadingDialog);
        btnTestExit->setObjectName(QString::fromUtf8("btnTestExit"));
        btnTestExit->setMinimumSize(QSize(0, 44));
        btnTestExit->setFont(font3);

        testButtonLayout->addWidget(btnTestExit);

        btnCancel = new QPushButton(LoadingDialog);
        btnCancel->setObjectName(QString::fromUtf8("btnCancel"));
        btnCancel->setMinimumSize(QSize(0, 44));

        testButtonLayout->addWidget(btnCancel);


        mainLayout->addLayout(testButtonLayout);


        retranslateUi(LoadingDialog);

        QMetaObject::connectSlotsByName(LoadingDialog);
    } // setupUi

    void retranslateUi(QDialog *LoadingDialog)
    {
        LoadingDialog->setWindowTitle(QCoreApplication::translate("LoadingDialog", "\354\260\250\353\237\211 \354\235\270\354\213\235\354\244\221", nullptr));
        lblTitle->setText(QCoreApplication::translate("LoadingDialog", "\354\260\250\353\237\211 \354\235\270\354\213\235\354\244\221", nullptr));
        lblPreview->setText(QCoreApplication::translate("LoadingDialog", "\354\272\241\354\263\220 \354\230\201\354\203\201", nullptr));
        lblMessage1->setText(QCoreApplication::translate("LoadingDialog", "\353\262\210\355\230\270\355\214\220\354\235\204 \354\235\270\354\213\235\355\225\230\352\263\240 \354\236\210\354\212\265\353\213\210\353\213\244", nullptr));
        lblMessage2->setText(QCoreApplication::translate("LoadingDialog", "\354\236\240\354\213\234\353\247\214 \352\270\260\353\213\244\353\240\244\354\243\274\354\204\270\354\232\224\342\200\246", nullptr));
        btnTestEntry->setText(QCoreApplication::translate("LoadingDialog", "\354\210\230\353\217\231 \354\236\205\354\260\250", nullptr));
        btnTestExit->setText(QCoreApplication::translate("LoadingDialog", "\354\210\230\353\217\231 \354\266\234\354\260\250", nullptr));
        btnCancel->setText(QCoreApplication::translate("LoadingDialog", "\354\267\250\354\206\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoadingDialog: public Ui_LoadingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOADINGDIALOG_H
