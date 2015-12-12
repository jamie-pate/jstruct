"""
C Preprocessor related functionality
"""
from subprocess import Popen, PIPE
import os

def preprocess(instr, cpp_path='cpp', cpp_args='', include_paths=[], defines=[]):
    """ Preprocess a a string using cpp.
        instr:
            c code to preprocess.
        cpp_path:
        cpp_args:
            Refer to the documentation of parse_file for the meaning of these
            arguments.
        When successful, returns the preprocessed file's contents.
        Errors from cpp will be printed out.
    """
    path_list = [cpp_path]
    if isinstance(cpp_args, list):
        path_list += cpp_args
    elif cpp_args != '':
        path_list += [cpp_args]
    path_list += ['-I' + os.path.abspath(p) for p in include_paths]
    path_list += ['-D ' + d for d in defines]
    # read from stdin
    path_list += ['-']

    try:
        # Note the use of universal_newlines to treat all newlines
        # as \n for Python's purpose
        #
        pipe = Popen(   path_list,
                        stdin=PIPE,
                        stdout=PIPE,
                        stderr=PIPE,
                        universal_newlines=True)
        text, err = pipe.communicate(instr)
    except OSError as e:
        raise RuntimeError("Unable to invoke 'cpp'.  " +
            'Make sure its path was passed correctly\n' +
            ('Original error: %s' % e))
    except Exception as ex:
        if hasattr(ex, 'child_traceback'):
            print(ex.child_traceback)
        raise ex

    return text, err
