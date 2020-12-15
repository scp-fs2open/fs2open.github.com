function get_package_name() {
  if git describe --match 'nightly_*' --exact-match >/dev/null; then
    echo -n "$(git describe --match "nightly_*" --exact-match)"
    return
  fi

  echo "unknown_config"
}

function get_version_name() {
  if git describe --match 'nightly_*' --exact-match >/dev/null; then
    local tag_name=$(git describe --match "nightly_*" --exact-match)

    # Use the bash regex matching for getting the relevant part of the tag name
    [[ $tag_name =~ ^nightly_(.*)$ ]]

    echo -n "${BASH_REMATCH[1]}"
    return
  fi

  echo "unknown_config"
}

function upload_files_to_sftp() {
  echo "cd $2/" > sftp_batch
  echo "mkdir $(get_version_name)" >> sftp_batch
  # Create directory but do not cause an error if it already exists
  sshpass -e sftp -oBatchMode=no -o "StrictHostKeyChecking no" -b sftp_batch $1 || true

  echo "cd $2/$(get_version_name)/" > sftp_batch
  for file in {*.tar.gz, *.7z}; do
    echo "put $file" >> sftp_batch
  done

  echo "bye" >> sftp_batch

  sshpass -e sftp -oBatchMode=no -o "StrictHostKeyChecking no" -b sftp_batch $1

  rm sftp_batch
}

