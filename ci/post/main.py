# Post-build python script
# Uploads build packages to Nebula
# Uses the following environment variables (Set by the github action workflow before execution)

# FSO versioning
# FSO_VERSION_MAJOR
# FSO_VERSION_MINOR
# FSO_VERSION_BUILD

# Credentials into Nebula Mod Database
# NEBULA_USER:
# NEBULA_PASSWORD:
# NEBULA_PRIVATE: "false" publishes publicly; anything else (or unset) uploads privately

# LINUX_RESULT: bool, if the linux builds were successfully uploaded
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

# Compile regexes for extracting version components
MAJOR_VERSION_PATTERN = re.compile("(?:set_if_not_defined|set)\(FSO_VERSION_MAJOR (\d+)\)")
MINOR_VERSION_PATTERN = re.compile("(?:set_if_not_defined|set)\(FSO_VERSION_MINOR (\d+)\)")
BUILD_VERSION_PATTERN = re.compile("(?:set_if_not_defined|set)\(FSO_VERSION_BUILD (\d+)\)")
REVISION_VERSION_PATTERN = re.compile("(?:set_if_not_defined|set)\(FSO_VERSION_REVISION (\d+)\)")
REVISION_STR_VERSION_PATTERN = re.compile("(?:set_if_not_defined|set)\(FSO_VERSION_REVISION_STR (\w*)\)")

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

def _match_version_str(text, regex):
	"""! Extracts the version component represented by `regex` from `text`
	
	@param [in] `text`  Filename with version string 'MAJOR.MINOR.BUILD'
	@param [in] `regex` Regex pattern of component to look for

	@return The version component as integer
	"""
	match = regex.search(text)
	return match.group(1)
	

def get_source_version(date_version: datetime, tag_name: str) -> semantic_version.Version:
	"""! Derives the release version from the git tag.

	@param[in] `date_version` Unused; kept for signature compatibility.
	@param[in] `tag_name`     Release tag, e.g. release_26_1_0-20260713

	@return The version parsed from the tag: release_MAJOR_MINOR_BUILD[-SUFFIX] becomes
	  MAJOR.MINOR.BUILD[-SUFFIX] (e.g. release_26_1_0-20260713 -> 26.1.0-20260713).

	@note OFP derives the Nebula version from the tag rather than from a generated
	  version_override.cmake (which OFP's manual tagging does not produce).
	"""
	m = re.match(r"^(?:release|nightly)_(.+)$", tag_name)
	if not m:
		print("  ERROR: malformed tag_name %s" % tag_name)
		sys.exit(1)

	numeric, _, suffix = m.group(1).partition("-")
	ver = numeric.replace("_", ".")
	if suffix:
		ver = "{}-{}".format(ver, suffix)

	version = semantic_version.Version(ver)
	print("  version: {}".format(version))
	print("  version.prerelease: {}".format(version.prerelease))
	return version


def main():
	os.chdir(os.path.dirname(__file__))	# Change working directory to this file's directory

	# Aggregate configuration data in a dictionary
	config = {
		"github": {
			"repo": os.environ["GITHUB_REPO"],
		},
		"nebula": {
			"user": os.environ["NEBULA_USER"],
			"password": os.environ["NEBULA_PASSWORD"],
			# OFP: releases upload to Nebula privately by default; set repo variable
			# NEBULA_PRIVATE=false to publish publicly (which also fires Nebula's announcement).
			"private": os.environ.get("NEBULA_PRIVATE", "true").strip().lower() != "false",
			# Option A (register GitHub URLs) needs Nebula's URLS_FOR allowlist. Default off:
			# upload the archives into Nebula storage instead (Option B). Set NEBULA_USE_URLS=true
			# once the account is allowlisted.
			"use_urls": os.environ.get("NEBULA_USE_URLS", "false").strip().lower() == "true",
		},
	}

	# bail if this script is run from a workflow that doesn't identify as a release or nightly
	if sys.argv[1] not in ("release", "nightly"):
		print("ERROR: Invalid release mode %s passed. Expected release or nightly!" % sys.argv[1])
		sys.exit(1)

	linux_success = os.environ["LINUX_RESULT"] == "success"
	windows_success = os.environ["WINDOWS_RESULT"] == "success"
	if (sys.argv[1] == "release"):
		if not (linux_success and windows_success):
			# bail if this is a release and either build workflow failed
			print("ERROR: One or more builds failed to publish! Cannot post release.")
			sys.exit(1)
	else:
		if not (linux_success or windows_success):
			# bail if this is a nightly build and both build workflows failed
			print("ERROR: All builds failed to publish! Cannot post nightly.")
			sys.exit(1)
	
	# bail if tag_name doesn't match with sys.argv[1]
	tag_name = os.environ["RELEASE_TAG"]	##!< commit tag string
	if (sys.argv[1] == "release") and (not tag_name.startswith("release_")):
		print("ERROR: Invalid tag name detected. Expected release_ prefix for release builds but found %s (full ref = %s)" % tag_name, os.environ["GITHUB_REF"])
		sys.exit(1)

	if (sys.argv[1] == "nightly") and (not tag_name.startswith("nightly_")):
		print("ERROR: Invalid tag name detected. Expected nightly_ prefix for nightly builds but found %s (full ref = %s)" % tag_name, os.environ["GITHUB_REF"])
		sys.exit(1)

	# bail if tag_name does not exist in the repo
	if not check_output(("git", "show-ref", "--tags", "refs/tags/%s" % tag_name), text=True):
		print("ERROR: Couldn't find tag %s in repo (git show-ref)!" % tag_name)
		sys.exit(1)

	# Populate version and date
	date = datetime.now()	##!< current date
	version = get_source_version(date.strftime(DATEFORMAT_VERSION), tag_name)	##!< form full version string
	
	# find the previous tag release (used by nightly)
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
		print("ERROR: Couldn't find tag %s in repo! (git for-each-ref)" % tag_name)
		sys.exit(1)

	if (sys.argv[1] == "nightly") and (not previous_tag):
		# couldn't find previous tag, bail
		print("ERROR: Could not find a previous tag for %s!" % tag_name)
		sys.exit(1)

	if sys.argv[1] == "release":
		# Release specific action
		print("Retrieving file list...")
		files, sources = file_list.get_release_files(tag_name, config)

		print("Hashing files...")
		if config["nebula"].get("use_urls"):
			# Option A: archives stay on GitHub; we only register their URLs (needs URLS_FOR).
			for file in files:
				installer.get_hashed_file_list(file)
		else:
			# Option B: upload each archive into Nebula storage (no URLS_FOR needed), reusing
			# the single download that content-hashing already performs.
			print("Uploading build archives to Nebula...")
			upload_session, upload_token = nebula.login_session(config)
			if not upload_token:
				print("ERROR: Nebula login failed; cannot upload archives.")
				sys.exit(1)

			def _upload_after_hash(f, local_file):
				if not nebula.upload_chunked(upload_session, upload_token, local_file, f.hash, f.size):
					print("ERROR: failed to upload %s to Nebula." % f.name)
					sys.exit(1)

			for file in files:
				installer.get_hashed_file_list(file, after_hash=_upload_after_hash)

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

		if "Mac" not in groups.keys():
			print("ERROR: No Mac builds were detected!")
			sys.exit(1)
		
		# z64: What dose this do???
		# print(installer.render_installer_config(version, groups, config))

		# Publish release builds to Nebula
		print("Publishing to Nebula...")
		if not version.prerelease:
			stability = "stable"
		elif any("rc" in str(part).lower() for part in version.prerelease):
			stability = "rc"
		else:
			stability = "nightly"
		nebula.submit_release(
			nebula.render_nebula_release(version, stability, files, config),
			config)

	else:
		# OFP does not publish nightlies through this script. Nightly builds are handled
		# separately, and this path previously relied on SCP-specific FTP/forum infra.
		print("Nightly publishing via ci/post is not used by OFP; nothing to do.")

	print("Done!")


if __name__ == "__main__":
	main()
