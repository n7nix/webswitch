#!/bin/bash
#
# dli_pumphouse.sh
# Use a Digital Loggers IoT Relay and a Sunfounders
# humidity/temperature sensor to control a light for heat in the
# pumphouse.
# dht11_temp is a C program that reads the DHT11 sensor and returns a
# temperature as an integer in  degrees Fahrenheit.
#
# ==== Which RPi GPIOs to use
# If UDRC HAT is used then these GPIOs are available:
#  BCM  wPi      cut
#  16  gpio.27   11
#  17  gpio. 0    5
#
# If the HD-15 connector on an UDRC/UDRC II is not used then these gpios
# are available:
#
#  BCM   wPi    hdr
#  22  gpio. 3  15
#  23  gpio. 4  16
#  24  gpio. 5  18
#  27  gpio. 2  13
# DEBUG=1

# set temperatures for when to turn light on/off
UPPER_TEMP=42
LOWER_TEMP=36

USER=$(whoami)
scriptname="`basename $0`"
read_temp="/home/$USER/bin/dht11_temp"
PUMPHOUSE_LOGFILE="/home/$USER/var/log/pumphouse.log"

# for email
SENDTO="gunn@beeble.localnet"
tmpdir="$HOME/tmp"
emailbody="${tmpdir}/emailtemp.txt"

# ==== Define which RPI gpio's to use
# cut defines are for the gpio readall command
#gpio_num="17"
#wpi_num="gpio. 0"
#gpio_mode_cut=5
#gpio_state_cut=6

gpio_num="27"
wpi_num="gpio. 2"
gpio_mode_cut=5
gpio_state_cut=6

#gpio_num=16
#wpi_num=gpio.27
#gpio_mode_cut=11
#gpio_state_cut=10

function dbgecho { if [ ! -z "$DEBUG" ] ; then echo "$*"; fi }

# ===== function usage
function usage() {
   echo "Usage: $scriptname [on][off]"
   echo
}
# ===== function pump_heater_ctrl
function pump_heater_ctrl() {
    on_off=$1
    gpio -g write $gpio_num $on_off
}

# ==== function get_temp()
function get_temp() {
    passcnt=0
    retcode=1
    while [[ "$retcode" -gt 0 ]] && [[ "$passcnt" -lt 24 ]] ; do
        # Read temperature from the Sunfounder DHT11 sensor
        current_temp=$($read_temp)
        retcode=$?
        (( ++passcnt ))
    done

  return $retcode
}

# ===== function send_email
function send_email() {
    station=$(uname -n)
    subject="Temp check: $current_temp from $station: $(date)"
    mutt  -s "$subject" $SENDTO  < $emailbody
}

# ===== function pump_heater
function pump_heater() {
    bsendmail=false

    get_temp
    if [ "$?" -eq 0 ] ; then
        stat_str1="$(date "+%Y %m %d %T %Z"): Current temp: $current_temp, upper: $UPPER_TEMP, lower: $LOWER_TEMP loop cnt: $passcnt, gpio: $state_gpio"
        stat_str2="$(date "+%Y %m %d %T %Z"): Current temp: $current_temp, loop cnt: $passcnt, gpio: $state_gpio, state unchanged."
        dbgecho $stat_str1 | tee -a $PUMPHOUSE_LOGFILE

        # Test current temperature UPPER bound
        if (( current_temp > $UPPER_TEMP )) ; then
            if [ "$state_gpio" == "1" ] ; then
                stat_str2="$(date "+%Y %m %d %T %Z"): Temp: $current_temp, loop: $passcnt Changing state: on to OFF"
                echo $stat_str2 | tee -a $PUMPHOUSE_LOGFILE
            fi
            pump_heater_ctrl 0
            bsendmail=true

        # Test current temperature LOWER bound
        elif (( current_temp <= $LOWER_TEMP )) ; then
            if [ "$state_gpio" == "0" ] ; then
                stat_str2="$(date "+%Y %m %d %T %Z"): Temp: $current_temp, loop: $passcnt Changing state: off to ON"
                echo $stat_str2 | tee -a $PUMPHOUSE_LOGFILE
            fi
            pump_heater_ctrl 1
            bsendmail=true
        fi

        # Send some email when temperature gets below some value
        if (( current_temp < 34 )) ; then
            bsendmail=true
        else
            echo "Pumphouse temp: $current_temp"
        fi

        if $bsendmail ; then
            {
            echo "$stat_str1"
            echo "$stat_str2"
            } > $emailbody
            send_email
        fi
    fi
}

# ===== main
mode_gpio="$(gpio readall | grep -i "$wpi_num" | cut -d "|" -f $gpio_mode_cut | tr -d '[:space:]')"

if [ "$mode_gpio" != "OUT" ] ; then
   echo "$(date "+%Y %m %d %T %Z"): gpio $gpio_num is in wrong mode: |$mode_gpio|, should be: OUT" | tee -a $PUMPHOUSE_LOGFILE
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
   -d)
      get_temp
      state_gpio="$(gpio readall | grep -i "$wpi_num" | cut -d "|" -f $gpio_state_cut | tr -d '[:space:]')"
      state_str="off"
      if [ "$state_gpio" == "1" ] ; then
         state_str="on"
      fi

      echo "gpio state: $state_str, temp: $current_temp, upper temp: $UPPER_TEMP, lower temp: $LOWER_TEMP"
      ;;
   ?|h|H)
      usage
      ;;
   *)
      state_gpio="$(gpio readall | grep -i "$wpi_num" | cut -d "|" -f $gpio_state_cut | tr -d '[:space:]')"
#
      state_str="off"
      if [ "$state_gpio" == "1" ] ; then
         state_str="on"
      fi
      dbgecho "$(date "+%Y %m %d %T %Z"): gpio $gpio_num is $state_str" | tee -a $PUMPHOUSE_LOGFILE
      pump_heater
   ;;
esac

exit 0
