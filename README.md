# RC_S660_S_Arduino
SONY RC-S660/S Driver for Microcontrollers (e.g., Arduino UNO R4 in the PlatformIO Environment)

個人で買えるMIC/KS付きNFCカードリーダ RC-S660/S を MCU から制御するためのドライバです。

Version 1.0 のコマンドリファレンスマニュアルベースで作成しています。

![image](https://www.hmpower.jp/github_img/rc_s660s_driverformcu/about.gif)

Arduinoを想定していますが、他のマイコンでも使用したいので uart ハードウェア部をカプセル化しています。

※ .c ファイルで書いていた頃の名残で共通ヘッダファイル rcs660s_uart_hw_interface.h で、Java で言うクラスとインタフェースの関係を模擬実現しています。

### 設計せずに実装していたらスパゲッティコード化しました。

アプリ公開用 I/F クラスを用意して臭い物に蓋をしておきますw

このドライバを使う作品が完成したらリファクタリングしたい・・・

## 対応カード
今のところ NFC-Type B + Type A の一部機能 です。

## 環境
下記で開発しています。

マイコンボード : Arduino UNO R4 WiFi

PC : Windows 11 Pro, VSCode, PlatformIO

**ハードウェアはArduino を使っていますが、PlatformIO 前提になっており、ArduinoIDEではビルドできません**

## 回路例

Arduino UNO R4 WiFi で使う場合の回路例です。

RC-S660/S は UART が 3.3V 系であるため、レベル変換回路が必要になります。

![image](https://www.hmpower.jp/github_img/rc_s660s_driverformcu/circuit.gif)