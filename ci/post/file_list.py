import re
from ftplib import FTP, error_perm
from typing import List, Tuple, Dict

import requests

from util import retry_multi, GLOBAL_TIMEOUT

class ReleaseFile:
    def __init__(self, name, url, group, subgroup=None, mirrors=None):
        if mirrors is None:
            mirrors = []
        self.mirrors = mirrors
        self.subgroup = subgroup
        self.group = group
        self.url = url
        self.name = name

        self.base_url = "/".join(url.split('/')[0:-1]) + "/"
        self.filename = url.split('/')[-1]

        # A list of tuples of (filename, hash)
        self.content_hashes = None

        self.hash = None
        self.size = 0


class SourceFile:
    def __init__(self, name, url, group):
        self.group = group
        self.url = url
        self.name = name


def get_release_files(tag_name, config) -> Tuple[List[ReleaseFile], Dict[str, SourceFile]]:
    @retry_multi(5)
    def execute_request(path):
        headers = {
            "Accept": "application/vnd.github.v3+json"
        }
        url = "https://api.github.com" + path

        response = requests.get(url, headers=headers, timeout=GLOBAL_TIMEOUT)

        response.raise_for_status()

        return response.json()

    build_group_regex = re.compile("fs2_open_.*-builds-([^.-]*)(-([^.]*))?.*")
    source_file_regex = re.compile("fs2_open_.*-source-([^.]*)?.*")

    response = execute_request(
        "/repos/{}/{}/releases/tags/{}".format(config["github"]["user"], config["github"]["repo"], tag_name))

    binary_files = []
    source_files = {}
    for asset in response["assets"]:
        url = asset["browser_download_url"]
        name = asset["name"]

        group_match = build_group_regex.match(name)

        if group_match is not None:
            platform = group_match.group(1)
            # x64 is the Visual Studio name but for consistency we need Win64
            if platform == "x64":
                platform = "Win64"

            binary_files.append(ReleaseFile(name, url, platform, group_match.group(3)))
        else:
            group_match = source_file_regex.match(name)

            if group_match is None:
                continue

            group = group_match.group(1)

            source_files[group] = SourceFile(name, url, group)

    return binary_files, source_files


def get_ftp_files(build_type, tag_name, config):
    tag_regex = re.compile("nightly_(.*)")
    build_group_regex = re.compile("nightly_.*-builds-([^.]+).*")

    files = []
    try:
        with FTP(config["ftp"]["host"], config["ftp"]["user"], config["ftp"]["pass"]) as ftp:
            version_str = tag_regex.match(tag_name).group(1)

            path_template = config["ftp"]["path"]
            path = path_template.format(type=build_type, version=version_str)
            file_entries = list(ftp.mlsd(path, ["type"]))

            for entry in file_entries:
                if entry[1]["type"] == "file":
                    files.append(entry[0])
    except error_perm:
        print("Received permanent FTP error!")
        return []

    out_data = []
    for file in files:
        file_match = build_group_regex.match(file)
        if file_match is None:
            print("Ignoring non nightly file '{}'".format(file))
            continue

        group_match = file_match.group(1)
        primary_url = None
        mirrors = []

        # x64 is the name Visual Studio uses but Win64 works better for us since that gets displayed in the nightly post
        if "x64" in group_match:
            group_match = group_match.replace("x64", "Win64")

        for mirror in config["ftp"]["mirrors"]:
            download_url = mirror.format(type=build_type, version=version_str, file=file)
            if primary_url is None:
                primary_url = download_url
            else:
                mirrors.append(download_url)

        out_data.append(ReleaseFile(file, primary_url, group_match, None, mirrors))

    return out_data