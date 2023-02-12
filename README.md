
# Description

![Yottagram Logo](/Yottagram/icons/128x128/yottagram.png "Yottagram icon") It's a [Telegram](https://telegram.org/ "Telegram website")  client app for [Sailfish OS](https://sailfishos.org/ "Sailfish OS website").
Telegram, is a cloud-based instant messaging and voice over IP service. Telegram client apps are available for Android, iOS, Windows Phone, Windows, macOS and Linux. Users can send messages and exchange photos, videos, stickers, audio and files of any type. [Wikipedia](https://en.wikipedia.org/wiki/Telegram_(software) "Wikipedia article on Telegram")

# Installing

Install from openrepos.

You also need our tdlib build (by mister) from [build.merproject.org](https://build.merproject.org/package/show/home:mister/tdlib).

# Developing

First clone Yottagram with subomdules (recursive flag) or just run
```git submodule update --init --recursive```
afterwards.

After opening it in SailfishOS IDE you need to add to qmake parameters
```DEFINES+="APP_TD_API_ID=<id>" DEFINES+="APP_TD_API_HASH=<hash>"```

Now we gotta build submodules. First enter the sfossdk
```ssh -p 2222 -i ~/Programs/SailfishOS/vmshare/ssh/private_keys/sdk mersdk@localhost```
Enter the target you want to use (repeat for every arch, substitute arch/version) and go to Yottagram directory, also you need to add my repo (substitute arch)
```
sb2 -t SailfishOS-4.4.0.58-armv7hl -R
zypper ar https://repo.sailfishos.org/obs/home:/mister/latest_armv7hl/home:mister.repo
zypper ref
cd /home/…/Yottagram
```

First we need to build tg_owt
```
cd Yottagram/vendor/tg_owt/upstream
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=Off -DTG_OWT_USE_PIPEWIRE=off
```
you will get errors so install necessary packages. For example:
```
libjpeg-turbo-devel
protobuf-devel
ffmpeg-devel
opus-devel
libvpx-devel
alsa-lib-devel
pulseaudio-devel
abseil-cpp-devel
```
then you can build it
```make -j$(nproc)```
copy libtg_owt.a to 
```Yottagram/vendor/tg_owt/prebuilds/(arm|arm64|i386)/```
and we're done with tg_owt

You need to also add my repo to sfossdk target, it's basically same ad what we've done just you'll need to add .default to target name probably so like
```
sb2 -t SailfishOS-4.4.0.58-armv7hl.default -R
zypper ar https://repo.sailfishos.org/obs/home:/mister/latest_armv7hl/home:mister.repo
zypper ref
```

You'll also need to remove -lvoicecall from YottagramVoicecallPlugin/upstream/plugins/plugin.pri
