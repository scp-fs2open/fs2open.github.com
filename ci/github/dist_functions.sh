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
