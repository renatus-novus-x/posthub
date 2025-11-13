[日本語](./README.ja.md)

[![windows](https://github.com/renatus-novus-x/posthub/workflows/windows/badge.svg)](https://github.com/renatus-novus-x/posthub/actions?query=workflow%3Awindows)
[![macos](https://github.com/renatus-novus-x/posthub/workflows/macos/badge.svg)](https://github.com/renatus-novus-x/posthub/actions?query=workflow%3Amacos)
[![ubuntu](https://github.com/renatus-novus-x/posthub/workflows/ubuntu/badge.svg)](https://github.com/renatus-novus-x/posthub/actions?query=workflow%3Aubuntu)

<table>
  <tr>
    <td width="50%">
      <img src="https://raw.githubusercontent.com/renatus-novus-x/posthub/main/images/x68k.gif" alt="x68k" title="x68k" width="100%">
    </td>
    <td width="50%">
      <img src="https://raw.githubusercontent.com/renatus-novus-x/posthub/main/images/windows.png" alt="windows" title="windows" width="100%">
    </td>
  </tr>
</table>

# posthub — A Minimal Maildir-like Messenger for Shared Directories

`posthub` is a lightweight, cross-platform (Windows/macOS/Ubuntu/X68000) local messaging system inspired by the Maildir format. It allows multiple hosts sharing the same directory (via SMB, NFS, or HostFS) to exchange short messages atomically.

## Download
- [posthub.exe (windows)](https://raw.githubusercontent.com/renatus-novus-x/posthub/main/bin/posthub.exe)
- [posthub-macos (macos)](https://raw.githubusercontent.com/renatus-novus-x/posthub/main/bin/posthub-macos)
- [posthub-ubuntu (ubuntu)](https://raw.githubusercontent.com/renatus-novus-x/posthub/main/bin/posthub-ubuntu)
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

## Initialization with posthub-init.{bat,sh}

The helper scripts `posthub-init.bat` or `posthub-init.sh` create a minimal example setup with two users and their Maildir-like directories.

- users.txt is populated with two sample users: alice and bob.
- Each user gets a maildir with the standard tmp / new / cur subdirectories.

These scripts are only meant as a minimal example. In a real setup, you are expected to edit users.txt and add your own user maildir trees as needed.

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

