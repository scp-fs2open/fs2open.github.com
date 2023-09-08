from typing import List

import requests
import semantic_version
from mako.template import Template

from file_list import ReleaseFile


class ForumAPI:
    POST_SIZE_MAX = 50000
    """!
    Maximum number of characters allowed in a post, including BBCode (03/17/2022)

    Check with HLP admins occasionally to verify this limit, they can set it in the forum's admin panel
    """

    def __init__(self, config):
        self.config = config

    def create_post(self, title, content, board):
        """!
        @brief Create a post on the forums

        @param[in] title    Title of the post
        @param[in] content  Body of the post
        @param[in] board    Which board/forum to post to

        @returns { "thread_url": <url_of_created_thread> }
        """

        if (self.config["hlp"]["api"] == "") or (self.config["hlp"]["key"] == ""):
            print("Post failed! No API or API_KEY given!")
            return

        resp = requests.post(self.config["hlp"]["api"], data={
            "api_key": self.config["hlp"]["key"],
            "board": str(board),
            "subject": title,
            "body": content
        })

        if resp.status_code != 200:
            print("Post failed! Response: %s" %resp.text)
        
        return resp.json()

    def post_nightly(self, date, revision, files, log, success):
        """!
        @brief Post a new Nightly topic on the forums using nightly.mako

        @param[in] date         The date to use
        @param[in] version      The build's version
        @param[in] groups       dict[Any, file_list.FileGroup]; FileGroups of SSE2, AVX, etc.
        @param[in] sources      URL's of builds on github or fs2net
        """

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
        """!
        @brief Post a new Release topic on the forums using release.mako.

        @param[in] date         The date to use
        @param[in] version      The build's version
        @param[in] groups       dict[Any, file_list.FileGroup]; FileGroups of SSE2, AVX, etc.
        @param[in] sources      URL's of builds on github or fs2net

        @returns If Successful, the URL of the created post
        """

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

        json = self.create_post(title, rendered, self.config["release"]["hlp_board"])
        return json["thread_url"]
