#include "pulseaudiohelper.h"
#include<QDebug>

PulseaudioHelper::PulseaudioHelper(QObject *parent) : QObject(parent)
{
    _mainloop = pa_glib_mainloop_new(NULL);
    if (!_mainloop) {
        fprintf(stderr, "pa_mainloop_new() failed.\n");
    }

    _mainloop_api = pa_glib_mainloop_get_api(_mainloop);
    if (!_mainloop_api) {
        fprintf(stderr, "pa_mainloop_get_api() failed\n");
    }

    _context = pa_context_new(_mainloop_api, "PulseAudio Test");
    if (!_context) {
        fprintf(stderr, "pa_context_new() failed\n");
    }

    if (pa_context_connect(_context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
        fprintf(stderr, "pa_context_connect() failed: %s\n", pa_strerror(pa_context_errno(_context)));
    }
}

PulseaudioHelper::~PulseaudioHelper()
{
    pa_context_disconnect(_context);
    pa_context_unref(_context);
    pa_glib_mainloop_free(_mainloop);
}

void PulseaudioHelper::successCallback(pa_context *, int success, void *)
{
    qDebug() << "successfully changed port " << success;
}

void PulseaudioHelper::changeSinkPort(QString port)
{
    pa_context_set_sink_port_by_name(_context, "sink.primary", port.toLatin1(), &successCallback, nullptr);
}
