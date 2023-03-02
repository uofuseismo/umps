#!/usr/bin/env bash
# Purpose: Starts/stops/checks status of the uOperator service.
# Author: Ben Baker 
source umps.sh

# Workspace directory
WORKDIR=`dirname $0`
# Config file
CONFIG_FILE=$(pwd)/operator.ini
# Name of executable
EXECUTABLE=uOperator
# Seconds to wait to stop/start a program
DURATION=10
# Name displayed by the init script
NAME="uOperator Connection Information and Messaging Service"

# Define the executable
ARGS="--ini=${CONFIG_FILE}"

ACTION=$1
EXIT_STATUS=$(umps "${ACTION}" "${EXECUTABLE}" "${ARGS}" "${NAME}" "${DURATION}" ${WORKDIR})
if [ ${EXIT_SUCCESS} ]; then
   echo "Errors detected: ${EXIT_STATUS}"
fi
exit ${EXIT_STATUS}
