#ifndef PULSEAUDIOHELPER_H
#define PULSEAUDIOHELPER_H

#include <QObject>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

class PulseaudioHelper : public QObject
{
    Q_OBJECT
public:
    explicit PulseaudioHelper(QObject *parent = nullptr);
    ~PulseaudioHelper();

    static void successCallback(pa_context *c, int success, void *userdata);

public slots:
    void changeSinkPort(QString port);

signals:

private:
    pa_glib_mainloop* _mainloop;
    pa_mainloop_api* _mainloop_api;
    pa_context* _context;
    pa_signal_event* _signal;

};

#endif // PULSEAUDIOHELPER_H
