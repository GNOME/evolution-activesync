# evolution-activesync

An ActiveSync protocol implementation for GNOME, providing email, calendar,
and contacts synchronisation with Microsoft Exchange servers via the
[Evolution](https://wiki.gnome.org/Apps/Evolution) mail client and
[Evolution Data Server](https://gitlab.gnome.org/GNOME/evolution-data-server).

## Components

1. **eas-daemon/** — A core library implementing the ActiveSync protocol,
   along with a dæmon which exposes all necessary functionality via D-Bus.
   Dual-licensed under LGPLv2.1+ *and* Apache v2.

2. **libeasaccount/** / **libeasclient/** — Client libraries used to access
   the above dæmon over D-Bus and to manage accounts in GSettings.
   Licensed under LGPLv2.1+.

3. **camel/** — A Camel (Evolution) email back end. Licensed under LGPLv2.1+.

## Supported ActiveSync versions

| Version | Status |
|---------|--------|
| 12.0 | Supported |
| 12.1 | Supported |
| 14.0 | Supported |
| 14.1 | Supported |
| 16.0 | Supported |
| 16.1 | Supported (default) |

The protocol version is negotiated automatically with the server via the HTTP `OPTIONS` command. The best mutually-supported version up to **16.1** is selected.

## Dependencies

- GLib 2.68+
- libsoup 3.0+
- libedataserver 1.2
- libebackend 1.2
- libebook 1.2
- libwbxml2 0.11+
- libical
- libxml2
- libsecret

For the Camel back end (`-DENABLE_CAMEL_BACKEND=ON`):
- camel 3.23.2+
- libemail-engine
- evolution-mail / evolution-shell

## Building

```sh
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
make install
```

### CMake options

| Option | Default | Description |
|---|---|---|
| `ENABLE_CAMEL_BACKEND` | `ON` | Build the Camel e-mail back end |
| `ENABLE_TESTS` | `ON` | Build the test suite |
| `ENABLE_MAINTAINER_MODE` | `OFF` | Enable extra compiler warnings |
| `LIB_SUFFIX` | *(empty)* | Library directory suffix (e.g. `64`) |

### Running tests

```sh
make check
```

## Patent Licensing

It is rumoured that in some parts of the world, there are patents
which cover the use of ActiveSync from a mobile client.

No evidence has been discovered which can confirm the truth of this;
nobody who repeats these rumours can cite specific patents which read
on the methods herein.

Furthermore, a technical assessment of the code would suggest that
there is nothing here that could *possibly* be covered by a patent,
*even* under the corrupt and widely-abused US patent system.

Thus, the rumoured patents covering the ActiveSync implementation
would seem to be either on the server side where they do not affect
us, or on obscure parts of the protocol which we have not needed to
implement in this client. For the basic email and calendar
synchronisation, there is *nothing* here which is not *completely*
trivial and obvious, and which has not been done elsewhere, long ago.

If you are contacted by any individual or corporation claiming to have
patents covering any of the code herein, and attempting to extort
payment from you in return for licensing of their alleged patents,
they are likely to be committing the criminal offence of fraud. You
should, in the first instance, refer the matter to your local
constabulary. Either they do not have patents which truly read upon
this code and are simply trying to extort money with menaces, or if
they *do* have patents in some parts of the world which read on this
code base, they must have obtained those patents (or the relevant claims
in those patents) by misrepresentation. Which makes it fraud.

You may well find that such a extortionist would refuse to specify
which patents they claim are relevant; they just want to threaten you
in the expectation that you will pay them off without investigating
their claims.  You should certainly not even *contemplate* paying for
a patent licence without demanding to see specific details of the
alleged patents which apply in your own country, and making your own
assessment as to whether those patents are valid and whether they
truly do read on this code base.

Nevertheless, the copyright licensing of this code base *DOES* allow
you to obtain a patent licence, if you see fit to support corrupt
business practices by doing so, and if your local laws do not prevent
you from making payments which contribute to corruption in foreign
countries.

Although the LGPLv2.1 alone would prevent you from distributing this
code if you had paid for a limited patent licence, the existence of
the dual-licence Apache v2 option means that you may continune to
distribute the code under *that* licence, even if you have paid for
patent licensing.

That is the reason for the dual LGPLv2.1 + Apachev2 licensing on the
libeas library and the ActiveSync dæmon which implement the core of
the protocol.

NOTE: This is not legal advice. But it *is* common sense.

David Woodhouse <dwmw2@infradead.org>

     (Not speaking for my employer,
                   for any ex-employer(s),
                   or any future employer(s)).
