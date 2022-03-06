# Post-build python script
# Uploads build packages to Nebula and creates a forum post on https://www.hard-light.net
# Uses the following environment variables (Set by the github action workflow before execution)

# FSO versioning
# FSO_VERSION_MAJOR
# FSO_VERSION_MINOR
# FSO_VERSION_BUILD

# Credentials into IndieGames file host
# INDIEGAMES_USER:
# INDIEGAMES_SSHPASS:

# Credentials into Nebula Mod Database
# NEBULA_USER:
# NEBULA_PASSWORD:

# Credentials into hard-light.net for forum post
# HLP_API:
# HLP_KEY:

# LINUX_RESULT: bool, if the linux builds were sucessfully uploaded
# WINDOWS_RESULT: bool, if the windows builds were successfully uploaded

import sys
import os
import re
from subprocess import check_output
from datetime import datetime
from itertools import groupby

import semantic_version

# local .py files
import file_list
import installer
import nebula
import forum

# Compile regexes for extracting version components
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
	"""! Extracts the version component represented by `regex` from `text`
	
	@param [in] `text`  Filename with version string 'MAJOR.MINOR.BUILD'
	@param [in] `regex` Regex pattern of component to look for

	@return The version component as integer
	"""
	match = regex.search(text)
	return int(match.group(1))
	

def get_source_version(date_version: datetime, tag_name: str) -> semantic_version.Version:
	"""! Retrieves the build's version from `version.cmake`, forms it as a string, and appends the date

	@param[in] `date_version` Date this build is published for release
	@param[in] `tag_name`     Tag of this release, used to determine if release, rc, or nightly

	@return If release: `MAJOR_VERSION.MINOR_VERSION.BUILD_VERSION` from version.cmake, or
	@return If rc:      `MAJOR_VERSION.MINOR_VERSION.BUILD_VERSION` from tag_name, or
	@return If nightly: `MAJOR_VERSION.MINOR_VERSION.BUILD_VERSION-date` from version.cmake
	"""
	major = minor = build = version = 0
	with open(os.path.join("..", "..", "cmake", "version.cmake"), "r") as f:
		filetext = f.read()
		major = _match_version_number(filetext, MAJOR_VERSION_PATTERN)
		minor = _match_version_number(filetext, MINOR_VERSION_PATTERN)
		build = _match_version_number(filetext, BUILD_VERSION_PATTERN)

	if "rc" in tag_name.lower():
		# Release candidates idenfity with their unstable version (ex: 21.5.0-03052022) within the game and the logs, but
		# the builds are named with the target release version (ex: 22.0.0).  Since this script works on the builds we
		# need to grab the target release version from tag_name
		x = tag_name.upper().split("_")
		# "release" = x[0], year = x[1], major = x[2], minor = x[3], build = x[4], 
		version = semantic_version.Version("{}.{}.{}-{}".format(x[1], x[2], x[3], x[4]))

	elif "release" in tag_name.lower():
		version = semantic_version.Version("{}.{}.{}".format(major, minor, build))

	elif "nightly" in tag_name.lower():
		version = semantic_version.Version("{}.{}.{}-{}".format(major, minor, build, date_version))
	
	else:	
		print("ERROR: malformed tag_name %s" % tag_name)
		sys.exit(1)

	print("version: {}".format(version))
	print("version.prerelease: {}".format(version.prerelease))
	return version


def main():
	os.chdir(os.path.dirname(__file__))	# Change working directory to this file's directory

	# Aggregate configuration data in a dictionary
	config = {
		"github": {
			"repo": os.environ["GITHUB_REPO"],
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

	# bail if this script is run from a workflow that doesn't identify as a release or nightly
	if sys.argv[1] not in ("release", "nightly"):
		print("ERROR: Invalid release mode %s passed. Expected release or nightly!" % sys.argv[1])
		sys.exit(1)

	tag_name = os.environ["RELEASE_TAG"]	##!< commit tag string
	date = datetime.now()	##!< current date
	version = get_source_version(date.strftime(DATEFORMAT_VERSION), tag_name)	##!< form full version string
	success = os.environ["LINUX_RESULT"] == "success" and os.environ["WINDOWS_RESULT"] == "success"	##!< true if both linux and windows builds successful

	# check that tag_name is actually in the git repo and find the previous tag release
	tags = check_output(("git", "for-each-ref", "--sort=-taggerdate", "--format='%(tag)'", "refs/tags"), text=True).splitlines()	# retrieve all tags in the repo by using git on the shell
	# NOTE: check_output returns canonical string representation. use repr() on strings its being compared to
	previous_tag = None
	found = False

	for tag in tags:
		if found:
			# Look for the "previous" tag with the same configuration ('release_' or 'nightly_')
			# Assumes older tags have a higher index than newer
			if tag.startswith("\'" + sys.argv[1] + "_"):
				previous_tag = tag
				break
		elif tag == repr(tag_name):
			# Ok, found the tag
			found = True

	if not found:
		# couldn't find tag, bail
		print("ERROR: Couldn't find tag %s in repo!" % tag_name)
		sys.exit(1)

	if not previous_tag:
		# couldn't find previous tag, bail
		print("ERROR: Could not find a previous tag for %s!" % tag_name)
		sys.exit(1)

	if sys.argv[1] == "release":
		# Release specific action
		if not tag_name.startswith("release_"):
			# Safety check tag is from a release
			print("ERROR: Invalid tag name detected. Expected release_ prefix for release builds but found %s (full ref = %s)" % tag_name, os.environ["GITHUB_REF"])
			sys.exit(1)

		files, sources = file_list.get_release_files(tag_name, config)

		print("Hashing files")
		for file in files:
			installer.get_hashed_file_list(file)

		# Construct the file groups
		# split the file list into a dictionary of {group: files} pairs using `groupby()`
		# Since `groupby()` returns a iterator reference to the original files list, we have to do a weird hand toss
		# to save it as a dictionary
		## NOTE z64: files need to be sorted or else key duplication will occur!
		groups = {key: file_list.FileGroup(key, list(group)) for key, group in groupby(files, lambda g: g.group)}
		
		# Error check for all group keys and subkeys to be used in the mako.  Do this here for better debugging QoL
		if "Win32" in groups.keys():
			if "AVX" not in groups["Win32"].subFiles.keys():
				print("ERROR: No Win32-AVX builds were detected!")
				sys.exit(1)
		else:
			print("ERROR: No Win32 builds were detected!")
			sys.exit(1)

		if "Win64" in groups.keys():
			if "AVX" not in groups["Win64"].subFiles.keys():
				print("ERROR: No x64-AVX builds were detected!")
				sys.exit(1)
		else:
			print("ERROR: No x64 builds were detected!")
			sys.exit(1)
		
		if "Linux" not in groups.keys():
			print("ERROR: No Linux builds were detected!")
			sys.exit(1)
		
		# z64: What dose this do???
		# print(installer.render_installer_config(version, groups, config))

		# Publish release builds to Nebula
		nebula.submit_release(
			nebula.render_nebula_release(version, "rc" if version.prerelease else "stable", files, config),
			config)

		# Publish forum post, using the release.mako template
		fapi = forum.ForumAPI(config)
		fapi.post_release(date.strftime(DATEFORMAT_FORUM), version, groups, sources)

	else:
		# Dead for now, nightly publish is done elsewhere
		# Nightly specific action
		if not tag_name.startswith("nightly_"):
			# safety check tag is from a nightly
			print("ERROR: Invalid tag name detected. Expected nightly_ prefix for nightly builds but found %s (full ref = %s)" % tag_name, os.environ["GITHUB_REF"])
			sys.exit(1)

		print("Retrieving file list")
		files = file_list.get_ftp_files("nightly", tag_name, config)

		print("Hashing files")
		for file in files:
			installer.get_hashed_file_list(file)

		# Publish nightly builds to Nebula
		nebula.submit_release(
			nebula.render_nebula_release(version, "nightly", files, config),
			config)

		# Compile commit messages into a log with all commits between this nightly and the previous nightly
		log = check_output(("git", "log", "%s^..%s^" % (previous_tag, tag_name), "--no-merges", "--stat", "--pretty=format:\"%s\"" % LOG_FORMAT.strip()))

		# Publish forum post, using the nighty.mako template
		fapi = forum.ForumAPI(config)
		fapi.post_nightly(date.strftime(DATEFORMAT_FORUM), os.environ["GITHUB_SHA"], files, log, success)

main()
