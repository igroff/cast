# vim: ft=sh
MY_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# super brute-force determination if we're running on OS X
# if replacing darwin in OSTYPE with pants changes the value
# of OSTYPE, it's a mac
IS_MAC=false
if [ "${OSTYPE/darwin/pants}" != "${OSTYPE}" ]; then
  IS_MAC=true
fi
export IS_MAC

silent_pushd(){ pushd $1 > /dev/null 2>&1; }
export -f silent_pushd

silent_popd(){ popd $1 > /dev/null 2>&1 ; }
export -f silent_popd

write_log(){ 
  LEVEL=${1}
  shift
  if [ -n "${BARE_LOG}" ]; then
    echo "$@" >&2;
  elif [ -n "${LEVEL}" ]; then
    echo "`date +"%x %X"` [${LEVEL}] $@" >&2;
  else
    echo "`date +"%x %X"` $@" >&2;
  fi
}
export -f write_log

hash_file(){
  THIS_FILE=$1
  openssl md5 ${THIS_FILE} | sed -e 's[MD5.* [[g'
}
export -f hash_file

log_divider() { write_log "******************************************"; }
export -f log_divider

error() { write_log ${FUNCNAME^^} "$@"; }
export -f error

warn()  { write_log ${FUNCNAME^^} "$@"; }
export -f warn

info()  { write_log ${FUNCNAME^^} "$@"; }
export -f info

debug() { [ -n "${DEBUG}" ] && write_log ${FUNCNAME^^} "$@"; }
export -f debug

die() { echo $1 ; exit ${2-1} ;}
export -f die

abs_path() {
  (cd "${1%/*}" &>/dev/null && printf "%s/%s" "$(pwd)" "${1##*/}")
}
export -f abs_path

make_temp_file(){
  TEMPLATE=${1?You must provide a template name}
  if [ "${IS_MAC}" == "true" ]; then
    mktemp -t ${TEMPLATE}
  else
    mktemp --tmpdir ${TEMPLATE}
  fi
}
export -f make_temp_file

function abspath {
    if [[ -d "$1" ]]; then
        pushd "$1" >/dev/null
        pwd
        popd >/dev/null
    elif [[ -e $1 ]]; then
        pushd "$(dirname "$1")" >/dev/null
        echo "$(pwd)/$(basename "$1")"
        popd >/dev/null
    else
        echo "$1" does not exist! >&2
        return 127
    fi
}
export -f abspath

show_help(){
  FROM_THIS_FILE=$1
  debug "Showing help from ${FROM_THIS_FILE}"
  FILE_ONLY=$(basename $(abspath ${FROM_THIS_FILE}))
  grep '^###.*' ${FROM_THIS_FILE} | sed -e s[#*[[g \
    | sed -e "s[\${ME}[$FILE_ONLY[g"
}
export -f show_help

show_usage(){
  FROM_THIS_FILE=$1
  debug "Showing help from ${FROM_THIS_FILE}"
  grep '^####.*' ${FROM_THIS_FILE} | sed -e s[#*[[g \
    | sed -e "s[\${ME}[$FROM_THIS_FILE[g"
}
export -f show_usage

process_command(){
  local FOR_THIS_FILE=$1
  debug "processing commands for ${FOR_THIS_FILE}"
  # our default command will be based on the script we're operating on
  local COMMAND_PATH=${FOR_THIS_FILE}
  local COMMAND_DIR=`dirname ${FOR_THIS_FILE}`
  local BASE_SCRIPT_NAME=`basename ${FOR_THIS_FILE}`
  shift # away our root script name
  # keep track of our root command 'name' 
  local COMMAND_NAME=${1}
  local WANT_HELP

  # help is stored in comand name, and no longer needed
  [ "${COMMAND_NAME}" == "help" ] && WANT_HELP="t" && shift

  debug do they want help? ${WANT_HELP}

  for part in $@
  do
    # if we don't already have a path to test, we'll use the original
    local TEST_COMMAND=${TEST_COMMAND:-${COMMAND_PATH}}_${part}
    debug "checking for command at ${TEST_COMMAND}"
    # if we find an executable at the path, that's our new command
    if [ -x "${TEST_COMMAND}" ]; then
      debug "found executable command: ${TEST_COMMAND}"
      COMMAND_PATH=${TEST_COMMAND}
    fi
  done
  COMMAND_NAME=`basename ${COMMAND_PATH}`
  # find the numer of underscores in the command name, we'll remove this
  # many arguments to get to the actual command arguments
  # this will include the EOL character, so we'll reduce our count by one
  COMMAND_PART_COUNT=$(echo ${COMMAND_NAME} | sed -e 's/[^_]//g' | wc -m)
  COMMAND_PART_COUNT=$(echo "${COMMAND_PART_COUNT} - 1" | bc)
  shift ${COMMAND_PART_COUNT}
  debug "found command at: ${COMMAND_PATH}"
  # if we have not found a different command path than what we started with we're
  # going to use the default
  [ "${COMMAND_PATH}" == "${FOR_THIS_FILE}" ] && COMMAND_PATH=${COMMAND_PATH}_default
  # we found a command, are they asking for help?
  if [ -n "${WANT_HELP}" ]; then
    debug "someone has asked for help"
    # if we've asked for help on the default command
    if [ "${COMMAND_PATH}" == "${FOR_THIS_FILE}_default" ]; then
      # and we have a help specific command
      if [ -x "${FOR_THIS_FILE}_help" ]; then
         ${FOR_THIS_FILE}_help "$@"
      fi
    else
      show_help "${COMMAND_PATH}"
    fi
    exit -1
  fi
  debug "running command ${COMMAND_PATH}"
  # if not help, we execute
  ${COMMAND_PATH} "$@"
  exit $?
}
export -f process_command

bail_on_error(){
  ERROR_CODE=$?
  MESSAGE=$1
  if [ 0 -ne ${ERROR_CODE} ]; then
    if [ -n "${MESSAGE}" ]; then
      echo "Error (${ERROR_CODE}): ${MESSAGE}"
    fi
    exit ${ERROR_CODE}
  fi
}
export -f bail_on_error

source_rc_file(){
  APP_NAME=$1
  RC_FILE=$(
  until [ `pwd` == "/" ]
  do
    [ -f ".${APP_NAME}rc" ] && echo `pwd`/.${APP_NAME}rc && break
    cd .. > /dev/null 2>&1
  done
  )
  [ -f "${RC_FILE}" ] && export RC_FILE && source ${RC_FILE} && return 0

  RC_FILE=~/.${APP_NAME}rc
  [ -f "${RC_FILE}" ] && export RC_FILE && source ${RC_FILE} && return 0
  RC_FILE=~/.${APP_NAME}/config
  [ -f "${RC_FILE}" ] && export RC_FILE && source ${RC_FILE} && return 0
  return 100
}
export -f source_rc_file

function hash_with_shasum() {
  if which shasum > /dev/null 2>&1 ; then
    function hasher() {
      shasum "${1}" | sed -e 's[  .*[[g'
    }
    return 0
  fi
  return 1
}

function hash_with_openssl() {
  if which openssl > /dev/null 2>&1 ; then
    function hasher() {
      openssl sha "${1}" | sed -e 's[SHA\(.*\)= [[g'
    }
    return 0
  fi
  return 1
}

function hash_with_md5(){
  if which md5sum > /dev/null 2>&1 ; then
    function hasher() {
      md5sum "${1}" | sed -e 's[  .*[[g'
    }
    return 0
  fi
  return 1
}

function find_a_hasher() {
  case "${1}" in
    "md5")
      hash_with_md5
      ;;
    "shasum")
      hash_with_shasum
      ;;
    "openssl")
      hash_with_openssl
      ;;
    *)
      if hash_with_openssl || hash_with_shasum || hash_with_md5 ; then
        return 0
      else
        return 1
      fi
      ;;
  esac
}
export -f find_a_hasher

function hash_with_sha() {
  openssl sha "${1}" | sed -e 's[SHA\(.*\)= [[g'
}
export -f hash_with_sha

export UTILS_IMPORTED=1
