#include "MiscUtil.h"
#include <windows.h>
#include "LineEditDialog.h"
#include <QMessageBox>
#include <QIcon>

void SetApplicationIcon(WId winId)
{
    HICON hIcon = LoadIcon(GetModuleHandleW(0), MAKEINTRESOURCE(100));
    SendMessageW((HWND)winId, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    DestroyIcon(hIcon);
}

QByteArray & ByteReverse(QByteArray & array)
{
    int length = array.length();
    for(int i = 0; i < length / 2; i++)
    {
        char temp = array[i];
        array[i] = array[length - i - 1];
        array[length - i - 1] = temp;
    }
    return array;
}

QString SimpleInputBox(QWidget* parent, const QString & title, QString defaultValue)
{
    LineEditDialog mEdit(parent);
    mEdit.setWindowIcon(parent->windowIcon());
    mEdit.setText(defaultValue);
    mEdit.setWindowTitle(title);
    mEdit.setCheckBox(false);
    if(mEdit.exec() == QDialog::Accepted)
        return mEdit.editText;
    else
        return "";
}

void SimpleErrorBox(QWidget* parent, const QString & title, const QString & text)
{
    QMessageBox msg(QMessageBox::Critical, title, text, QMessageBox::NoButton, parent);
    msg.setWindowIcon(QIcon(":/icons/images/compile-error.png"));
    msg.setParent(parent, Qt::Dialog);
    msg.setWindowFlags(msg.windowFlags() & (~Qt::WindowContextHelpButtonHint));
    msg.exec();
}

void SimpleWarningBox(QWidget* parent, const QString & title, const QString & text)
{
    QMessageBox msg(QMessageBox::Warning, title, text, QMessageBox::NoButton, parent);
    msg.setWindowIcon(QIcon(":/icons/images/compile-warning.png"));
    msg.setParent(parent, Qt::Dialog);
    msg.setWindowFlags(msg.windowFlags() & (~Qt::WindowContextHelpButtonHint));
    msg.exec();
}
