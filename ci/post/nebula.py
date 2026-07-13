import os.path
import sys
import hashlib
import traceback

import requests

from file_list import ReleaseFile
from util import retry_multi, GLOBAL_TIMEOUT

LINUX_X64_KEY = "Linux-x86_64"
LINUX_ARM64_KEY = "Linux-arm64"
MACOSX_X64_KEY = "Mac-x86_64"
MACOSX_ARM64_KEY = "Mac-arm64"
WIN32_SSE2_KEY = "Win32-SSE2"
WIN64_SSE2_KEY = "Win64-SSE2"
WIN32_AVX_KEY = "Win32-AVX"
WIN64_AVX_KEY = "Win64-AVX"
WINARM64_KEY = "WinARM64"

metadata = {
    'type': 'engine',
    'title': 'OFP',
    'notes': '',
    'banner': '17c6709138d5a8959c5605a1d6f465d6c1a0dada9881f5b905c01336848bb8de',
    'screenshots': [],
    'videos': [],
    'first_release': '2026-07-12',
    'cmdline': '',
    'mod_flag': ['OFP'],
    'tile': '12372e28d4654659b3f3a38f52b627a4ad37fb0af4fef63fa9dafa6afe5a27db',
    'logo': None,
    'release_thread': None,
    'description': '',
    'id': 'OFP',
    'packages': [],
}

platforms = {
    LINUX_X64_KEY: 'linux',
    LINUX_ARM64_KEY: 'linux',
    MACOSX_X64_KEY: 'macosx',
    MACOSX_ARM64_KEY: 'macosx',
    WIN32_SSE2_KEY: 'windows',
    WIN64_SSE2_KEY: 'windows',
    WIN32_AVX_KEY: 'windows',
    WIN64_AVX_KEY: 'windows',
    WINARM64_KEY: 'windows',
}

envs = {
    LINUX_X64_KEY: 'linux && x86_64',
    LINUX_ARM64_KEY: 'linux && arm64',
    MACOSX_X64_KEY: 'macosx && x86_64',
    MACOSX_ARM64_KEY: 'macosx && arm64',
    WIN32_SSE2_KEY: 'windows',
    WIN64_SSE2_KEY: 'windows && x86_64',
    WIN32_AVX_KEY: 'windows && avx',
    WIN64_AVX_KEY: 'windows && avx && x86_64',
    WINARM64_KEY: 'windows && arm64'
}

subdirs = {
    WIN32_SSE2_KEY: 'x86',
    WIN64_SSE2_KEY: 'x64',
    WIN32_AVX_KEY: 'x86_avx',
    WIN64_AVX_KEY: 'x64_avx'
}


def render_nebula_release(version, stability, files, config):
    meta = metadata.copy()
    meta['version'] = str(version)
    meta['stability'] = stability  # This can be one of ('stable', 'rc', 'nightly')
    meta['private'] = config['nebula'].get('private', True)  # OFP: default private until explicitly published

    for file in files:
        if file.content_hashes is None:
            # The extraction probably failed which is why we don't have any hashes
            continue

        group = file.group

        # release.py sets subgroup but nightly.py doesn't which means we have to normalize group here.
        if file.subgroup:
            group += '-' + file.subgroup

        file_entry = {
            'dest': platforms[group] + '/' + subdirs.get(group, ''),
            'filesize': file.size,
            'checksum': ['sha256', file.hash],
            'filename': file.name
        }
        if config['nebula'].get('use_urls'):
            # Option A: register externally-hosted (GitHub) URLs. Requires the account to be
            # in Nebula's URLS_FOR allowlist. When off, the archive is uploaded to Nebula
            # (Option B) and matched here by checksum instead.
            file_entry['urls'] = [file.url] + file.mirrors

        pkg = {
            'name': group,
            'notes': '',
            'is_vp': False,
            'files': [file_entry],
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

            if group.startswith('Linux') and fn.endswith('.AppImage'):
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
                    "arm64": "arm64" in fn,
                    "x64": "x64" in fn,
                    "sse2": True,  # Linux builds are forced to compile with SSE2 but not AVX
                    "avx": False,
                    "avx2": False,
                }

                pkg['executables'].append({
                    'file': dest_fn,
                    'label': label,
                    'properties': props,
                })
            elif group.startswith('Mac') and fn.startswith(os.path.basename(fn) + '.app/'):
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
                    "arm64": "arm64" in fn,
                    "x64": "x64" in fn,
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
                    "arm64": "arm64" in fn,
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


# Size of each chunk sent to Nebula's multiupload endpoint (Option B uploads).
UPLOAD_CHUNK_SIZE = 10 * 1024 * 1024


def login_session(config):
    """! Opens a requests Session and logs into Nebula.

    @returns (session, token). token is None if the login failed.
    """
    session = requests.Session()
    result = nebula_request(session, 'post', 'login', data={
        'user': config['nebula']['user'],
        'password': config['nebula']['password']
    })
    if result.status_code != 200 or not result.json().get('result'):
        return session, None
    return session, result.json()['token']


def upload_chunked(session, token, fileobj, checksum, filesize):
    """! Uploads a file into Nebula's own storage via the multiupload API (Option B).

    No-op if a file with this checksum is already present. `fileobj` must be a seekable
    binary stream; `checksum` is its sha256 (also used as the upload id).

    @returns True on success, False otherwise.
    """
    headers = {'X-KN-TOKEN': token}

    # Skip if Nebula already has this exact file (re-runs, or shared across releases)
    chk = nebula_request(session, 'post', 'upload/check', headers=headers,
                         data={'checksum': checksum})
    if chk.status_code == 200 and chk.json().get('result'):
        print("  already on Nebula, skipping upload: {}".format(checksum))
        return True

    parts = max(1, (filesize + UPLOAD_CHUNK_SIZE - 1) // UPLOAD_CHUNK_SIZE)

    start = nebula_request(session, 'post', 'multiupload/start', headers=headers,
                          data={'id': checksum, 'size': str(filesize), 'parts': str(parts)})
    if start.status_code != 200 or not start.json().get('result'):
        print("  ERROR: multiupload/start failed for {}".format(checksum))
        return False
    finished = set(start.json().get('finished_parts', []))

    fileobj.seek(0)
    for idx in range(parts):
        chunk = fileobj.read(UPLOAD_CHUNK_SIZE)
        if idx in finished:
            continue

        part = nebula_request(session, 'post', 'multiupload/part', headers=headers,
                             data={'id': checksum, 'part': str(idx)},
                             files={'file': ('chunk', chunk)})
        if part.status_code != 200:
            print("  ERROR: multiupload/part {} failed for {}".format(idx, checksum))
            return False

        verify = nebula_request(session, 'post', 'multiupload/verify_part', headers=headers,
                               data={'id': checksum, 'part': str(idx),
                                     'checksum': hashlib.sha256(chunk).hexdigest()})
        if verify.status_code != 200 or not verify.json().get('result'):
            print("  ERROR: multiupload/verify_part {} failed for {}".format(idx, checksum))
            return False

    finish = nebula_request(session, 'post', 'multiupload/finish', headers=headers,
                           data={'id': checksum, 'checksum': checksum})
    if finish.status_code != 200 or not finish.json().get('result'):
        print("  ERROR: multiupload/finish failed for {}: {}".format(checksum, finish.text[:200]))
        return False

    print("  uploaded to Nebula: {}".format(checksum))
    return True


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
