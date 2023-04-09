# Pico-W上でluaスクリプトを実行する

raspberrypi pico Wの無線機能を使用して、luaスクリプトを受信し実行します。

[waveshareのディスプレイ](https://www.switch-science.com/products/7331)を接続して、描画します。

描画・SDカードのアクセスについては[waveshareのコード](https://github.com/waveshare)を取り込んで```lib```以下に配置し、使用しています。

また描画に使用するフォントは自前の形式になっており、[BDFからのコンバート](https://github.com/zsuzuki/bdfconv2)を使用してコンバートしたものです。
これを```writedata.sh```によって転送します。転送には[picotool](https://github.com/raspberrypi/picotool)を使用しています。

## TODO

- lua実行中のデータ送受信
- coroutine(ウェイト機能があれば要らない？)
