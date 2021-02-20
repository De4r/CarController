#ifndef SENSORSCONTROLLER_H
#define SENSORSCONTROLLER_H

#include <QObject>

class SensorsController : public QObject
{
    Q_OBJECT
public:
    explicit SensorsController(QObject *parent = nullptr);

signals:

};

#endif // SENSORSCONTROLLER_H
