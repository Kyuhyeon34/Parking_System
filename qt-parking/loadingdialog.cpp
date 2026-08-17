#include "loadingdialog.h"
#include "ui_loadingdialog.h"

#include <QPixmap>
#include <QTimer>

namespace {
/* The bar loops while waiting; it is a busy indicator, not real progress. */
const int kTickIntervalMs = 40;
const int kTickStep = 2;
}

LoadingDialog::LoadingDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::LoadingDialog),
      m_timer(new QTimer(this))
{
    ui->setupUi(this);

    connect(m_timer, &QTimer::timeout, this, &LoadingDialog::onTick);
    m_timer->start(kTickIntervalMs);

    connect(ui->btnTestEntry, &QPushButton::clicked,
            this, [this]() { finish(Outcome::Entry); });
    connect(ui->btnTestExit, &QPushButton::clicked,
            this, [this]() { finish(Outcome::Exit); });
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

LoadingDialog::~LoadingDialog()
{
    delete ui;
}

void LoadingDialog::setPreview(const QImage &frame)
{
    if (frame.isNull()) {
        return;
    }

    ui->lblPreview->setPixmap(QPixmap::fromImage(frame).scaled(
        ui->lblPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void LoadingDialog::onTick()
{
    int value = ui->progressBar->value() + kTickStep;
    if (value > ui->progressBar->maximum()) {
        value = 0;
    }
    ui->progressBar->setValue(value);
}

void LoadingDialog::reject()
{
    finish(Outcome::Cancelled);
}

void LoadingDialog::finish(Outcome outcome)
{
    m_outcome = outcome;
    m_timer->stop();

    /* Calling the base class directly (not the virtual reject()) avoids
     * recursing back into our own override above. */
    if (outcome == Outcome::Cancelled) {
        QDialog::reject();
    } else {
        QDialog::accept();
    }
}
