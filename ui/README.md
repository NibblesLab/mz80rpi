# User Interface by Web - mz80rpi
mz80rpiでは元々LCD内蔵化改造を施したPasocomMini MZ-80Cに入れて使用することを想定していたので、大きな画面によるユーザーインタフェースを制作することが難しいと考えていました。そこで、せっかくRaspberry Piが立派なLinuxマシンであるのだから、Webサーバーを立てて、ブラウザをユーザーインタフェースとして使うことを思いつきました。Webに用意された様々なパーツでより多彩な操作が可能になると期待できます。

この文書ではその構成と使い方について説明する予定ですが、まだ制作途中ということでもあるので、当面はメモ書きとして使用することにいたします。
## DBの構成
### イメージデータ管理(PROGRAMS)
* title (タイトル)
* file (実データのファイル名)
* fkind (ROMかMZTかの種別)
* attribute (MZTの場合先頭データの属性(OBJ, BTX, BSD, その他))
* author (作者/メーカー)
* source (出典・出自(掲載誌など))
* description (説明)
* 予定
    * ジョイスティック設定情報
### エミュレータ設定(SETTINGS)
* title (設定名)
* mrom (モニタROM)
* cgrom (CG ROM)
* color (CRT表示色)
* startup (起動時に使用)
* 予定
    * 拡張機能(1200相当/80A相当)
