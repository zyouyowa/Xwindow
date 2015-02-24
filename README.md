OS : Mac OS X Yosemite

#How to use
1,X11を起動(MacならデフォルトでApplicationフォルダに入ってる。)
2,X11のアプリケーションからxtermを開く。
3,
「gcc kadai.c -I/opt/X11/include -L/opt/X11/lib -lX11」
でコンパイル。
「./a.out」
で実行。

#How to play
'A'で左移動
'D'で右移動
クリックした方向に弾を撃つ。弾がなくなったら負け。
HPが0になっても負け。
