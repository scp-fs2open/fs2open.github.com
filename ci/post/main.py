import sys
import os
import re
from subprocess import check_output
from datetime import datetime
from itertools import groupby

import semantic_version

import file_list
import installer
import nebula
import forum


MAJOR_VERSION_PATTERN = re.compile("set_if_not_defined\(FSO_VERSION_MAJOR (\d+)\)")
MINOR_VERSION_PATTERN = re.compile("set_if_not_defined\(FSO_VERSION_MINOR (\d+)\)")
BUILD_VERSION_PATTERN = re.compile("set_if_not_defined\(FSO_VERSION_BUILD (\d+)\)")
LOG_FORMAT = """
------------------------------------------------------------------------%n
commit %h%nAuthor: %an <%ad>%n
Commit: %cn <%cd>%n%n    %s
"""
DATEFORMAT_VERSION = "%Y%m%d"
DATEFORMAT_FORUM = "%d %B %Y"


def _match_version_number(text, regex):
    match = regex.search(text)
    return int(match.group(1))


def get_source_version(date_version):
    with open(os.path.join("..", "..", "cmake", "version.cmake"), "r") as f:
        filetext = f.read()

        major = _match_version_number(filetext, MAJOR_VERSION_PATTERN)
        minor = _match_version_number(filetext, MINOR_VERSION_PATTERN)
        build = _match_version_number(filetext, BUILD_VERSION_PATTERN)

        return semantic_version.Version("{}.{}.{}-{}".format(major, minor, build, date_version))


def main():
	os.chdir(os.path.dirname(__file__))

	config = {
		"github": {
			"user": "scp-fs2open",
			"repo": "fs2open.github.com",
		},
		"ftp": {
			"host": "scp.indiegames.us",
			"user": os.environ["INDIEGAMES_USER"],
			"pass": os.environ["INDIEGAMES_SSHPASS"],
			"path": "public_html/builds/{type}/{version}/",
			"mirrors": (
				"https://porphyrion.feralhosting.com/datacorder/builds/{type}/{version}/{file}",
				"http://scp.indiegames.us/builds/{type}/{version}/{file}",
			),
		},
		"nebula": {
			"user": os.environ["NEBULA_USER"],
			"password": os.environ["NEBULA_PASSWORD"],
		},
		"hlp": {
			"api": os.environ["HLP_API"],
			"key": os.environ["HLP_KEY"],
		},
		"templates": {
			"nightly": "nightly.mako",
			"release": "release.mako",
		},
		"nightly": {
			"hlp_board": 173,
		},
		"release": {
			"hlp_board": 50,
		},
	}

	if sys.argv[1] not in ("release", "nightly"):
		print("ERROR: Invalid release mode %s passed. Expected release or nightly!" % sys.argv[1])
		sys.exit(1)

	tag_name = os.path.basename(os.environ["GITHUB_REF"])
	date = datetime.now()
	version = get_source_version(date.strftime(DATEFORMAT_VERSION))
	success = os.environ["LINUX_RESULT"] == "success" and os.environ["WINDOWS_RESULT"] == "success"

	tags = check_output(("git", "for-each-ref", "--sort=-taggerdate", "--format", "%(tag)", "refs/tags")).splitlines()
	previous_tag = None
	found = False

	for tag in tags:
		if found:
			if tag.startswith(sys.argv[1] + "_"):
				previous_tag = tag
				break
		elif tag == tag_name:
			found = True

	if not found:
		print("ERROR: Couldn't find tag %s in repo!" % tag_name)
		sys.exit(1)

	if not previous_tag:
		print("ERROR: Could not find a previous tag for %s!" % tag_name)
		sys.exit(1)

	if sys.argv[1] == "release":
		if not tag_name.startswith("release_"):
			print("ERROR: Invalid tag name detected. Expected release_ prefix for release builds but found %s (full ref = %s)" % tag_name, os.environ["GITHUB_REF"])
			sys.exit(1)

		files, sources = file_list.get_release_files(tag_name, config)

		print("Hashing files")
		for file in files:
			installer.get_hashed_file_list(file)

		# Construct the file groups
		groups = dict(((x[0], file_list.FileGroup(x[0], list(x[1]))) for x in groupby(files, lambda g: g.group)))

		print(installer.render_installer_config(version, groups, config))

		nebula.submit_release(
			nebula.render_nebula_release(version, "rc" if version.prerelease else "stable", "files", config),
			config)

		fapi = forum.ForumAPI(config)
		fapi.post_release(date.strftime(DATEFORMAT_FORUM), version, groups, sources)
	else:
		if not tag_name.startswith("nightly_"):
			print("ERROR: Invalid tag name detected. Expected nightly_ prefix for nightly builds but found %s (full ref = %s)" % tag_name, os.environ["GITHUB_REF"])
			sys.exit(1)

		print("Retrieving file list")
		files = file_list.get_ftp_files("nightly", tag_name, config)

		print("Hashing files")
		for file in files:
			installer.get_hashed_file_list(file)

		nebula.submit_release(nebula.render_nebula_release(version, "nightly", files, config), config)

		log = check_output(("git", "log", "%s^..%s^" % (previous_tag, tag_name), "--no-merges", "--stat", "--pretty=format:\"%s\"" % LOG_FORMAT.strip()))

		fapi = forum.ForumAPI(config)
		fapi.post_nightly(date.strftime(DATEFORMAT_FORUM), os.environ["GITHUB_SHA"], files, log, success)

main()
