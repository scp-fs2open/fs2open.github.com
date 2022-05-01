# Class for use with interfacing with github via HTTP requests

import requests

from util import retry_multi, GLOBAL_TIMEOUT

class GithubAPI:
    headers = {
        "Accept": "application/vnd.github.v3+json"
    }
    url_root = "https://api.github.com"

    def __init__(self, config):
        self.config = config

    @retry_multi(5)
    def get(self, path):
        """!
        @brief Performs a GET request with the given path. To be used with Github's REST API.
        @returns If successful, returns a .JSON object
        """
        url = self.url_root + path

        # GET https://api.github.com/<path> Accept: "application/vnd.github.v3+json"

        response = requests.get(url, headers=self.headers, timeout=GLOBAL_TIMEOUT)

        response.raise_for_status() # Raise a RequestException if we failed, and trigger retry

        return response.json()
    
    def get_release(self, tag_name):
        """!
        @brief Retrieves the Github Release metadata as a JSON object
        @see https://docs.github.com/en/rest/reference/releases#get-a-release-by-tag-name
        """
        return self.get("/repos/{}/releases/tags/{}".format(self.config["github"]["repo"], tag_name))


    def patch(self, path, data):
        """!
        @brief Performs a PATCH request with the given path and body data (as dictionary)
        @returns If successful, returns a .JSON object
        """
        url = self.url_root + path

        response = requests.patch(url, headers=self.headers, timeout=GLOBAL_TIMEOUT, data=data)

        response.raise_for_status()

        return response.json()
        
    def update_release(self, release_id, tag_name="", name="", body="", draft=None, prerelease=None):
        """!
        @brief Updates a Github Release
        @param[in] release_id   The id of the release, as obtained by get_release()
        @param[in] tag_name     (optional) The git tag this release is associated with
        @param[in] name         (optional) The name of the release
        @param[in] body         (optional) Body text describing the release
        @param[in] draft        (optional) If true, the release is marked as a "draft"
        @param[in] prerelease   (optional) If true, the release is marked as a "prerelease"

        @note If an optional argument is not provided, then its associated data is not changed
        """
        path = "/repos/{}/releases/{}".format(self.config["github"]["repo"], release_id)

        # Add to the data JSON if options are specified
        data = {}
        if tag_name != "":
            data["tag_name"] = tag_name
        
        if name != "":
            data["name"] = name

        if body != "":
            data["body"] = body
        
        if draft != None:
            data["draft"] = data

        if prerelease != None:
            data["prerelease"] = prerelease
        
        self.patch(path, data)

