#!/bin/bash
#
# Setup udev rules to use usb driver as not root
#
# Uncomment this statement for debug echos
DEBUG=1
#
scriptname="`basename $0`"
USER="pi"
GROUP="usb"
udev_usb_file="/etc/udev/rules.d/99-usbftdi.rules"

# Be sure we're running as root
if [[ $EUID != 0 ]] ; then
   echo "Must be root."
   exit 1
fi

# check if group usb exists

if [ $(getent group $GROUP) ]; then
  echo "group $GROUP exists."
else
  echo "group $GROUP does not exist...adding"
  /usr/sbin/groupadd $GROUP
fi

# check if user is in group usb
if id -nG "$USER" | grep -qw "$GROUP"; then
    echo $USER belongs to $GROUP
else
    echo $USER does not belong to group $GROUP
    usermod -a -G $GROUP $USER
fi

if [ ! -f $udev_usb_file ] ; then
   echo "Set udev permissions for ftdi usb device"
   echo "SUBSYSTEMS==\"usb\", ACTION==\"add\", MODE=\"0664\", GROUP=\"$GROUP\"" >> $udev_usb_file

   echo " Reload udev rules"
   /etc/init.d/udev reload
   udevadm trigger
fi

echo "$scriptname done, may need to reboot $(hostname)"
