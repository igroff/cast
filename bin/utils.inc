source ${MY_DIR}/lib.inc
export AWS_CLI=${CAST_AWS_CLI:-aws}
export STORAGE_ROOT=${CAST_STORAGE_ROOT:-"s3://intimidating-cast"}
export SINGLE_USER=true
export APP=cast


function handle_default_args() {
  export USER_ID=${1?You must provide the user id}
  shift
}

export -f handle_default_args

#! /usr/bin/env bash
# vi: ft=sh
AWS=${AWS_CLI:-aws}
STORAGE_ROOT="${STORAGE_ROOT:-root_bucket}"
function create_storage_path() {
  if [ -z "${SINGLE_USER}" ]; then
    USER_ID="${1?You must provide a user id}"
    THE_PATH="${2?You must provide a path to a file}"
  else
    THE_PATH="${1?You must provide a path to a file}"
  fi
  # strip the / at the start of the path, if it's there
  THE_PATH="${THE_PATH#/}"
  if [ -z "${SINGLE_USER}" ]; then
    printf "%s/%s%s\n" "${USER_ID}" "$(date +"%Y/%m/%d/%H_")" "$(basename "${THE_PATH}")"
  else
    printf "%s%s\n" "$(date +"%Y/%m/%d/%H_")" "$(basename "${THE_PATH}")"
  fi
}
function create_daily_storage_path() {
  if [ -z "${SINGLE_USER}" ]; then
    USER_ID="${1?You must provide a user id}"
    THE_PATH="${2?You must provide a path to a file}"
  else
    THE_PATH="${1?You must provide a path to a file}"
  fi
  THE_PATH="${THE_PATH#/}"
  if [ -z "${SINGLE_USER}" ]; then
    printf "%s/%s%s\n" "${USER_ID}" "$(date +"%Y/%m/%d/")" "$(basename "${THE_PATH}")"
  else
    printf "%s%s\n" "$(date +"%Y/%m/%d/")" "$(basename "${THE_PATH}")"
  fi
}

function open_daily_note(){
  if [ -z "${SINGLE_USER}" ]; then
    USER_ID="${1?You must provide a user id}"
    NOTE_NAME="${2?You must provide a name for the daily note}"
    THE_PATH="$(create_daily_storage_path "${USER_ID}" "${NOTE_NAME}")"
  else
    NOTE_NAME="${1?You must provide a name for the daily note}"
    THE_PATH="$(create_daily_storage_path "${NOTE_NAME}")"
  fi
  if [ "${STORAGE_ROOT:0:5}" = "s3://" ]; then
    printf "I don't know how to do notes with s3 storage locations"
    exit 1
  else
    mkdir -p $(dirname "${STORAGE_ROOT}/${THE_PATH}")
    ${EDITOR:-vim} "${STORAGE_ROOT}/${THE_PATH}.markdown"
  fi
}

function create_metadata_storage_path() {
  if [ -z "${SINGLE_USER}" ]; then
    USER_ID="${1?You must provide a user id}"
    THE_PATH="${2?You must provide a path to a file}"
    THE_PATH="$(create_storage_path "${USER_ID}" "${THE_PATH}")"
  else
    THE_PATH="${1?You must provide a path to a file}"
    THE_PATH="$(create_storage_path "${THE_PATH}")"
  fi
  printf "%s.mjson\n" "${THE_PATH}"
}

function store_file() {
  if [ -z "${SINGLE_USER}" ]; then
    USER_ID="${1?You must provide a user id}"
    THE_PATH="${2?You must provide a path to a file}"
    STORAGE_PATH="$(create_storage_path "${USER_ID}" "${THE_PATH}")"
    METADATA_STORAGE_PATH="$(create_metadata_storage_path "${USER_ID}" "${THE_PATH}")"
  else
    THE_PATH="${1?You must provide a path to a file}"
    STORAGE_PATH="$(create_storage_path "${THE_PATH}")"
    METADATA_STORAGE_PATH="$(create_metadata_storage_path "${THE_PATH}")"
  fi
  if [ "${STORAGE_ROOT:0:5}" = "s3://" ]; then
    echo "aws s3 cp '${THE_PATH}' '${STORAGE_ROOT}/${STORAGE_PATH}'"
    echo "aws s3 cp '${THE_PATH}.mjson' '${STORAGE_ROOT}/${METADATA_STORAGE_PATH}'"
  else
    echo "cp '${THE_PATH}' '${STORAGE_ROOT}/${STORAGE_PATH}'"
    echo "cp '${THE_PATH}.mjson' '${STORAGE_ROOT}/${METADATA_STORAGE_PATH}'"
  fi
}

export -f create_storage_path
export -f create_daily_storage_path
export -f open_daily_note
export -f create_metadata_storage_path
export -f store_file
