[English](./README.md)

[![windows](https://github.com/renatus-novus-x/iperf/workflows/windows/badge.svg)](https://github.com/renatus-novus-x/iperf/actions?query=workflow%3Awindows)
[![macos](https://github.com/renatus-novus-x/iperf/workflows/macos/badge.svg)](https://github.com/renatus-novus-x/iperf/actions?query=workflow%3Amacos)
[![ubuntu](https://github.com/renatus-novus-x/iperf/workflows/ubuntu/badge.svg)](https://github.com/renatus-novus-x/iperf/actions?query=workflow%3Aubuntu)

<table>
  <tr>
    <td width="50%">
      <img src="https://raw.githubusercontent.com/renatus-novus-x/iperf/main/images/server.gif" alt="server" title="server" width="100%">
    </td>
    <td width="50%">
      <img src="https://raw.githubusercontent.com/renatus-novus-x/iperf/main/images/client.gif" alt="client" title="client" width="100%">
    </td>
  </tr>
</table>

# iperf
   最小限のシングルスレッドTCPスループットテスター（クライアント/サーバー）
   クロスプラットフォーム: Windows / macOS / Linux / X68000
   - サーバー: 受信して破棄し、1秒あたりのレートと合計レートを出力
   - クライアント: 指定秒数で固定サイズのチャンクを送信し、1秒あたりのレートと合計レートを出力
## Download
- [iperf.exe (windows)](https://raw.githubusercontent.com/renatus-novus-x/iperf/main/bin/iperf.exe)
- [iperf (macos)](https://raw.githubusercontent.com/renatus-novus-x/iperf/main/bin/iperf)
- [iperf.x (x68000)](https://raw.githubusercontent.com/renatus-novus-x/iperf/main/bin/iperf.x)

## ワークフロー例 (LAN テスト)

### 1. サーバー (受信側) を起動します

**サーバーマシン** (Windows / Mac / Linux / X68000) で以下のコマンドを実行します:

```bash
./iperf s 5201
```

想定される出力:

```
[server] listen on port 5201 ...
[server] local=192.168.1.23:5201 remote=192.168.1.45:53422
[server] 0-1s: 7​​52640 bytes 6.02 Mb/s (0.75 MB/s)
[server] TOTAL: 7532800 bytes in 10.00s 6.02 Mb/s (0.75 MB/s)
```

---

### 2. クライアント (送信側) を起動します

**クライアントマシン** (例: Windows / Mac / Linux / X68000):

```bash
./iperf c <server-ip> 5201 10 64
```

- `<server-ip>` : サーバーのIPアドレス (例: `192.168.1.23`)
- `5201` : ポート番号 (サーバーのポート番号と一致している必要があります)
- `10` : テスト実行時間 (秒)
- `64` : 送信ごとのバッファサイズ (KB)

例:

```bash
./iperf c 192.168.1.23 5201 10 64
```

期待される出力:

```
[client] connect 192.168.1.23:5201 ...
[client] seconds=10  buf=64KB  (single TCP stream, IPv4)
[client] 0-1s: 654321 bytes  5.23 Mb/s (0.65 MB/s)
[client] TOTAL: 6532100 bytes in 10.00s  5.23 Mb/s (0.65 MB/s)
```

---

### 3. スループットの確認

サーバーとクライアントの両方で、1秒あたりおよび合計スループットが以下の単位で表示されます。
- **Mb/秒** (メガビット/秒)
- **MB/秒** (メガバイト/秒)

これにより、単一のTCP接続を使用して2台のマシン間で簡単な**LAN速度テスト**を行うことができます。

## Profile

<img src="https://raw.githubusercontent.com/renatus-novus-x/iperf/main/images/profile.png" title="profile" />