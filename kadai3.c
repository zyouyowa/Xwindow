#include <stdio.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>

#define MACOSX

#define BORDER 2
#define WIDTH  700
#define HIGHT 500
#define SPEED 9.8
#define LIM 1
#define MANYKEY 4
#define W 25
#define A 38
#define S 39
#define D 40
#define BULLET_SPEED 10
#define BULLET_LIMIT 100
#define P_SPEED 30
#define PI 3.141592

main(int argc,char **argv){
	Display *dpy;
	Window w;
	Window root;
	Window quit,start;
	int    screen;
	int i=0;						//for loop用変数
	unsigned long black, white;
	GC       gc;
    char moji[256];            /* 日本語メッセージが入る場所 */
	XEvent e;
    XFontSet fs;             /* XFontsetで日本語フォントを選ぶ（日） */
    char **miss,*def;
    int n_miss;              /* 変数として宣言しておく（日） */
	Colormap cmap;             /* カラーマップ構造体 */
	XColor c0[3],c1[3];
	char colorname[3][15]={"red","green","blue"};
    setlocale(LC_ALL,"");        /* 1st. lacaleをセットして（日） */
	if((dpy=XOpenDisplay(NULL))==NULL){
		printf("サーバ接続失敗\n");
		return 1;
	}
	root = DefaultRootWindow (dpy);
	screen = DefaultScreen (dpy);
	white = WhitePixel (dpy, screen);
	black = BlackPixel (dpy, screen);
	cmap = DefaultColormap(dpy,0);  /* カラーマップを得る */
	for(i=0;i<3;i++){
		XAllocNamedColor(dpy,cmap,&colorname[i][0],&c1[i],&c0[i]);  /* c1.pixel */
	}		
	w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HIGHT, BORDER, black, white); 
	quit = XCreateSimpleWindow(dpy, w, 3, 3, 40, 20, BORDER, black, white);
	start = XCreateSimpleWindow(dpy, w, 3, HIGHT/2, 110, 50, BORDER, black, white);
	gc = XCreateGC(dpy, w, 0, NULL);
    fs=XCreateFontSet(dpy,"-*-*-medium-r-normal-*-16-*",&miss,&n_miss,&def);
        /* 少なくともdpyに値がはいってからでないと駄目（日） */
	XSelectInput(dpy,w,ButtonPressMask|ButtonReleaseMask|KeyPressMask);
	XSelectInput(dpy,quit,ButtonPressMask);
	XSelectInput(dpy,start,ButtonPressMask);
	XMapWindow(dpy, w);
	XMapSubwindows(dpy,w);

	typedef struct{
		double x, y;
		int fx, fy;
		int able;
	} Bullet;
	typedef struct{
		int hp;
		float x, y;
		int power;
		int x_jb, y_jb;		//今回はx_jbしか使わない
	} Character;

	enum Mode{START, PLAY, POSE, GAMEOVER};
	enum Mode mode = PLAY;
	
	//パラメータセッティング
	double bl_x = 0, bl_y = 0;
	int counter_e2 = 0;
	Character player = { 20, 335, HIGHT-120, 10, 335, 0 };
	Character enemy = { 50, 0, 90, 10, 0, 0 };
	Bullet b[BULLET_LIMIT];
	Bullet e_b[BULLET_LIMIT];
	int counter = BULLET_LIMIT;
	int counter_e = BULLET_LIMIT-1;		//数値なんでもいい
	float sincounter = 0;
	int buttle = 0;
	int flag_end = 0;
	for (i = 0; i < BULLET_LIMIT; ++i){
		b[i].x = 0;
		b[i].y = 0;
		b[i].fx = 0;
		b[i].fy = 0;
		b[i].able = 0;
	}
	for (i = 0; i < BULLET_LIMIT; ++i){
		e_b[i].x = 0;
		e_b[i].y = 0;
		e_b[i].fx = 0;
		e_b[i].fy = 0;
		e_b[i].able = 0;
	}

	while(1){
		if(mode == PLAY){
			if(!XEventsQueued(dpy, QueuedAfterReading)){
				XSetForeground(dpy, gc, white);
				XFillRectangle(dpy,start,gc,0,0,100,100);				//サイズはなんでもいい
				XSetForeground(dpy, gc, black);
				sprintf(moji, "Exit");       /* 画面に出す文字列を変数の中に入れる */
        		XmbDrawString(dpy, quit, fs, gc, 1, 15, moji, strlen(moji) );   //文字表示
        		sprintf(moji,"Enemy:");
        		XmbDrawString(dpy, w, fs, gc, 50, 60, moji, strlen(moji) );
        		sprintf(moji,"Player:");
	  	      	XmbDrawString(dpy, w, fs, gc, 50, HIGHT - 30, moji, strlen(moji) );
	  	      	sprintf(moji,"Bullet:%d", counter + 1);
	  	      	XmbDrawString(dpy, start, fs, gc, 1, 20, moji, strlen(moji) );

				XSetForeground(dpy, gc, white);
				XFillRectangle(dpy, w, gc, enemy.x, enemy.y, 70, 55);
				sincounter += 0.01;
				if( (counter_e2 % 13) == 11 ){
					if(counter_e <= 0){
						XSetForeground(dpy, gc, white);
						XFillRectangle(dpy, w, gc, 0,150,700,215);		//残っちゃう弾を消す
						XFillRectangle(dpy, w, gc, 0,410,700,200);		//残っちゃう弾を消す
						counter_e = BULLET_LIMIT-1;
						for(i = counter_e; i > 0; i--){
							e_b[i].able=0;
						}
					}
					e_b[counter_e].able = 1;
					e_b[counter_e].fx = enemy.x;
					e_b[counter_e].fy = enemy.y;
					counter_e--;
				}
				if(sincounter > 2) sincounter = 0;
				enemy.x = sin(sincounter * PI) * 200 + 250;
				XSetForeground(dpy, gc, c1[0].pixel);
				XFillRectangle(dpy, w, gc, enemy.x, enemy.y, 70, 55);
			
				//bullets
				for(i = BULLET_LIMIT; i > 0; i--){
					if(b[i].able){
						XSetForeground(dpy, gc, white);
						XFillArc(dpy, w, gc, b[i].fx, b[i].fy, 10, 10, 0, 360*32);
						b[i].fx += BULLET_SPEED * b[i].x;
						b[i].fy += BULLET_SPEED * b[i].y;
						XSetForeground(dpy, gc, c1[2].pixel);
						XFillArc(dpy, w, gc, b[i].fx, b[i].fy, 10, 10, 0, 360*32);
						if( (enemy.x <= b[i].fx && (enemy.x + 70) >= b[i].fx) &&
							((enemy.y - 55) <= b[i].fy && enemy.y >= b[i].fy)){
							enemy.hp--;
							printf("hit\n");
						}
						if(enemy.hp <= 0){
							mode = GAMEOVER;
							buttle = 1;
							flag_end = 1;
						}
					}
				}
				//eneymy bullets
				int ex_ = player.x - enemy.x, ey_ = player.y - enemy.y;
				e_b[counter_e].fx = enemy.x + 35;
				e_b[counter_e].fy = enemy.y + 55;
				e_b[counter_e].x = ex_ / sqrt( powf(ex_, 2) + powf(ey_, 2) );
				e_b[counter_e].y = ey_ / sqrt( powf(ex_, 2) + powf(ey_, 2) );
				for(i = BULLET_LIMIT-1; i > 0; i--){
					if(e_b[i].able){
						XSetForeground(dpy, gc, white);
						XFillArc(dpy, w, gc, e_b[i].fx, e_b[i].fy, 10, 10, 360*32, 360*32);
						e_b[i].fx += BULLET_SPEED * e_b[i].x;
						e_b[i].fy += BULLET_SPEED * e_b[i].y;
						XSetForeground(dpy, gc, c1[0].pixel);
						XFillArc(dpy, w, gc, e_b[i].fx, e_b[i].fy, 10, 10, 360*32, 360*32);
						if( (player.x <= e_b[i].fx && (player.x + 15) >= e_b[i].fx) &&
							((player.y - 15) <= e_b[i].fy && player.y >= e_b[i].fy) ){
							player.hp--;
						}
						if(player.hp <= 0){
							mode = GAMEOVER;
							buttle = 0;
							flag_end = 1;
						}
					}
				}
    	    	XSetForeground(dpy, gc, white);
        		XFillRectangle(dpy, w, gc, 110, 30, 700,50);
        		XFillRectangle(dpy, w, gc, 110, HIGHT - 70, 700, 50);
	        	XSetForeground(dpy, gc, c1[0].pixel);
    	    	XFillRectangle(dpy, w, gc, 110, 30, enemy.hp * 10, 50);
				XSetForeground(dpy, gc, c1[1].pixel);
        		XFillRectangle(dpy, w, gc, 110, HIGHT - 70, player.hp * 25, 50);
				counter_e2++;
				if(counter_e2 >= 100) counter_e2 = 0;
				if(flag_end){
					XSetForeground(dpy, gc, white);
					XFillRectangle(dpy, w, gc,0,0,WIDTH,HIGHT);
					XFillRectangle(dpy, start, gc,0,0,WIDTH,HIGHT);
				}
				usleep(10000);
				XFlush(dpy);
			}else{
				XNextEvent(dpy, &e);
				switch(e.type){
					case KeyPress:
						#ifdef MACOSX
						if(e.xkey.keycode == 8) player.x -= P_SPEED;
						if(e.xkey.keycode == 10) player.x += P_SPEED;
						#else
						if(e.xkey.keycode == 38) player.x -= P_SPEED;
						if(e.xkey.keycode == 40) player.x += P_SPEED;
						#endif
						if(player.x <= 0) player.x = 0;
						if(player.x + 30 >= 700) player.x = 670;
						XSetForeground(dpy, gc, white);
						XFillArc(dpy, w, gc, player.x_jb, player.y, 30, 30, 0, 360*64);
						XSetForeground(dpy, gc, black);
						XFillArc(dpy, w, gc, player.x, player.y, 30, 30, 0, 360*64);
						player.x_jb = player.x;
						if(e.xkey.keycode == 57){
							mode = POSE;
							XSetForeground(dpy, gc, white);
							XFillRectangle(dpy, w, gc,0,0,WIDTH,HIGHT);
						}
				  		break;
					case KeyRelease:
						break;
					case ButtonPress:
						if(e.xany.window == quit){
								if(e.xbutton.button == 1)return 1;
						}
						break;
					case ButtonRelease:
						if(e.xbutton.button == 1){
							bl_x = e.xbutton.x - player.x;
							bl_y = e.xbutton.y - player.y;
							b[counter].fx = player.x + 15;
							b[counter].fy = player.y + 15;
							b[counter].able = 1;
							b[counter].x = bl_x / sqrt( powf(bl_x,2) + powf(bl_y,2) );					//方向ベクトル
							b[counter].y = bl_y / sqrt( powf(bl_x,2) + powf(bl_y,2) );
							counter--;
							printf("%d\n",counter);
							if(counter <= 0){
								mode = GAMEOVER;
								buttle = 0;
								XSetForeground(dpy, gc, white);
								XFillRectangle(dpy, w, gc,0,0,WIDTH,HIGHT);
								XFillRectangle(dpy, start, gc,0,0,WIDTH,HIGHT);
							}
						}
						break;
				}
			}
		}else if(mode == POSE){
			if(!XEventsQueued(dpy, QueuedAfterReading)){
				XSetForeground(dpy, gc, black);
				sprintf(moji, "Exit");
        		XmbDrawString(dpy, quit, fs, gc, 1, 15, moji, strlen(moji) );
        		sprintf(moji, "Posing...");
        		XmbDrawString(dpy, w, fs, gc, WIDTH/2, HIGHT/3, moji, strlen(moji) );
				usleep(10000);
				XFlush(dpy);
			}else{
				XNextEvent(dpy,&e);
				switch(e.type){
					case KeyPress:
						if(e.xkey.keycode == 57){
							mode = PLAY;
							XSetForeground(dpy, gc, white);
							XFillRectangle(dpy, w, gc,0,0,WIDTH,HIGHT);
						}
						break;
					case ButtonPress:
						if(e.xany.window == quit){
							if(e.xbutton.button == 1)return 1;
						}
						break;
				}
			}
		}else if(mode == START){
			if(!XEventsQueued(dpy, QueuedAfterReading)){
				XSetForeground(dpy, gc, black);
				sprintf(moji, "Exit");
        		XmbDrawString(dpy, quit, fs, gc, 1, 15, moji, strlen(moji) );
        		sprintf(moji, "あああ");
        		XmbDrawString(dpy, w, fs, gc, WIDTH/2, HIGHT/3, moji, strlen(moji) );
        		sprintf(moji, "START!!");
        		XmbDrawString(dpy, start, fs, gc, 20, 30, moji, strlen(moji) );
				usleep(10000);
				XFlush(dpy);
			}else{
				XNextEvent(dpy,&e);
				switch(e.type){
					case KeyPress:
						break;
					case ButtonPress:
						if(e.xany.window == quit){
							if(e.xbutton.button == 1)return 1;
						}else if(e.xany.window == start){
							if(e.xbutton.button == 1){
								mode = PLAY;
								//初期化
								bl_x = 0;
								bl_y = 0;
								counter_e2 = 0;
								Character pl = { 20, 335, HIGHT-120, 10, 335, 0 };
								Character en = { 50, 0, 90, 10, 0, 0 };
								player = pl;
								enemy = en;
								counter = BULLET_LIMIT-1;
								counter_e = BULLET_LIMIT-1;		//数値なんでもいい
								sincounter = 0;
								buttle = 0;
								flag_end = 0;
								for (i = 0; i < BULLET_LIMIT; ++i){
									b[i].x = 0;
									b[i].y = 0;
									b[i].fx = 0;
									b[i].fy = 0;
									b[i].able = 0;
								}
								for (i = 0; i < BULLET_LIMIT; ++i){
									e_b[i].x = 0;
									e_b[i].y = 0;
									e_b[i].fx = 0;
									e_b[i].fy = 0;
									e_b[i].able = 0;
								}
								XSetForeground(dpy, gc, white);
								XFillRectangle(dpy, w, gc,0,0,WIDTH,HIGHT);
								XFillRectangle(dpy, start, gc,0,0,WIDTH,HIGHT);
							}
						}
						break;
				}
			}
		}else if(mode == GAMEOVER){
			if(!XEventsQueued(dpy, QueuedAfterReading)){
				XSetForeground(dpy, gc, black);
				sprintf(moji, "Exit");
        		XmbDrawString(dpy, quit, fs, gc, 1, 15, moji, strlen(moji) );
        		sprintf(moji, "Back to Start");
        		XmbDrawString(dpy, start, fs, gc, 1, 25, moji, strlen(moji) );
        		if(buttle) sprintf(moji, "You Win!");
        		else sprintf(moji, "You Lose...");
        		XmbDrawString(dpy, w, fs, gc, WIDTH/3, HIGHT/3, moji, strlen(moji) );
				usleep(10000);
				XFlush(dpy);
			}else{
				XNextEvent(dpy,&e);
				switch(e.type){
					case KeyPress:
						break;
					case ButtonPress:
						if(e.xany.window == quit){
							if(e.xbutton.button == 1) return 1;
						}else if(e.xany.window == start){
							mode = START;
							XSetForeground(dpy, gc, white);
							XFillRectangle(dpy, w, gc,0,0,WIDTH,HIGHT);
							XFillRectangle(dpy, start, gc,0,0,WIDTH,HIGHT);
						}
						break;
				}
			}
		}


		//XDrawString(dpy, w, gc, WIDTH / 2, HIGHT / 2,here, strlen(here));
	}
}

