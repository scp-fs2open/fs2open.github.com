# Retrives a log of commit messages and attempts to pretty them into a human-readbile format

import datetime
import os
import re
import requests
import sys

from datetime import datetime
from subprocess import check_output

MAX_PAGES = 100  # maximum number of PR pages to go through
warnings = 0

def get_last_page(response: requests.Response) -> int:
    """
    @brief Extract the last page number from the response header

    @param[in] response The response from a GET call

    @returns The page number of the last page, or MAX_PAGES
    """

    # The link header is a heckn dictionary in string form, and for that matter the
    # value is in front of the key.  It is of the form '<<url1>>; rel=<key1>, <url2>, <key2>'
    link = response.headers['link']
    link = link.split(',')
    for x in link:
        y = x.split(';')        # <url>; rel=<key>
        if "last" not in y[1]:  # look for rel='last'
            continue
        # search in the url for `page=`` and grab the int immediately after
        # the int may be delimited by '&' or '>'
        str = y[0][y[0].find('page=') : ]   # slice everything out before 'page='
        str = str.split('&', 1)[0]          # slice out any possible queries after page=<int>
        last_page = int( re.sub('\D', "", str) )    # strip out just the page number, as integer
        if last_page > MAX_PAGES:
            return MAX_PAGES
        else:
            return last_page
    

def main():
    os.chdir(os.path.dirname(__file__))	# Change working directory to this file's directory
    
    start_date = datetime.strptime(sys.argv[1], "%Y-%m-%d")

    print("start_date: {}".format(start_date))

    # Retrive the git log for master, since start date, order by date, in ascending order, use YYYY-MM-DD date format,
    #   each line is of the form "<author short date> | <message>", only grab commits from master,
    #   output to `fsofordigest.txt`,
    
    # check_output(("git log master", "--after=2021-09-15", "--date-order", "--reverse", "--date=short",
    # "--pretty=\"%as | %s\"",  "--first-parent",
    # "--output fsofordigest.txt"), text=True)

    # The commit messages are either of the form "<body> (#<PR_number>)" or "Merge pull request #<PR_number> from <source_repo>"
    # For our purposes, we want them be all of the form "<body> (#<PR_number>)"
    # Typically, the second form does not have a usable body, so we'll have to pull the PR title from github

    # Github's REST API allows us to retrieve PR's, but due to it being HTTP requests it has to page in the data.
    # We can loop over the GET request and check for a merge date.
    # The GET request doesn't have sorting by merge date, but does have sorting by the last time it was updated... which is usually
    # also the merge date but not always.

    # Key attributes within the PR JSON objects:
    #   "merged_at: <date>"
    #   "merged: <bool>"
    #   "html_url: <string>"
    #   "number: <int>"     # The PR id number
    #   "title: <string>"   # Title of the PR
    #   "body: <string>"    # The body paragraph detailing the PR
    #   "labels: [label_object]"    # All labels attached to this PR

    changelog = {}
    url = "https://api.github.com/repos/scp-fs2open/fs2open.github.com/pulls"
    page = 1
    last_page = MAX_PAGES
    while (page <= last_page):
        # GET <url>?base=master&sort=updated&direction=desc&state=closed&page=1
        response = requests.get(url, params={
            "base": "master",
            "direction": "desc",
            "page": page,
            "state": "closed",
            "sort": "updated"
        })

        # Check if good response
        if (response.status_code != 200) :
            print("Error: ", response.status_code)
            print(response.json()['message'])
            sys.exit(1)
        else:
            print("Request successful: 200")
            print("page={}".format(page))

        # Update the last_page index at first iteration and possibly last iteration
        # This should cover the edge case of a new PR being added to the database before this script is finished
        if (page == 1) or (page == last_page):
            last_page = get_last_page(response)
            print("last_page={}".format(last_page))

        # assume pulls are organized from newest to oldest
        for pull in response.json():
            if (pull['state'] != 'closed'):
                # This pull is not merged, so ignore
                print("Found irrelevant pull, skipping...")
                print("state: {}; number: {}".format(pull['state'], pull['number']))
                continue
            
            if (pull['merged_at'] == None):
                # This pull wasn't merged, skip
                print("Found closed pull #{} ({}), skipping...".format(pull['number'], pull['closed_at']))
                continue

            merge_date = datetime.strptime(pull['merged_at'], "%Y-%m-%dT%H:%M:%SZ")
            updated_date = datetime.strptime(pull['updated_at'], "%Y-%m-%dT%H:%M:%SZ")            
            if (merge_date < start_date) and (updated_date >= start_date):
                # This pull is too old, but was updated within our timeframe. skip
                print("Found old pull #{} ({}), skipping...".format(pull['number'], pull['merged_at']))
                continue

            if (updated_date < start_date):
                # This pull is too old
                print("Found old pull #{} ({}), stopping loop...".format(pull['number'], pull['updated_at']))
                page = last_page + 1
                break

            # Add the pull to the changelog, indexed by its pull.number
            if pull['number'] not in changelog:
                # only add if the key isn't there already
                changelog[pull['number']] = {
                    "title":    pull['title'],
                    "labels":   pull['labels'],
                    "url":      pull['html_url']
                }
                print("Added pull #{} ({})".format(pull['number'], merge_date))
            # Else, silently ignore duplicate (for now...)

        page += 1


    # Write the changelog to file
    print("Writing to change.log...")
    str = ''
    for key, value in changelog.items():
        str += "{} ({})\n".format(value['title'], key)
    
    with open("change.log", "w") as file:
        file.write(str)

    print("Done!")

main()