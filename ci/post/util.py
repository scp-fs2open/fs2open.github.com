# miscellaneous utilities

import os
import time
from functools import wraps

from requests import RequestException

GLOBAL_TIMEOUT = 30

def retry_multi(max_retries):
    """!
    @brief Decorator - Retry a function `max_retries` times.
    
    @param [in] `max_retries` Maximum number of times to retry the decorated function before giving up

    @details  Intended for use with functions from the `request` module, only catches `RequestException`'s
    @note This has been copied from https://stackoverflow.com/a/23892489
    """

    def retry(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            num_retries = 0
            while num_retries <= max_retries:
                try:
                    ret = func(*args, **kwargs)
                    break
                except RequestException:
                    if num_retries == max_retries:
                        raise
                    num_retries += 1
                    time.sleep(5)
            return ret

        return wrapper

    return retry


def expand_config_vars(config):
    """
    @brief Expands the shell variable for the repo string from `config`
    """

    config["git"]["repo"] = os.path.expandvars(config["git"]["repo"])
