source ${MY_DIR}/lib.inc
export AWS_CLI=${CAST_AWS_CLI:-aws}
export BUCKET_NAME="cast.io"
export WORKING_DIR=${CAST_WORKING_DIR:-~/.cast}
export SINGLE_USER=true
export APP=cast


function user_path_args() {
  if [ -z "${SINGLE_USER}" ]; then
    USER_ID="${1?You must provide a user id}"
    shift
    THE_PATH="${1?You must provide a path to a file}"
    THE_PATH="$(create_storage_path "${USER_ID}" "${THE_PATH}")"
    return 1
  else
    THE_PATH="${1?You must provide a path to a file}"
    shift
    THE_PATH="$(create_storage_path "${THE_PATH}")"
    return 0
  fi
}

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
function create_daily_storage_path_suffix() {
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
  local THE_PATH=$(create_daily_storage_path_suffix "$@")
  local LOCAL_PATH="${WORKING_DIR}/${THE_PATH}.md"
  mkdir -p $(dirname "${LOCAL_PATH}")
  ${EDITOR:-vim} "${LOCAL_PATH}"
}

function create_metadata_storage_path() {
  user_path_args "$@"
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
  mkdir -p $(dirname "${WORKING_DIR}/${STORAGE_PATH}")
  echo "cp '${THE_PATH}' '${WORKING_DIR}/${STORAGE_PATH}'"
  echo "cp '${THE_PATH}.mjson' '${WORKING_DIR}/${METADATA_STORAGE_PATH}'"
}

export -f create_storage_path
export -f create_daily_storage_path_suffix
export -f open_daily_note
export -f create_metadata_storage_path
export -f store_file
export -f user_path_args
