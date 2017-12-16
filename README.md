# virtual_input
virtual input device for sunxi ir and kodi


root@OrangePi:~# cd /usr/src/test
root@OrangePi:/usr/src/test# wget https://github.com/rubitwa/virtual_input/archive/master.tar.gz
--2017-12-16 05:29:14--  https://github.com/rubitwa/virtual_input/archive/master.tar.gz
Resolving github.com (github.com)... 192.30.253.112
Connecting to github.com (github.com)|192.30.253.112|:443... connected.
HTTP request sent, awaiting response... 302 Found
Location: https://codeload.github.com/rubitwa/virtual_input/tar.gz/master [following]
--2017-12-16 05:29:16--  https://codeload.github.com/rubitwa/virtual_input/tar.gz/master
Resolving codeload.github.com (codeload.github.com)... 192.30.253.121
Connecting to codeload.github.com (codeload.github.com)|192.30.253.121|:443... connected.
HTTP request sent, awaiting response... 200 OK
Length: unspecified [application/x-gzip]
Saving to: 'master.tar.gz'

master.tar.gz                                 [   <=>                                                                                ]  19.15K  8.99KB/s    in 2.1s

2017-12-16 05:29:19 (8.99 KB/s) - 'master.tar.gz' saved [19610]

root@OrangePi:/usr/src/test# tar -xvf master.tar.gz
virtual_input-master/
virtual_input-master/Makefile
virtual_input-master/README.md
virtual_input-master/virtual_input
virtual_input-master/virtual_input.c
virtual_input-master/virtual_input.conf
virtual_input-master/virtual_input.h
virtual_input-master/virtual_input.service
root@OrangePi:/usr/src/test# cd virtual_input-master/
root@OrangePi:/usr/src/test/virtual_input-master# make install
mkdir -p /opt/virtual_input
install -m 555 virtual_input /opt/virtual_input/virtual_input
install -m 644 virtual_input.service /etc/systemd/system/virtual_input.service
systemctl enable virtual_input
Created symlink from /etc/systemd/system/multi-user.target.wants/virtual_input.service to /etc/systemd/system/virtual_input.service.
root@OrangePi:/usr/src/test/virtual_input-master# /opt/virtual_input/virtual_input
Usage: github.com/rubitwa/virtual_input_for_ir
root@OrangePi:/usr/src/test/virtual_input-master# /opt/virtual_input/virtual_input --create
Creating new config.

Please enter the config file path (example: /etc/virtual_input.conf)
/etc/virtual_input.conf

Please enter the device name (example: IR-Keyboard)
IR-Keyboard

Please enter the input device path (example: /dev/input/event6)
/dev/input/by-path/platform-1f02000.s_cir-event-kbd

Please enter the name for the button (or press <ENTER> to complete the setting)
KEY_ENTER

Now click the button for "KEY_ENTER", you have 30 seconds for this.
New button KEY_ENTER            0x00000009 added.

Please enter the name for the button (or press <ENTER> to complete the setting)
KEY_VOLUMEDOWN

Now click the button for "KEY_VOLUMEDOWN", you have 30 seconds for this.
New button KEY_VOLUMEDOWN       0x00000014 added.

Please enter the name for the button (or press <ENTER> to complete the setting)
KEY_BACK

Now click the button for "KEY_BACK", you have 30 seconds for this.
New button KEY_BACK             0x00000012 added.

Please enter the name for the button (or press <ENTER> to complete the setting)
KEY_ESC

Now click the button for "KEY_ESC", you have 30 seconds for this.
New button KEY_ESC              0x00000010 added.

Please enter the name for the button (or press <ENTER> to complete the setting)
KEY_RIGHT

Now click the button for "KEY_RIGHT", you have 30 seconds for this.
New button KEY_RIGHT            0x0000000A added.

Please enter the name for the button (or press <ENTER> to complete the setting)
KEY_LEFT

Now click the button for "KEY_LEFT", you have 30 seconds for this.
New button KEY_LEFT             0x00000008 added.

Please enter the name for the button (or press <ENTER> to complete the setting)
KEY_DOWN

Now click the button for "KEY_DOWN", you have 30 seconds for this.
New button KEY_DOWN             0x0000000D added.

Please enter the name for the button (or press <ENTER> to complete the setting)
KEY_UP

Now click the button for "KEY_UP", you have 30 seconds for this.
New button KEY_UP               0x00000005 added.

Please enter the name for the button (or press <ENTER> to complete the setting)
KEY_PLAYPAUSE

Now click the button for "KEY_PLAYPAUSE", you have 30 seconds for this.
New button KEY_PLAYPAUSE        0x00000002 added.

Please enter the name for the button (or press <ENTER> to complete the setting)
KEY_POWER

Now click the button for "KEY_POWER", you have 30 seconds for this.
New button KEY_POWER            0x00000000 added.

Please enter the name for the button (or press <ENTER> to complete the setting)

New config file saved to: /etc/virtual_input.conf
^Croot@OrangePi:/usr/src/test/virtual_input-master# cat /etc/virtual_input.conf
name IR-Keyboard
input /dev/input/by-path/platform-1f02000.s_cir-event-kbd
begin codes
  KEY_POWER            0x00000000
  KEY_PLAYPAUSE        0x00000002
  KEY_UP               0x00000005
  KEY_DOWN             0x0000000D
  KEY_LEFT             0x00000008
  KEY_RIGHT            0x0000000A
  KEY_ESC              0x00000010
  KEY_BACK             0x00000012
  KEY_VOLUMEDOWN       0x00000014
  KEY_ENTER            0x00000009
end codes
root@OrangePi:/usr/src/test/virtual_input-master# echo "begin codes fake" >> /etc/virtual_input.conf
root@OrangePi:/usr/src/test/virtual_input-master# echo "KEY_OK  0x000000FF" >> /etc/virtual_input.conf
root@OrangePi:/usr/src/test/virtual_input-master# echo "end codes fake" >> /etc/virtual_input.conf
root@OrangePi:/usr/src/test/virtual_input-master# cat /etc/virtual_input.conf
name IR-Keyboard
input /dev/input/by-path/platform-1f02000.s_cir-event-kbd
begin codes
  KEY_POWER            0x00000000
  KEY_PLAYPAUSE        0x00000002
  KEY_UP               0x00000005
  KEY_DOWN             0x0000000D
  KEY_LEFT             0x00000008
  KEY_RIGHT            0x0000000A
  KEY_ESC              0x00000010
  KEY_BACK             0x00000012
  KEY_VOLUMEDOWN       0x00000014
  KEY_ENTER            0x00000009
end codes
begin codes fake
KEY_OK  0x000000FF
end codes fake
root@OrangePi:/usr/src/test/virtual_input-master# systemctl restart virtual_input
root@OrangePi:/usr/src/test/virtual_input-master#

