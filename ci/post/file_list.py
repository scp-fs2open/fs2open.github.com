import re # regex module
from ftplib import FTP, error_perm
from itertools import groupby
from typing import List, Tuple, Dict

import requests  # HTTP requests module

from util import retry_multi, GLOBAL_TIMEOUT	# from util.py


class ReleaseFile:
    """! Class representing a Released file on Nebula

    `name`: str
        Mod (or build) name,
    `url`: str
        Primary host URL,
    `group`: str
        Mod group string,
    `subgroup`: str
        Mod subgroup string,
    `mirrors`: List[str]
        List of URL's of FTP mirrors
    """
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
    """! Class represeting a source file

    `name`: str
        File name,
    `url`: str
        FTP URL,
    `group`
        <unknown>
 
    @details More details
    """
    def __init__(self, name, url, group):
        self.group = group
        self.url = url
        self.name = name


class FileGroup:
    """! Represents a file group

    `name`: str
        Name of this group
    `files`: List[ReleaseFile]
        List of files within this group
    `mainFile`: str
        If this FileGroup has a subgroup, `mainFile` is the head of that group
    `subFiles`: List[ReleaseFile]
        Files within a subgroup
    """

    def __init__(self, name, files: List[ReleaseFile]):
        self.files = files
        self.name = name

        if len(files) == 1:
            self.mainFile = files[0]
            self.subFiles = {}
        else:
            self.mainFile = None
            subFiles = []
            for file in files:
                # We only have subcategories for Windows where SSE2 is the main group
                if file.subgroup == "SSE2":
                    self.mainFile = file
                else:
                    subFiles.append(file)

            self.subFiles = dict(((x[0], next(x[1])) for x in groupby(subFiles, lambda f: f.subgroup)))


def get_release_files(tag_name, config) -> Tuple[List[ReleaseFile], Dict[str, SourceFile]]:
    """! Brief Gets the binary and source files from the Github Release server

    @param[in] `tag_name` Git tag of the current release
    @param[in] `config`   confi metadata set in main.py

    @returns `List[ReleaseFile]`        List of release files
    @returns `Dict[str, SourceFile]`    Dictionary of source files

    @details Sends an `HTTP GET` request to github using their REST API to retrieve metadata.  The files are not
        actually downloaded here, just their metadata is gathered and organized in their respective container for later
        use.
    """

    @retry_multi(5)	# retry at most 5 times
    def execute_request(path):
        """!
        @brief Performs a GET request with the given path. To be used with Github's REST API.
        @returns If successful, returns a .JSON object
        """
        headers = {
            "Accept": "application/vnd.github.v3+json"
        }
        url = "https://api.github.com" + path

        # GET https://api.github.com/<path> Accept: "application/vnd.github.v3+json"

        response = requests.get(url, headers=headers, timeout=GLOBAL_TIMEOUT)

        response.raise_for_status() # Raise a RequestException if we failed, and trigger retry

        return response.json()

    build_group_regex = re.compile("fs2_open_.*-builds-([^.-]*)(-([^.]*))?.*")  # regex for matching binary .zip's and .7z's
    source_file_regex = re.compile("fs2_open_.*-source-([^.]*)?.*") # regex for matching source .zip's and .7z's

    # Get the github release metadata of the given tag name
    response = execute_request(
        "/repos/{}/{}/releases/tags/{}".format(config["github"]["user"], config["github"]["repo"], tag_name))

    # Extract the binary and source files from the response["asset"] metadata
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


def get_ftp_files(build_type, tag_name, config) -> List[ReleaseFile] :
    """!
    @brief Gets file metadata for nightlies hosted on FTP, as determined by config["ftp"] attributes
    
    @param [in] `build_type` Unknown str
    @param [in] `tag_name`   Github tag name of the release
    @param [in] `config`     config metadata set in main.py
    """

    tag_regex = re.compile("nightly_(.*)")
    build_group_regex = re.compile("nightly_.*-builds-([^.]+).*")

    files = []
    try:
        with FTP(config["ftp"]["host"], config["ftp"]["user"], config["ftp"]["pass"]) as ftp:
            # extract version
            version_str = tag_regex.match(tag_name).group(1)

            # extract filepath w/ version
            # then list all ftp hits with that path
            path_template = config["ftp"]["path"]
            path = path_template.format(type=build_type, version=version_str)
            file_entries = list(ftp.mlsd(path, ["type"]))

            # get all ftp hits of type file
            for entry in file_entries:
                if entry[1]["type"] == "file":
                    files.append(entry[0])
    except error_perm:
        print("Received permanent FTP error!")
        return []

    out_data = []
    for file in files:
        # from the file list, extract only nightly files
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

        # construct the download URL list for all mirrors.  The first listed ftp location is taken as the Primary
        for mirror in config["ftp"]["mirrors"]:
            download_url = mirror.format(type=build_type, version=version_str, file=file)
            if primary_url is None:
                primary_url = download_url
            else:
                mirrors.append(download_url)

        # Form the List[ReleaseFile] list with the download URL links
        out_data.append(ReleaseFile(file, primary_url, group_match, None, mirrors))

    return out_data