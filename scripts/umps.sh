#!/usr/bin/env bash
# Purpose: Starts/stops/checks the status of an UMPS processes.
# Author: Ben Baker 

WORKDIR=`dirname $0`
DURATION=10
UMPS_BIN_DIR=/home/bbaker/Codes/umps/clang_build
EXIT_STATUS=0

# Define the executable
export PATH=${PATH}:${UMPS_BIN_DIR}
exe="${EXECUTABLE} --ini=${CONFIG_FILE}"

# Set the umask for any started programs
umask 022

# ps does not output beyond 80 columns which is problematic for ps commands
export COLUMNS=2048


# Description: Finds the process identifier on a Linux system using ps.
# Arguments: The command name to grep for.
function get_pid()
{
    local ps_grep_search=$1
    local pid=`ps aux | grep "${ps_grep_search}" | grep -v grep | awk '{print \$2}'`
    if [ ! -z "${pid}" ]; then
       echo ${pid}
    else
       echo ""
    fi
}

# Description: Stops the program.
# Arguments: The name of the program.
#            The command.
#            Optionally, the duration.
function stop_program()
{
    local name=$1
    local command=$2
    local duration=${3:-10}
    local pid=$(get_pid "${command}")
    local pid_save=${pid}
    EXIT_STATUS=0
    if [ -z "${pid}" ]; then
        echo "${name} not running" >&2
    else
        echo -n "Stopping ${NAME} (pid=${pid}) - " >&2
        echo "${pid}" | xargs kill

        #wait up to n seconds for stop
        pid=$(get_pid "${command}")
        seconds_waited=0
        while [ ! -z "${pid}" ] && [ ${seconds_waited} -lt ${duration} ]; do
            sleep 1
            echo -n "." >&2
            let seconds_waited=seconds_waited+1
            pid=$(get_pid "${command}")
        done

        if [ -z "${pid}" ]; then
            #not running
            echo "  TERMINATED ${name} which was running on pid=${pid_save}" >&2
            EXIT_STATUS=0
        else
            #still running
            echo "  ERROR: ${name} is still running on pid ${pid}" >&2
            EXIT_STATUS=3
        fi
    fi
    echo ${EXIT_STATUS}
}

function start_program()
{
    local name=$1
    local command=$2
    local workdir=$3
    local duration=${4-10}
    local pid=$(get_pid "${command}")
    EXIT_STATUS=0
    if [ ! -z "${pid}" ]; then
        echo "${name} already running (pid=${pid})" >&2
    else
        echo -n "Starting ${name} - " >&2
        # Run the command
        pushd ${workdir} > /dev/null
        nohup ${command} > /dev/null 2>&1 &
        popd > /dev/null 

        # Wait and see if this is running
        pid=$(get_pid "${command}")
        seconds_waited=0
        while [ -z "${pid}" ] && [ ${seconds_waited} -lt ${duration} ]; do
            sleep 1
            echo -n "." >&2
            let seconds_waited=seconds_waited+1
            pid=$(get_pid "${command}")
        done

        pid=$(get_pid "${command}")
        if [ ! -z "${pid}" ]; then
            echo "${name} is running on pid ${pid}" >&2
        else
            echo "  ERROR: Unable to start ${name} using command:" >&2
            echo "  ${command}" >&2
            EXIT_STATUS=3
        fi
    fi
    echo ${EXIT_STATUS}
}

#Initialization logic
function umps()
{
    local action=$1
    local executable=$2
    local args=${3-""}
    local name=${4-"Unknown"}
    local duration=${5-${DURATION}}
    local workdir=${6-${WORKDIR}}
    local exec_command="${executable} ${args}"
    EXIT_STATUS=0
    case "$ACTION" in
        start)
            EXIT_STATUS=$(start_program "${name}" "${exec_command}" "${workdir}" ${duration})
            ;;

        stop)
            EXIT_STATUS=$(stop_program "${name}" "${exec_command}" ${duration})
            ;;

        restart|reload|condrestart)
            EXIT_STATUS=$(stop_program "${name}" "${exec_command}" "${workdir}" ${duration})
            EXIT_STATUS=$(start_program "${name}" "${exec_command}" ${duration})
            ;;

        status)
            pid=$(get_pid "${exec_command}")

            if [ -z "${pid}" ]; then
                echo "${NAME} not running" >&2
                EXIT_STATUS=0
            else
                echo "${NAME} running on pid ${pid}" >&2
                EXIT_STATUS=0
            fi
            ;;

        *)
            echo "Usage: $0 {start|stop|restart|reload|condrestart|status}" >&2
            EXIT_STATUS=1
            ;;
    esac
    echo ${EXIT_STATUS}
}
