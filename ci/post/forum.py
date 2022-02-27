from itertools import groupby
from typing import List

import requests
import semantic_version
from mako.template import Template

from file_list import ReleaseFile


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



class ForumAPI:
    def __init__(self, config):
        self.config = config

    def create_post(self, title, content, board):
        resp = requests.post(self.config["hlp"]["api"], data={
            "api_key": self.config["hlp"]["key"],
            "board": str(board),
            "subject": title,
            "body": content
        })

        if resp.text != "OK":
            print("Post failed!")

    def post_nightly(self, date, revision, files, log, success):
        print("Posting nightly thread...")

        title = "Nightly: {} - Revision {}".format(date, revision)

        template = Template(filename=self.config["templates"]["nightly"])
        rendered = template.render_unicode(**{
            "date": date,
            "revision": revision,
            "files": files,
            "log": log,
            "success": success
        })

        print("Creating post...")
        self.create_post(title, rendered, self.config["nightly"]["hlp_board"])

    def post_release(self, date, version: semantic_version.Version, groups, sources):
        print("Posting release thread...")

        title = "Release: {}".format(version)

        template = Template(
            filename=self.config["templates"]["release"].format(major=version.major, minor=version.minor),
            module_directory='/tmp/mako_modules')
        rendered = template.render_unicode(**{
            "date": date,
            "version": version,
            "groups": groups,
            "sources": sources
        }).strip("\n")

        print("Creating post...")

        self.create_post(title, rendered, self.config["release"]["hlp_board"])
