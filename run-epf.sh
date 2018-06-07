#!/bin/sh

# A shell script to run 'epf' utility for feeding Linux entropy pool with true random bytes downloaded from entropy service
# Last updated on Sat Jan 20 05:59:04 UTC 2018
# Configure with 'crontab -e' like the following:
# @reboot /usr/local/bin/run-epf.sh >> /var/log/epf/run-epf.log 2>&1

# Configuration file   
CFGFILE=/etc/epf/epf.properties

APPDIR=/usr/local/bin

# The application that feeds the Linux entropy pool
APPNAME=epf

APPCMD="$APPDIR/$APPNAME $CFGFILE"

if [ ! -e "$CFGFILE" ]; then
 echo "$CFGFILE configuration file could not be found"
 exit 1
fi

if [ ! -e "$APPDIR/$APPNAME" ]; then
 echo "$APPDIR/$APPNAME is not installed. Did you run 'make install' ?"
 exit 1
fi

if [ ! -x "$APPDIR/$APPNAME" ]
then
 echo "File '$APPDIR/$APPNAME' is not executable"
 exit 1
fi
CURRENTDATE=`date +"%Y-%m-%d %T"`

echo "$CURRENTDATE Start looping $APPCMD" 

while :
do
 $APPCMD
 sleep 15
done
