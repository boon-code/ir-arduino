#!/bin/bash

NORMAL_DEV=/dev/ttyIRUSB
BOOT_DEV=/dev/ttyARDUINO
DEFAULT_BAUDRATE=19200

wait_for_dev()
{
	while [ ! -e "$1" ]; do
		sleep 0.3
	done
}

do_enter_bl() {
	echo "[program]: Find device node ${NORMAL_DEV} ..."
	wait_for_dev "${NORMAL_DEV}"

	echo "[program]: Enter monitor mode ..."
	stty -F ${NORMAL_DEV} 1200
	if [ $? -ne 0 ]; then
		echo "[program]: Failed to enter monitor mode on ${NORMAL_DEV}"
		exit 1
	fi

	echo -n "[program]: Try to enter bootloader "

	while [ -e "${NORMAL_DEV}" ]; do
		echo -n "."
		echo "b" > ${NORMAL_DEV} 2>/dev/null
		sleep 0.5
	done

	echo ""
}

program() {
	echo "[program]: Wait for bootloader ${BOOT_DEV} ..."
	sleep 1
	wait_for_dev "${BOOT_DEV}"

	echo "[program]: Flash program to device ..."
	make avrdude
	if [ $? -ne 0 ]; then
		echo "[program]: Programming device ${BOOT_DEV} failed!"
		exit 1
	fi
}

wait_normal() {
	echo "[program]: Wait for device node ${NORMAL_DEV} ..."
	sleep 1
	wait_for_dev "${NORMAL_DEV}"

	echo "[program]: Set default baud-rate ${DEFAULT_BAUDRATE} ..."
	stty -F ${NORMAL_DEV} ${DEFAULT_BAUDRATE}
	if [ $? -ne 0 ]; then
		echo "[program]: Failed to set default baud-rate on ${NORMAL_DEV}"
		# no error
	fi

	echo "[program]: Finished programming!"
}

[ -z "${NOENTER}" ] && do_enter_bl
program
[ -z "${NOENTER}" ] && wait_normal

exit 0
