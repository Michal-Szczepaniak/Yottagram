
# Description

![Yottagram Logo](/icons/128x128/yottagram.png "Yottagram icon") It's a [Telegram](https://telegram.org/ "Telegram website")  client app for [Sailfish OS](https://sailfishos.org/ "Sailfish OS website").
Telegram, is a cloud-based instant messaging and voice over IP service. Telegram client apps are available for Android, iOS, Windows Phone, Windows, macOS and Linux. Users can send messages and exchange photos, videos, stickers, audio and files of any type. [Wikipedia](https://en.wikipedia.org/wiki/Telegram_(software) "Wikipedia article on Telegram")

# Installing

Install from openrepos.

You also need our tdlib build (by mister) from [build.merproject.org](https://build.merproject.org/package/show/home:mister/tdlib).

# Developing

## Installing tdlib

Download our latest build (by mister) from [build.merproject.org](https://build.merproject.org/package/show/home:mister/tdlib).

To install it in sfos build engine ssh to it, then use sb2 to get onto it.

Assuming SFOS version 3.3.0.16 and tdlib version 1.6.0-1.2.2, as well as installing on both emulator and arm device.

```bash

# SSH into arm device, and install rpm
ssh -p 2222 -i ~/SailfishOS/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost

sb2 -t SailfishOS-3.3.0.16-armv7hl -R -R

zypper install ./share/Downloads/tdlib-1.6.0-1.2.2.jolla.armv7hl.rpm

exit

# Install into build engine for emulator (i486)
sb2 -t SailfishOS-3.3.0.16-i486 -R -R

zypper install ./share/Downloads/tdlib-1.6.0-1.2.2.jolla.i486.rpm

exit

exit

# Copy over rpm to emulator (i486)
rsync -avzP --rsh='ssh -p 2223 -i~/SailfishOS/vmshare/ssh/private_keys/Sailfish_OS-Emulator-latest/nemo' ~/Downloads/tdlib-*.jolla.i486.rpm nemo@localhost:/home/nemo/Downloads

ssh -p 2223 -i ~/SailfishOS/vmshare/ssh/private_keys/Sailfish_OS-Emulator-latest/nemo nemo@localhost

pkcon install-local /home/nemo/Downloads/tdlib-*.jolla.i486.rpm

exit

```

## Importing the project into Sailfish IDE

git clone https://github.com/Mister_Magister/Yottagram.git

...profit
