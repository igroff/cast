#! /usr/bin/env bash
# vi: ft=sh
AWS=${AWS_CLI:-aws}
ROOT_BUCKET="${ROOT_BUCKET:-root_bucket}"
function create_storage_path() {
  USER_ID="${1?You must provide a user id}"
  THE_PATH="${2?You must provide a path to a file}"
  # strip the / at the start of the path, if it's there
  THE_PATH="${THE_PATH#/}"
  printf "%s/%s%s\n" "${USER_ID}" "$(date +"%Y/%m/%d/%H_")" "$(basename "${THE_PATH}")"
}

function create_metadata_storage_path() {
  THE_PATH="$(create_storage_path "${USER_ID}" "${THE_PATH}")"
  printf "%s.mjson\n" "${THE_PATH}"
}

function store_file() {
  USER_ID="${1?You must provide a user id}"
  FILE_TO_STORE="${2?You must provide a path to a file}"
  STORAGE_PATH="$(create_storage_path "${USER_ID}" "${FILE_TO_STORE}")"
  METADATA_STORAGE_PATH="$(create_metadata_storage_path "${USER_ID}" "${FILE_TO_STORE}")"
  echo "aws s3 cp '${FILE_TO_STORE}' 's3://${ROOT_BUCKET}/${STORAGE_PATH}'"
  echo "aws s3 cp 'created_metadata_file.mjson' 's3://${ROOT_BUCKET}/${METADATA_STORAGE_PATH}'"
}

export -f create_storage_path
export -f create_metadata_storage_path
export -f store_file

create_storage_path "${1?need userid}" "${2? need path}"
create_metadata_storage_path "${1?need userid}" "${2? need path}"

create_storage_path "chuck" "thing.txt"
create_metadata_storage_path "chuck" "thing.txt"

store_file "igroff" "/var/log/syslog"
