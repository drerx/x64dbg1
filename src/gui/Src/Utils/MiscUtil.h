#ifndef MISCUTIL_H
#define MISCUTIL_H

#include <QWidget>

void SetApplicationIcon(WId winId);
QByteArray & ByteReverse(QByteArray & array);
QString SimpleInputBox(QWidget* parent, const QString & title, QString defaultValue = "");
void SimpleErrorBox(QWidget* parent, const QString & title, const QString & text);
void SimpleWarningBox(QWidget* parent, const QString & title, const QString & text);

#endif // MISCUTIL_H
