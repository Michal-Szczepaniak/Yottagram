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

    void setDefaultSink(QString sink);
    QString getDefaultSink() const;
    void setDefaultSource(QString source);
    QString getDefaultSource() const;
    static void sinkInfoListCallback(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void successCallback(pa_context *c, int success, void *userdata);

public slots:
    void changeLoudspeakerMode(bool isLoudspeaker);
    void probeOutputs();

signals:

private:
    pa_glib_mainloop* _mainloop;
    pa_mainloop_api* _mainloop_api;
    pa_context* _context;
    pa_signal_event* _signal;
    QString _defaultSink;
    QString _defaultSource;
    bool _isLoudspeaker;
};

#endif // PULSEAUDIOHELPER_H
