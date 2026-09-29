#pragma once
#include <QObject>

class QWidget;

class UpdateChecker : public QObject
{
public:
    explicit UpdateChecker(QObject* parent = nullptr) : QObject(parent) {}
    void check(QWidget* parent);
};
