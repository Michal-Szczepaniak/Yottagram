#include "pulseaudiohelper.h"
#include <QDebug>
#include <QTimer>
#include <functional>

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

    _context = pa_context_new(_mainloop_api, "Yottagram");
    if (!_context) {
        fprintf(stderr, "pa_context_new() failed\n");
    }

    if (pa_context_connect(_context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
        fprintf(stderr, "pa_context_connect() failed: %s\n", pa_strerror(pa_context_errno(_context)));
    }

    _defaultSink = "sink.primary";
}

PulseaudioHelper::~PulseaudioHelper()
{
    pa_context_disconnect(_context);
    pa_context_unref(_context);
    pa_glib_mainloop_free(_mainloop);
}

void PulseaudioHelper::setDefaultSink(QString sink)
{
    _defaultSink = sink;
    qDebug() << "default sink " << sink;
}

void PulseaudioHelper::sinkInfoListCallback(pa_context *, const pa_sink_info *i, int, void *userdata)
{
    PulseaudioHelper* helper = (PulseaudioHelper*)userdata;
    if (i && i->state == PA_SINK_RUNNING) {
        qDebug() << "sink " << QString::fromLatin1(i->name);
        helper->setDefaultSink(QString::fromLatin1(i->name));
        pa_context_set_sink_port_by_index(helper->_context, i->index, helper->_port.toLatin1(), &successCallback, nullptr);
    }
}

void PulseaudioHelper::successCallback(pa_context *c, int success, void *userdata)
{
    qDebug() << "succ" << success;
}

void PulseaudioHelper::changeSinkPort(QString port)
{
    qDebug() << _port;
    _port = port;
    pa_context_get_sink_info_list(_context, &sinkInfoListCallback, this);
}
