#include "pulseaudiohelper.h"
#include <QDebug>
#include <QTimer>
#include <functional>

PulseaudioHelper::PulseaudioHelper(QObject *parent) : QObject(parent)
{
    _mainloop = pa_glib_mainloop_new(NULL);
    if (!_mainloop) {
        qDebug() << "pa_mainloop_new() failed.";
    }

    _mainloop_api = pa_glib_mainloop_get_api(_mainloop);
    if (!_mainloop_api) {
        qDebug() << "pa_mainloop_get_api() failed";
    }

    _context = pa_context_new(_mainloop_api, "Yottagram");
    if (!_context) {
        qDebug() << "pa_context_new() failed";
    }

    if (pa_context_connect(_context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
         qDebug() << "pa_context_connect() failed: " << pa_strerror(pa_context_errno(_context));
    }

    _defaultSink = "sink.deep_buffer";
    _defaultSource = "source.droid";
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

QString PulseaudioHelper::getDefaultSink() const
{
    qDebug() << "get default sink " << _defaultSink;
    return _defaultSink;
}

void PulseaudioHelper::setDefaultSource(QString source)
{
    _defaultSource = source;
    qDebug() << "default source " << source;
}

QString PulseaudioHelper::getDefaultSource() const
{

    qDebug() << "get default source " << _defaultSource;
    return _defaultSource;
}

void PulseaudioHelper::sinkInfoListCallback(pa_context *, const pa_sink_info *i, int, void *userdata)
{
    PulseaudioHelper* helper = (PulseaudioHelper*)userdata;
    if (i) {
        QString name = QString::fromLatin1(i->name);
        if (name == "sink.primary") {
            helper->setDefaultSink(name);
        } else if (name == "sink.deep_buffer") {
            helper->setDefaultSink(name);
        }
    }
}

void PulseaudioHelper::successCallback(pa_context *c, int success, void *userdata)
{
    qDebug() << "succ" << success << " err: " << pa_strerror(pa_context_errno(c));
}

void PulseaudioHelper::changeLoudspeakerMode(bool isLoudspeaker)
{
    qDebug() << "Is loudspeaker: " << (isLoudspeaker ? "true" : "false");
    _isLoudspeaker = isLoudspeaker;

    if (_defaultSink == "sink.primary") {
        pa_context_set_sink_port_by_name(_context, "sink.primary", isLoudspeaker ? "output-speaker" : "output-earpiece", &successCallback, nullptr);
    } else {
        pa_context_set_sink_port_by_name(_context, "sink.primary_output", isLoudspeaker ? "output-speaker" : "output-earpiece", &successCallback, nullptr);
        pa_context_set_sink_port_by_name(_context, "sink.deep_buffer", isLoudspeaker ? "output-speaker" : "output-earpiece", &successCallback, nullptr);
    }
}

void PulseaudioHelper::probeOutputs()
{
    pa_context_get_sink_info_list(_context, &sinkInfoListCallback, this);
}
