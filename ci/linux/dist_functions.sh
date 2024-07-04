function get_package_name() {
  if git describe --match 'nightly_*' --exact-match 2>/dev/null >/dev/null; then
    echo -n "$(git describe --match "nightly_*" --exact-match)"
    return
  fi

  if git describe --match 'release_*' --exact-match 2>/dev/null >/dev/null; then
    echo -n "$(git describe --match "release_*" --exact-match | sed 's/release_/fs2_open_/i')"
    return
  fi

  if [[ "$(git branch --show-current)" =~ ^test\/(.*)$ ]]; then
    echo -n "test_${BASH_REMATCH[1]}"
    return
  fi

  echo "unknown_config"
}

function get_version_name() {
  if git describe --match 'nightly_*' --exact-match 2>/dev/null >/dev/null; then
    local tag_name=$(git describe --match "nightly_*" --exact-match)

    # Use the bash regex matching for getting the relevant part of the tag name
    [[ $tag_name =~ ^nightly_(.*)$ ]]

    echo -n "${BASH_REMATCH[1]}"
    return
  fi

  if git describe --match 'release_*' --exact-match 2>/dev/null >/dev/null; then
    local tag_name=$(git describe --match "release_*" --exact-match)

    # Use the bash regex matching for getting the relevant part of the tag name
    [[ $tag_name =~ ^release_(.*)$ ]]

    echo -n "${BASH_REMATCH[1]}"
    return
  fi

  if [[ "$(git branch --show-current)" =~ ^test\/(.*)$ ]]; then
    echo -n "${BASH_REMATCH[1]}"
    return
  fi

  echo "unknown_config"
}

function upload_files_to_sftp() {
  # We need empty glob patterns to "disappear" in the for loop below since this will be run for different configurations
  # where it may be possible that some of the file patterns will not be present
  shopt -s nullglob

  echo "cd $2/" > sftp_batch
  echo "mkdir $(get_version_name)" >> sftp_batch
  # Create directory but do not cause an error if it already exists
  sshpass -e sftp -oBatchMode=no -o "StrictHostKeyChecking no" -b sftp_batch $1 || true

  echo "cd $2/$(get_version_name)/" > sftp_batch
  for file in *.tar.gz *.7z *.zip; do
    echo "put $file" >> sftp_batch
  done

  echo "bye" >> sftp_batch

  sshpass -e sftp -oBatchMode=no -o "StrictHostKeyChecking no" -b sftp_batch $1

  rm sftp_batch
}

