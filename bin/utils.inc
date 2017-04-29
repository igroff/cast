source ${MY_DIR}/lib.inc
export AWS_CLI=${CAST_AWS_CLI:-aws}
export S3_STORAGE_LOCATION=${CAST_S3_STORAGE_LOCATION:-"s3://cast.io"}
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
  local NOTE_FILE_PATH="${THE_PATH}.md"
  local LOCAL_PATH="${WORKING_DIR}/${NOTE_FILE_PATH}"
  local REMOTE_PATH="${S3_STORAGE_LOCATION}/${NOTE_FILE_PATH}"
  if [ -f "${LOCAL_PATH}" ]; then
    ${AWS_CLI} s3api head-object --bucket "${S3_STORAGE_LOCATION}" --key "${NOTE_FILE_PATH}"
    # we have a local copy, so we want to make sure it matches the remote
    # if it doesn't match the remote, the remote wins so we fetch it over
    # the local
  else
    mkdir -p $(dirname "${LOCAL_PATH}")
    # we don't already have the file locally, so we fetch it
    if COPY_RESULT=$(${AWS_CLI} s3 cp "${REMOTE_PATH}" "${LOCAL_PATH}" 2>&1); then
      # the copy worked, all is good
      :
    else
      if echo "${COPY_RESULT}" | grep 404 | grep 'does not exist' > /dev/null 2>&1; then
        # it's not remote either, we'll need to create it
        touch "${LOCAL_PATH}"
        echo "we don't have it local, we'll need to create it"
        :
      else
        # something really went wronte
        error "unable to fetch daily note to ${LOCAL_PATH}"
        error "${COPY_RESULT}"
      fi
    fi
  fi
  
  debug "local cached: ${LOCAL_PATH}"
  debug "remote location: ${REMOTE_PATH}"
  #mkdir -p $(dirname "${S3_STORAGE_LOCATION}/${THE_PATH}")
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
