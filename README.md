# GHCLI

An effort to build a somewhat portable, simple and easily
comprehendable GitHub commandline utility so you can avoid the web
browser entirely.

But hold on, there is an official utility out there!

Yes, but it is written in Go, which is a security nightmare for sane
people using Tor. This is, because the Go runtime circumvents the DNS
resolution mechanisms of the Operating System entirely, thus at least
leaking your IP to the DNS server.

## Building

- Required dependencies:
  - libcurl
  - pkg-config
  - C99 Compiler and linker
  - make (bmake or smake is recommended)
    + BUG WARNING: GNU make has two longstanding bugs in the include
      directive. This may lead to it generating files in the incorrect
      order and spitting out incorrect warnings or causing builds to
      fail. You may want to use smake or bmake instead.

In order to perform a build, do:
```console
$ make
# make DESTDIR=/ PREFIX=/usr/local/ install
```

You may leave out `DESTDIR` and `PREFIX`. The above are the default
values.

In case any of this does not work, please either report a bug, or
submit a patch in case you managed to fix it. Please also try to do a
`make snmk-update`, as I sometimes update my Makefile template without
syncing it to this project. This target will pull the latest files and
rebuild the project. You will need
[cURL](https://github.com/curl/curl) for this to work.

Tested Operating Systems so far:
- FreeBSD 13.0-RELEASE amd64 and arm64
- Solaris 10 sparc64
- Devuan GNU/Linux Chimaera x86_64
- Fedora 34 x86_64
- Haiku x86_64

## Support

You can ask your local frenchman aka. neutaaaaan for emotional support
when using this piece of software. Otherwise you can read the man page
at »man ghcli«.

## Bugs and contributions

Please report bugs to nsonack@outlook.com .
You can also submit patches this way using git-send-email.

## License

BSD-2 CLAUSE (aka. FreeBSD License). Please see the LICENSE file
attached.


                                     herrhotzenplotz aka. Nico Sonack
                                                         October 2021
