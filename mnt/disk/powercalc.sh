#! /bin/sh
#daemon startup script

PROG_NAME=powercalc

case "$1" in
start)
	echo "Starting ${PROG_NAME} daemon..."
	/disk/${PROG_NAME}
	;;
	
stop)
	echo "Stopping ${PROG_NAME} daemon..."
	kill `cat /var/run/${PROG_NAME}.pid`
	;;

restart)
	echo "Restarting ${PROG_NAME} daemon..."
	kill `cat /var/run/${PROG_NAME}.pid`
	sleep 5
	/disk/${PROG_NAME}
	;;
	
*)

	echo "Usage: ${PROG_NAME}.sh [start|stop|restart]"
	exit 1
esac

exit 0
