#include "global.h"

Global *Global::m_ptr = 0;

Global *Global::ptr() {
    if ( !m_ptr )
	m_ptr = new Global();

    return m_ptr;
}

Global::Global() {
    m_networkProxy = new QNetworkProxy();
}

Global::~Global() {
    delete m_networkProxy;
}

QNetworkProxy *Global::networkProxy() {
    return m_networkProxy;
}