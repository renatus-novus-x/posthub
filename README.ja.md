[English](./README.md)

[![windows](https://github.com/renatus-novus-x/posthub/workflows/windows/badge.svg)](https://github.com/renatus-novus-x/posthub/actions?query=workflow%3Awindows)
[![macos](https://github.com/renatus-novus-x/posthub/workflows/macos/badge.svg)](https://github.com/renatus-novus-x/posthub/actions?query=workflow%3Amacos)
[![ubuntu](https://github.com/renatus-novus-x/posthub/workflows/ubuntu/badge.svg)](https://github.com/renatus-novus-x/posthub/actions?query=workflow%3Aubuntu)

# posthub - 共有ディレクトリを用いた最小構成の Maildir 風メッセンジャー

`posthub` は、Maildir形式に着想を得た、軽量でクロスプラットフォームなローカルメッセージングシステムです。SMB、NFS、HostFS経由で同一ディレクトリを共有する複数のホスト間で、短いメッセージを atomic に交換できます。

## Download
- [posthub.exe (windows)](https://raw.githubusercontent.com/renatus-novus-x/posthub/main/bin/posthub.exe)
- [posthub (macos)](https://raw.githubusercontent.com/renatus-novus-x/posthub/main/bin/posthub)
- [posthub.x (X68000)](https://raw.githubusercontent.com/renatus-novus-x/posthub/main/bin/posthub.x)

## ディレクトリ構造

```
posthub/
users.txt
alice/maildir/{tmp,new,cur}
bob/maildir/{tmp,new,cur}
```

- `users.txt` 1 行につき 1 つのユーザー名をリストします。
- 各ユーザーは `Maildir` 形式の標準サブディレクトリを持つ必要があります。

## 使い方

### すべてのユーザーにメッセージを送信する
```
posthub send all "hello!"
```
### 1人のユーザーにダイレクトメッセージを送信する
```
posthub send alice "meet at 18:00?"
```
### ユーザーの保留中のメッセージをすべて受信する
```
posthub recv alice
```
Messages in new/ are printed to stdout and moved to cur/ after reading.

## 将来の拡張

- バックグラウンド レシーバー (ポーリングまたは inotify/ReadDirectoryChangesW)
- TCP 多重化のための hub リレー

