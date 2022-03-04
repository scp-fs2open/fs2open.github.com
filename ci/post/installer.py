import os
import hashlib
import tarfile
import traceback
import zipfile
from tempfile import NamedTemporaryFile
from zipfile import ZipFile

import requests
import sys
from mako.template import Template

from file_list import ReleaseFile
from util import GLOBAL_TIMEOUT


def _download_file(url, dest_file, session):
    print("Downloading " + url)
    # NOTE the stream=True parameter
    r = session.get(url, stream=True, timeout=GLOBAL_TIMEOUT)
    for chunk in r.iter_content(chunk_size=1024):
        if chunk:  # filter out keep-alive new chunks
            dest_file.write(chunk)
    dest_file.flush()


def _gen_hash(fileobj, hash_alg):
    h = hashlib.new(hash_alg)
    while True:
        data = fileobj.read(4096)

        if not data:
            break

        h.update(data)

    return h.hexdigest()


def get_hashed_file_list(file: ReleaseFile, session: requests.Session = None, hash_alg: str = "sha256"):
    if session is None:
        session = requests.Session()

    with NamedTemporaryFile('w+b', suffix=file.filename) as local_file:
        _download_file(file.url, local_file, session)

        filename = local_file.name
        hash_list = []

        local_file.seek(0)
        file.hash = _gen_hash(local_file, hash_alg)
        file.size = os.stat(filename).st_size

        try:
            if tarfile.is_tarfile(filename):
                with tarfile.open(filename) as archive:
                    for entry in archive:
                        if not entry.isfile():
                            continue

                        print("Computing hash for " + entry.name)
                        fileobj = archive.extractfile(entry)
                        hash_list.append((entry.path, _gen_hash(fileobj, hash_alg)))
            elif zipfile.is_zipfile(filename):
                with ZipFile(filename) as archive:
                    for entry in archive.infolist():
                        # Python 3.6 has is_dir but that version is relatively new so we'll use the "safe" version here
                        if entry.filename.endswith('/'):
                            continue

                        print("Computing hash for " + entry.filename)
                        with archive.open(entry) as fileobj:
                            hash_list.append((entry.filename, _gen_hash(fileobj, hash_alg)))
            else:
                raise NotImplementedError("Unsupported archive type!")
        except:
            traceback.print_exception(*sys.exc_info())
            return

        file.content_hashes = hash_list


def render_installer_config(version, groups, config):
    # filename=config["templates"]["installer"]
    # NOTE: z64: no idea what this originally was intended to be, forced to "release"
    template = Template(filename=config["templates"]["release"], module_directory='/tmp/mako_modules')
    return template.render(**{
        "version": version,
        "groups": groups,
    }).strip("\n")
