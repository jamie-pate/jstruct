#Utility resources

Extra files that help when generating jstruct header files

## fake_libc_include
Copied from https://github.com/eliben/pycparser/tree/master/utils/fake_libc_include

Used to override actual c headers since pycparser needs all symbols defined,
but they don't necessarily have to be correct. Using the fake includes is also faster.
