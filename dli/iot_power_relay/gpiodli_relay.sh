#!/bin/bash
#
# If UDRC is the HAT then these GPIOs are available:
#  BCM  wPi      cut
#  16  gpio.27   11
#  17  gpio. 0    5
#
# If HD-15 connector is not used then these gpios are available:
#  BCM   wPi    hdr
#  22  gpio. 3  15
#  23  gpio. 4  16
#  24  gpio. 5  18
#  27  gpio. 2  13

gpio_num="17"
wpi_num="gpio. 0"
gpio_mode_cut=5
gpio_state_cut=6

gpio_num="27"
wpi_num="gpio. 2"
gpio_mode_cut=5
gpio_state_cut=6

#gpio_num=16
#wpi_num=gpio.27
#gpio_mode_cut=11
#gpio_state_cut=10

scriptname="`basename $0`"

# ===== function usage
function usage() {
   echo "Usage: $scriptname [on][off]"
   echo
}

# ===== main
mode_gpio="$(gpio readall | grep -i "$wpi_num" | cut -d "|" -f $gpio_mode_cut | tr -d '[:space:]')"

if [ "$mode_gpio" != "OUT" ] ; then
   echo "gpio $gpio_num is in wrong mode: |$mode_gpio|, should be: OUT"
   gpio -g mode $gpio_num out
fi

arg="$1"

case $arg in
   on|On|ON)
      gpio -g write $gpio_num 1
      ;;
   off|Off|OFF)
      gpio -g write $gpio_num 0
      ;;
   ?|h|H)
      usage
      ;;
   *)
      state_gpio="$(gpio readall | grep -i "$wpi_num" | cut -d "|" -f $gpio_state_cut | tr -d '[:space:]')"
#      echo "debug: gpio state: $state_gpio"
      state_str="off"
      if [ "$state_gpio" == "1" ] ; then
         state_str="on"
      fi
      echo "gpio $gpio_num is $state_str"
   ;;
esac

exit 0
