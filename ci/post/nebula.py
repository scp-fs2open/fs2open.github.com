import os.path
import sys
import traceback

import requests

from file_list import ReleaseFile
from util import retry_multi, GLOBAL_TIMEOUT

metadata = {
    'type': 'engine',
    'title': 'FSO',
    'notes': '',
    'banner': 'https://fsnebula.org/storage/0d/e7/bf64bcdea9a9c115969cfb784e1ca457d24a7c2da4fc6f213521c3bb6abb.png',
    'screenshots': [],
    'videos': [],
    'first_release': '2017-09-24',
    'cmdline': '',
    'mod_flag': ['FSO'],
    'tile': 'https://fsnebula.org/storage/ec/cc/0bf23e028c26d5175ff52d003bff85b0a17b0ddfc1130d65bdf6d36f6324.png',
    'logo': None,
    'release_thread': None,
    'description': '',
    'id': 'FSO',
    'packages': [],
}

platforms = {
    'Linux': 'linux',
    'MacOSX': 'macosx',
    'Win32-SSE2': 'windows',
    'Win64-SSE2': 'windows',
    'Win32-AVX': 'windows',
    'Win64-AVX': 'windows'
}

envs = {
    'Linux': 'linux && x86_64',  # Linux only has 64bit builds
    'MacOSX': 'macosx',
    'Win32-SSE2': 'windows',
    'Win64-SSE2': 'windows && x86_64',
    'Win32-AVX': 'windows && avx',
    'Win64-AVX': 'windows && avx && x86_64'
}

subdirs = {
    'Win32-SSE2': 'x86',
    'Win64-SSE2': 'x64',
    'Win32-AVX': 'x86_avx',
    'Win64-AVX': 'x64_avx'
}


def render_nebula_release(version, stability, files: list[ReleaseFile], config):
    meta = metadata.copy()
    meta['version'] = str(version)
    meta['stability'] = stability  # This can be one of ('stable', 'rc', 'nightly')

    for file in files:
        if file.content_hashes is None:
            # The extraction probably failed which is why we don't have any hashes
            continue

        group = file.group

        # release.py sets subgroup but nightly.py doesn't which means we have to normalize group here.
        if file.subgroup:
            group += '-' + file.subgroup

        pkg = {
            'name': group,
            'notes': '',
            'is_vp': False,
            'files': [{
                'dest': platforms[group] + '/' + subdirs.get(group, ''),
                'filesize': file.size,
                'checksum': ['sha256', file.hash],
                'urls': [file.url] + file.mirrors,
                'filename': file.name
            }],
            'environment': envs.get(group),
            'filelist': [],
            'status': 'required',
            'folder': None,
            'dependencies': [],
            'executables': []
        }

        for fn, checksum in file.content_hashes:
            if group in subdirs:
                dest_fn = subdirs[group] + '/' + fn
            else:
                dest_fn = fn

            # If the group is a known platform then we put the binaries into a separate subfolder for each platform
            if group in platforms:
                dest_fn = platforms[group] + '/' + dest_fn

            pkg['filelist'].append({
                'orig_name': fn,
                'checksum': ('sha256', checksum),
                'archive': file.name,
                'filename': dest_fn
            })

            if group == 'Linux' and fn.endswith('.AppImage'):
                if 'qtfred' in fn:
                    if '-FASTDBG' in fn:
                        label = 'QtFRED Debug'
                    else:
                        label = 'QtFRED'

                else:
                    if '-FASTDBG' in fn:
                        label = 'Fast Debug'
                    else:
                        label = None

                props = {
                    "x64": True,  # All Linux builds are 64-bit
                    "sse2": True,  # Linux builds are forced to compile with SSE2 but not AVX
                    "avx": False,
                    "avx2": False,
                }

                pkg['executables'].append({
                    'file': dest_fn,
                    'label': label,
                    'properties': props,
                })
            elif group == 'MacOSX' and fn.startswith(os.path.basename(fn) + '.app/'):
                if 'qtfred' in fn:
                    if '-FASTDBG' in fn:
                        label = 'QtFRED Debug'
                    else:
                        label = 'QtFRED'
                else:
                    if '-FASTDBG' in fn:
                        label = 'Fast Debug'
                    else:
                        label = None

                props = {
                    "x64": True,  # All Mac builds are 64-bit
                    "sse2": True,  # There are no forced options on mac but 64-bit always implies SSE2 so we set that here
                    "avx": False,
                    "avx2": False,
                }

                pkg['executables'].append({
                    'file': dest_fn,
                    'label': label,
                    'properties': props,
                })
            elif group.startswith('Win') and fn.endswith('.exe'):
                if 'fred2' in fn:
                    if '-FASTDBG' in fn:
                        label = 'FRED2 Debug'
                    else:
                        label = 'FRED2'

                elif 'qtfred' in fn:
                    if '-FASTDBG' in fn:
                        label = 'QtFRED Debug'
                    else:
                        label = 'QtFRED'

                else:
                    if '-FASTDBG' in fn:
                        label = 'Fast Debug'
                    else:
                        label = None

                props = {
                    "x64": "x64" in fn,
                    "sse2": "SSE2" in fn or "AVX" in fn,  # AVX implies SSE2
                    "avx": "AVX" in fn,  # This conveniently also covers the AVX2 case since AVX2 implies AVX
                    "avx2": "AVX2" in fn,
                }

                pkg['executables'].append({
                    'file': dest_fn,
                    'label': label,
                    'properties': props,
                })

        meta["packages"].append(pkg)

    return meta


@retry_multi(5)
def nebula_request(session, kind, path, **kwargs):
    uri = "https://fsnebula.org/api/1/{}".format(path)

    request_args = {
        "timeout": GLOBAL_TIMEOUT
    }
    request_args.update(kwargs)

    return session.request(kind, uri, **request_args)


def submit_release(meta, config):
    try:
        with requests.Session() as session:
            print('Logging into Nebula...')
            result = nebula_request(session, 'post', 'login', data={
                'user': config['nebula']['user'],
                'password': config['nebula']['password']
            })

            if result.status_code != 200:
                print('Login failed!')
                return False

            data = result.json()
            if not data['result']:
                print('Login failed!')
                return False

            print('Submitting release...')
            result = nebula_request(session, 'post', 'mod/release', headers={
                'X-KN-TOKEN': data['token']
            }, json=meta)

            if result.status_code != 200:
                print('Request failed!')
                return False

            data = result.json()
            if not data['result']:
                print('ERROR: ' + data['reason'])
                return False

            print('Success!')
            return True
    except Exception:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_exception(exc_type, exc_value, exc_traceback)
        return False
