[日本語](./README.ja.md)

[![windows](https://github.com/renatus-novus-x/posthub/workflows/windows/badge.svg)](https://github.com/renatus-novus-x/posthub/actions?query=workflow%3Awindows)
[![macos](https://github.com/renatus-novus-x/posthub/workflows/macos/badge.svg)](https://github.com/renatus-novus-x/posthub/actions?query=workflow%3Amacos)
[![ubuntu](https://github.com/renatus-novus-x/posthub/workflows/ubuntu/badge.svg)](https://github.com/renatus-novus-x/posthub/actions?query=workflow%3Aubuntu)

# posthub — A Minimal Maildir-like Messenger for Shared Directories

`posthub` is a lightweight, cross-platform local messaging system inspired by the Maildir format.
It allows multiple hosts sharing the same directory (via SMB, NFS, or HostFS) to exchange short messages and atomically.

## Download
- [posthub.exe (windows)](https://raw.githubusercontent.com/renatus-novus-x/posthub/main/bin/posthub.exe)
- [posthub (macos)](https://raw.githubusercontent.com/renatus-novus-x/posthub/main/bin/posthub)
- [posthub.x (X68000)](https://raw.githubusercontent.com/renatus-novus-x/posthub/main/bin/posthub.x)

## Directory Structure

```
posthub/
users.txt
alice/maildir/{tmp,new,cur}
bob/maildir/{tmp,new,cur}
```

- `users.txt` lists one username per line.
- Each user has a `Maildir` with the standard subdirectories.

## Usage

### Send a message to all users
```
posthub send all "hello!"
```
### Send a direct message to one user
```
posthub send alice "meet at 18:00?"
```
### Receive all pending messages for a user
```
posthub recv alice
```
Messages in new/ are printed to stdout and moved to cur/ after reading.

## Future Extensions

- Background receiver (polling or inotify/ReadDirectoryChangesW)
- Hub relay for TCP multiplexing

