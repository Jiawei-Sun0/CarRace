M5CamStreaming
------------
- M5 Unit V2 WiFiカメラのストリーミング サンプルコード

## コンパイル（ビルド）
- Visual Studio 2022 C++ を使用
- OpenCV 4.3.0 with Gstreamer が必要
  - ダウンロード https://we.tl/t-bQhVfOmzbX
- 解凍したフォルダを以下のように配置する
  - d:\SDK\OpenCV4.3.0_Gstreamer
  - d:\SDK\bin
- 'd:\SDK\bin' をPATH環境変数に追加する  
（変更を有効にするにはwindows再起動が必要）

## M5 Unit V2とPCの通信
- 有線接続
  - M5 Unit V2とPCをUSBケーブルで直結する
- WiFi接続
  - ssid: M5UV2_* (*はMY_ID) passkey: 12345678
- アクセス
  - ホスト名 unitv2.py  http://unitv2.py/
  - IPアドレス 10.254.239.1  http://10.254.239.1/

# 実行
- 実行するとWiFiカメラの映像が表示される
- [ESC]キーで終了
