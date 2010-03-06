#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtNetwork>

class Global 
{
public:
    static Global *ptr();
    ~Global();

    QNetworkProxy *networkProxy();

private:
    Global();

    QNetworkProxy *m_networkProxy;

    static Global *m_ptr;
};

#endif // GLOBAL_H
