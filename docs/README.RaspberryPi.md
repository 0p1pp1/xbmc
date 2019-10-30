![Kodi Logo](resources/banner_slim.png)

# rootfs(のコピー)を使ったRaspberry Pi4用クロスビルドの手順

公式の手順は、Kodiのすべての依存ライブラリをビルドする上に、
ビルドに必要なnativeのツールもすべてビルドするようになっているため
時間・ディスクスペースを非常に多く消費する。
またクロスビルドの設定では非常にエラーが起きやすくなっている。
そこで、依存ライブラリはできるだけrpi4(raspbian Buster)にインストールしたパッケージを使い、
nativeツールはビルドホスト上のものを使用する場合の手順を以下に述べる。

# WARNING!!

Raspian BusterでのKodiのビルド・実行には、使用するGL Driver他いくつかの注意が必要。\
参照: [\[Guide\] Kodi on Raspbian Buster](https://www.raspberrypi.org/forums/viewtopic.php?f=66&t=251645)

## ルートファイルシステムのコピーの用意

1. 依存ライブラリをrpiに事前にインストール
- 面倒な場合は、ディストリビューションのkodiパッケージの情報を調べて、
  ビルドに必要なdevパッケージをすべてインストールすればよい。
- 依存ライブラリはdocs/README.{Linux, Ubuntsu}.mdに挙げられているが、
  オプション扱いのものが含まれている上、ビルドホストで必要なパッケージも含まれている。
  必須のライブラリは、./CMakeLists.txtの required\_deps 及びcmake/platform/linux/gbm.cmake
  に挙げられている。具体的なライブラリ名はcmake/modules/FindXXXX.cmakeを参照。
- 必須ライブラリ自体の依存ライブラリも必要となること、-devパッケージが必要であることに注意。
- 必須ライブラリの内、ffmpeg, crossguid, libfmt, fstrcmp, RapidJSON, flatbuffer, spdlogは
  インストール不要。(後のビルド処理内でスタティックライブラリとして自動的にビルド・リンクされる)
- その他の依存ライブラリも、後ほどスタティックライブラリとしてビルドすることもできるので、
  Kodiだけのためにここで必ずしもインストールしておかなくてもよい。
- ただし、libinputはうまくクロスビルドできないのでこの段階でインストールしておくと良い。
- xkbcommonも実行時にエラーが出るので、この段階でインストールしておく
- ./CMakeList.txtでoptional\_depsとして挙げられているライブラリについては、
  自分が使用する機能に関係なければインストールしなくても良い。
- KodiのVideo再生の機能として、ISDBのMULTI2暗号化されたままのファイル等も再生したい場合は、
  libdemulti2 と {libpcscliteかlibyakisoba}のインストールも必要。\
  NOTE: tvheadendからストリーミングして視聴する分には不要。(tvheadend側で復号するため)

2. rpiルートファイルシステムのコピー
- ファイルシステム全体をコピーする必要はなく、/lib, /usr/{include,lib}があればOK。
以下では、そのパスを/opt/rpi/rootfsとする。

## クロスビルド環境の用意

1. コピーしたファイルシステムで、絶対パス指定のシンボリックリンクを修正 \
(のちのconfigure/ビルド時にエラーになるため)
```
$ find /opt/rpi/rootfs -type l -lname /\* -execdir bash -c \
      'ln -sf /opt/rpi/rootfs$(readlink {}) {}' \;
```
2. pkgconfigファイルの修正
```
$ sed -i --follow-symlinks 's|^prefix=/usr|prefix=/opt/rpi/rootfs/usr|' \
      /opt/rpi/rootfs/usr/lib/{,arm-linux-gnueabihf/}pkgconfig/*.pc
```
3. リンカースクリプト内の絶対パスの修正
- /opt/rpi/rootfs/usr/lib/arm-linux-gnueabihf/libc.soを以下のように変換する。
```
GROUP ( /lib/arm-linux-gnueabihf/libc.so.6 /usr/lib/arm-linux-gnueabihf/libc_nonshared.a  AS_NEEDED ( /lib/arm-linux-gnueabihf/ld-linux-armhf.so.3 ) )
↓
GROUP ( ../../../lib/arm-linux-gnueabihf/libc.so.6 ./libc_nonshared.a  AS_NEEDED ( ../../../lib/arm-linux-gnueabihf/ld-linux-armhf.so.3 ) )
```
4. クロスビルドのtool chainの用意 \
簡略化のため、ここではプリビルトされたツールを使用
- `https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads` \
より、"AArch32 target with hard float (arm-linux-gnueabihf)"のtarballをダウンロードし、展開

あるいは、
- `https://github.com/abhiTronix/raspberry-pi-cross-compilers`から好みのgccをダウンロードし、展開\
 ここでは`cross-gcc-8.3.0-pi_3+.tar.gz`を利用する
```
$ mkdir /opt/rpi/tools; cd /opt/rpi/tools
$ tar xvf cross-gcc-8.3.0-pi_3+.tar.xz
$ cd /opt/rpi/tools/cross-pi-gcc-8.3.0-2/arm-linux-gnueabihf
$ ln -s /opt/rpi/rootfs sysroot
$ # toolchainのlibcでなくrootfsのlibcを使うためのハック
$ mv libc/usr/lib libc/usr/lib.org
$ ln -s /opt/rpi/rootfs/usr/lib/arm-linux-gnueabihf libc/usr/lib
$ mv libc/lib libc/lib.org
$ ln -s /opt/rpi/rootfs/lib/arm-linux-gnueabihf libc/lib
$ mv libc/usr/include libc/usr/include.org
$ ln -s /opt/rpi/rootfs/usr/include libc/usr/include
```

## クロスビルドする依存ライブラリとnativeツールの格納先の準備

1. 依存ライブラリとネイティブツールのための"ビルドディレクトリ"を作成
```
$ mkdir -p /opt/rpi/kodi-rpi
```
2. ネイティブツールはビルドホストにインストール済みのものを利用するための細工
```
$ cd /opt/rpi/kodi-rpi
$ mkdir native-work; mkdir -p native-upper/{bin,include,lib,share}
$ mkdir x86_64-linux-gnu-native
$ sudo mount -t overlay -o lowerdir=/usr,upperdir=native-upper,workdir=native-work \
    overlay x86_64-linux-gnu-native
```
3. Kodiのソースの用意
```
$ git clone https://github.com/0p1pp1/xbmc kodi
```
4. 本体および不足している依存ライブラリをクロスビルドするためのconfigure
```
$ cd /opt/rpi/kodi/tools/depends
$ ./bootstrap
$ ./configure --host=arm-linux-gnueabihf --prefix=/opt/rpi/kodi-rpi \
      --with-cpu=cortex-a53 --with-toolchain=/opt/rpi/tools/cross-pi-gcc-8.3.0-2 \
      --disable-debug --with-rpi-rootfs=/opt/rpi/rootfs
```

## ビルド

1. 必要なネイティブツールのビルド
```
$ cd /opt/rpi/kodi/tools/depends;
$ make -C native/TexturePacker
$ make -C native/JsonSchemaBuilder
```
2. 不足しているライブラリをクロスビルド(スタティックライブラリとして)
```
$ cd /opt/rpi/kodi/tools/depends
$ make -C target/XXX
$ # "XXX"はkodiの依存ライブラリで、/opt/rpi/rootfs下に無く、自動で内部ビルドできないもの。
$ # (ffmpegやcrossguidなどは自動で内部ビルドできる。)
....
```
XXXに必要な依存ライブラリが不足していると、上記ステップは(configure中に)エラーで停止する。
不足ライブラリは、エラー出力(の末尾付近)にCould NOT find YYY (...)のようなメッセージとして示されるので、
tools/depends/target/{libyyy, yyy}等を探し、同様に`make -C target/libyyy`でビルドする。
それが成功したら、`make -C target/XXX distclean`してから`make -C target/XXX`でやり直す。

なお、始めは本ステップを飛ばして、先に3.(`cmakebuildsys`)を実行し、
足りないライブラリを探して本ステップに戻ってインストールするという手順により、
ここでビルドする依存ライブラリを最小限に抑えられる。

3. Kodi本体のクロスビルド
```
$ # 本体のconfigure
$ make -C target/cmakebuildsys CMAKE_EXTRA_ARGUMENTS='-DAPP_RENDER_SYSTEM=gles \
  -DCORE_PLATFORM_NAME=gbm [-DFFMPEG_EXTRA_FLAGS=--enable-libdemulti2]'
```
([]内はDEMULTI2復号しながらの再生機能が必要な場合のみ)

必須ライブラリが見つからないとエラーで停止するので、上記2.の手順でビルド、インストールした後、
再度`make -C target/cmakebuildsys`を実行する。これをエラーが出なくなるまで繰り返す。
```
$ cd /opt/rpi/kodi/build # 本体のビルドディレクトリ
$ cmake --build .
```
4. 追加のプラグインのビルド
```
$ cd /opt/rpi/kodi/tools/depend/target/binary-addons
$ make ADDONS="pvr.hts" PREFIX=/opt/rpi/kodi/build/addons EXTRA_CMAKE_ARGS=-DPACKAGE_ZIP=ON
```

5. インストール・実行例
```
$ rsync -avzh /opt/rpi/kodi/build/{kodi-gbm, addons, media, system, userdata} \
  pi@raspberry-pi:/home/pi/kodi

$ # raspberry-piのVTにloginして
$ ./kodi/kodi-gbm --standalone
```

# Raspberry Pi build guide

The Raspberry Pi platform has been merged into the GBM build. See [README.Linux.md](README.Linux.md).
