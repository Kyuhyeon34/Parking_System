#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include <QDialog>
#include <QImage>

QT_BEGIN_NAMESPACE
namespace Ui { class LoadingDialog; }
QT_END_NAMESPACE

class QTimer;

/* Stands in for plate recognition. It only reports which branch the caller
 * should take; MainWindow owns the flow and opens the next dialog itself.
 *
 * Once real recognition lands, the test buttons get replaced by a
 * recognition result and MainWindow decides Entry vs Exit by looking the
 * plate up in the store. */
class LoadingDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Outcome { Cancelled, Entry, Exit };

    explicit LoadingDialog(QWidget *parent = nullptr);
    ~LoadingDialog() override;

    void setPreview(const QImage &frame);
    Outcome outcome() const { return m_outcome; }

protected:
    /* Escape closes a QDialog by calling this directly, bypassing whatever
     * a button's clicked() handler would normally do. Routing it through
     * finish() keeps the timer-stop and outcome bookkeeping consistent
     * regardless of how the dialog gets cancelled. */
    void reject() override;

private:
    void onTick();
    void finish(Outcome outcome);

    Ui::LoadingDialog *ui;
    QTimer *m_timer;
    Outcome m_outcome = Outcome::Cancelled;
};

#endif
