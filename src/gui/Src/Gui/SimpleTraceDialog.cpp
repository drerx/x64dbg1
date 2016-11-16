#include "SimpleTraceDialog.h"
#include "ui_SimpleTraceDialog.h"
#include "Bridge.h"
#include <QMessageBox>

SimpleTraceDialog::SimpleTraceDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::SimpleTraceDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    duint setting;
    if(!BridgeSettingGetUint("Engine", "MaxTraceCount", &setting))
        setting = 50000;
    ui->spinMaxTraceCount->setValue(int(setting));
    ui->editBreakCondition->setPlaceholderText(tr("Example: eax == 0 && ebx == 0"));
    ui->editLogText->setPlaceholderText(tr("Example: 0x{p:cip} {i:cip}"));
    ui->editLogCondition->setPlaceholderText(tr("Example: eax == 0 && ebx == 0"));
    ui->editCommandText->setPlaceholderText(tr("Example: eax=4;StepOut"));
    ui->editCommandCondition->setPlaceholderText(tr("Example: eax == 0 && ebx == 0"));
}

SimpleTraceDialog::~SimpleTraceDialog()
{
    delete ui;
}

void SimpleTraceDialog::setTraceCommand(const QString & command)
{
    mTraceCommand = command;
}

static QString escapeText(QString str)
{
    str.replace(QChar('\\'), QString("\\\\"));
    str.replace(QChar('"'), QString("\\\""));
    return str;
}

void SimpleTraceDialog::on_btnOk_clicked()
{
    auto logText = ui->editLogText->addHistoryClear();
    auto logCondition = ui->editLogCondition->addHistoryClear();
    if(!DbgCmdExecDirect(QString("TraceSetLog \"%1\", \"%2\"").arg(escapeText(logText), escapeText(logCondition)).toUtf8().constData()))
    {
        QMessageBox::warning(this, tr("Error"), tr("Failed to set log text/condition!"));
        return;
    }
    auto commandText = ui->editCommandText->addHistoryClear();
    auto commandCondition = ui->editCommandCondition->addHistoryClear();
    if(!DbgCmdExecDirect(QString("TraceSetCommand \"%1\", \"%2\"").arg(escapeText(commandText), escapeText(commandCondition)).toUtf8().constData()))
    {
        QMessageBox::warning(this, tr("Error"), tr("Failed to set command text/condition!"));
        return;
    }
    auto breakCondition = ui->editBreakCondition->addHistoryClear();
    auto maxTraceCount = ui->spinMaxTraceCount->value();
    if(!DbgCmdExecDirect(QString("%1 \"%2\", .%3").arg(mTraceCommand, escapeText(breakCondition)).arg(maxTraceCount).toUtf8().constData()))
    {
        QMessageBox::warning(this, tr("Error"), tr("Failed to start trace!"));
        return;
    }
    accept();
}
