source ${MY_DIR}/lib.inc
export AWS_CLI=${AWS_CLI:-aws}
export ROOT_BUCKET=s3://intimidating-cast
export APP=cast


function handle_default_args() {
  export USER_ID=${1?You must provide the user id}
  shift
}

export -f handle_default_args

