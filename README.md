# RC_S660_S_Arduino
SONY RC-S660/S Driver for Microcontrollers (e.g., Arduino UNO R4 in the PlatformIO Environment)

個人で買えるMIC/KS付きNFCカードリーダ RC-S660/S を MCU から制御するためのドライバです。

Version 1.0 のコマンドリファレンスマニュアルベースで作成しています。

Arduinoを想定していますが、他のマイコンでも使用したいので uart ハードウェア部をカプセル化しています。

※ .c ファイルで書いていた頃の名残で共通ヘッダファイル rcs660s_uart_hw_interface.h でクラスとインタフェースの関係な実現しています。

### 設計せずに実装していたらスパゲッティコード化しました。

このドライバを使う作品が完成したらリファクタリングしたい・・・

## 対応カード
今のところ NFC-Type B + Type A の一部機能 です。

## 環境
下記で開発しています。

マイコンボード : Arduino UNO R4 WiFi

PC : Windows 11 Pro, VSCode, PlatformIO
