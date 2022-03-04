from typing import List

import requests
import semantic_version
from mako.template import Template

from file_list import ReleaseFile


class ForumAPI:
    def __init__(self, config):
        self.config = config

    def create_post(self, title, content, board):
        print(self.config["hlp"]["api"])
        print(self.config["hlp"]["key"])
        if (self.config["hlp"]["api"] == "") or (self.config["hlp"]["key"] == ""):
            print("Post failed! No API or API_KEY given!")
            return

        resp = requests.post(self.config["hlp"]["api"], data={
            "api_key": self.config["hlp"]["key"],
            "board": str(board),
            "subject": title,
            "body": content
        })

        if resp.text != "OK":
            print("Post failed! Response: %s" %resp.text)

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
