function formatMute(muteFor) {
    if (muteFor == 0) return "";
    if (muteFor > 0 && muteFor < 60) return qsTr("%L1 second(s)").arg(muteFor);
    var minuteMuteFor = Math.floor(muteFor/60);
    if (minuteMuteFor < 60) return qsTr("%L1 minute(s)").arg(minuteMuteFor);
    var hourMuteFor = Math.floor(minuteMuteFor/60);
    if (hourMuteFor < 24) return qsTr("%L1 hour(s)").arg(hourMuteFor);
    var dayMuteFor = Math.floor(hourMuteFor/24);
    if (dayMuteFor > 2) return ""
    return qsTr("%L1 day(s)").arg(dayMuteFor);
}

function appendSpace(value) {
    if (value !== "") return value + " "
    return value
}

function formatAutoDestruct(value) {
    if (value == 0) return "";
    if (value > 0 && value < 60) return qsTr("%L1").arg(muteFor);
    var minuteMuteFor = Math.floor(muteFor/60);
    if (minuteMuteFor < 60) return qsTr("%L1m").arg(minuteMuteFor);
    var hourMuteFor = Math.floor(minuteMuteFor/60);
    if (hourMuteFor < 24) return qsTr("%L1h").arg(hourMuteFor);
    var dayMuteFor = Math.floor(hourMuteFor/24);
    if (dayMuteFor > 2) return ""
    return qsTr("%L1d").arg(dayMuteFor);
}
