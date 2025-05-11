# RC_S660_S_Arduino
SONY RC-S660/S Driver for Microcontrollers (e.g., Arduino UNO R4 in the PlatformIO Environment)

個人で買えるMIC/KS付きNFCカードリーダ RC-S660/S を MCU から制御するためのドライバです。

Version 1.0 のコマンドリファレンスマニュアルベースで作成しています。

![image](https://www.hmpower.jp/github_img/rc_s660s_driverformcu/about.gif)

Arduinoを想定していますが、他のマイコンでも使用したいので uart ハードウェア部をカプセル化しています。

※ .c ファイルで書いていた頃の名残で共通ヘッダファイル rcs660s_uart_hw_interface.h で、Java で言うクラスとインタフェースの関係を模擬実現しています。

**rcs660s_uart.h** 内で include したい実体を指定します。

| 用途 | 使用する実体 | 使用例 |
|---|---|---|
| Arduinoアプリ | rcs660s_uart_hw_arduino.h | rcs660s_app_if.h のAPIをアプリから叩く |
| UARTコマンドジェネレータ |rcs660s_uart_hw_windowspc.h<br>(src_winディレクトリ内にあります)  |1 main関数から各APIを叩いて実行<br>2 画面に表示されたコマンドをコピー&ペーストしてシリアル通信で送る |
| TeraTermマクロジェネレータ | rcs660s_uart_hw_windowspc.h<br>(src_winディレクトリ内にあります)  |1 main関数から各APIを叩いて実行<br>2 画面に表示されたコマンドをコピー&ペーストしてTTLファイルで保存<br>3 USBシリアルアダプタでRC-S660/Sを接続しTeraTermマクロ実行<br>詳細：https://www.hmcircuit.jp/nfc/rcs660_hands_on.html  |


### 設計せずに実装していたらスパゲッティコード化しました。

アプリ公開用 I/F クラスを用意して臭い物に蓋をしておきますw

このドライバを使う作品が完成したらリファクタリングしたい・・・

# Arduinoアプリ での使用

## 対応カード
今のところ NFC-Type B + Type A の一部機能 です。

免許証をタッチしないとエンジン掛からないシステム( https://github.com/hmpow/DLC_Starter )
の部品として作成したもので、免許証(従来・マイナ)でのみ NFC コマンド実行の動作確認済みです。

## 環境
下記で開発しています。

マイコンボード : Arduino UNO R4 WiFi

PC : Windows 11 Pro, VSCode, PlatformIO

**ハードウェアはArduino を使っていますが、PlatformIO 前提になっており、ArduinoIDEではビルドできません**

## Arduino UNO R4 WiFi での回路例

RC-S660/S は UART が 3.3V 系であるため、レベル変換回路が必要になります。

![image](https://www.hmpower.jp/github_img/rc_s660s_driverformcu/circuit.gif)