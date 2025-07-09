/*
	作者：裴振宇
	老师：bilibili
	开始时间：2024年1月17日
	完工时间：2024年2月2日
	开发日志：
		1、创建新项目
		2、导入素材
		3、实现最开始的游戏场景
		4、实现游戏顶部的工具栏
		5、实现工具栏中的植物卡牌
		6、实现天上掉落阳光球
		7、实现植物的种植
		8、创建僵尸，实现僵尸的行走
		9、实现豌豆的发射
		10、实现豌豆子弹和僵尸的碰撞
		11、实现僵尸吃植物的场景
		12、创建僵尸的血量，血量降为0时实现死亡
		13、创建植物的血量，血量降为0时植物消失
		14、实现向日葵生产阳光
		15、判断游戏的结束，包括僵尸进屋和僵尸全部死亡

	游戏说明：
		1、总僵尸数为10，全部打死游戏获胜
		2、阳光必须落到地上再点击才会收集成功！！！
*/

#include<stdio.h>
#include<graphics.h>
#include"tools.h"/*头文件：实现图片的上传*/
#include"vector2.h"/*头文件：贝塞尔曲线，用于实现向日葵生产阳光的场景*/
#include<time.h>
#include<math.h>
#include<mmsystem.h>
#pragma comment(lib, "winmm.lib")/*预编译指令，用于插入声音*/
//场景的长度和宽度
#define WIN_WIDTH 900
#define WIN_HEIGHT 600
#define ZM_MAX 10/*僵尸的总数*/

enum{WAN_DOU,XIANG_RI_KUI,ZHI_WU_COUNT};/*植物种类*/

IMAGE imgBg;//表示背景图片
IMAGE imgBar;/*表示菜单栏*/
IMAGE imgCards[ZHI_WU_COUNT];/*表示植物卡片*/
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];/*用于形成植物的动态效果*/

enum {GOING,WIN,FAIL};/*游戏的状态*/
int killCount;//已经杀掉僵尸的个数
int zmCount;//当前出现僵尸的个数
int gameStatus;/*游戏的状态*/


int curX, curY;//当前选中的植物，在移动过程中的位置
int curZhiWu;//0:没有选择，1：选择了第一种植物······

struct zhiwu {
	int type;//0:没有选择，1：选择了第一种植物······
	int frameIndex;//序列帧的序号

	bool catched;//是否被僵尸捕获
	int deadTime;//死亡计数器

	int timer;/*向日葵生产阳光的计时器*/
	int x, y;/*坐标*/
};

struct zhiwu map[3][9];

enum {SUNSHINE_DOWN,SUNSHINE_GROUND,SUNSHINE_COLLECT,SUNSHINE_PRODUCT};/*阳光的状态*/

struct sunshineBall {
	int x, y;//阳光球在飘落过程中的坐标位置（x不变）
	int frameIndex;//当前显示的图片帧的序号
	int destY;//飘落的目标位置的y坐标
	bool used;//是否在使用
	int timer;/*从阳光球掉落到地上到阳光球消失的时间*/

	//float xoff;
	//float yoff;

	float t;//贝塞尔曲线的时间点
	vector2 p1, p2, p3, p4;/*贝塞尔曲线控制点*/
	vector2 pCur;//当前时刻阳光球的位置
	float speed;
	int status;/*阳光的状态*/
};
struct sunshineBall balls[10];
//导入阳光图片
IMAGE imgSunshineBall[29];
int sunshine;

//创建僵尸池
struct zm {
	int x, y;
	int frameIndex;//序列帧的序号
	bool used;/*判断僵尸是否使用*/
	int speed;
	int row;
	int blood;
	bool dead;/*判断僵尸是否死亡*/

	bool eating;/*判断僵尸是否处于吃植物的状态*/
};
struct zm zms[10];
//导入僵尸图片
IMAGE imgZM[22];
IMAGE imgZMDead[20];
IMAGE imgZMEat[21];

//子弹的数据类型
struct bullet {
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;//是否发生爆炸
	int frameIndex;//帧序号
};
struct bullet bullets[30];
//导入子弹图片
IMAGE imgBulletNormal;
IMAGE imgBullBlast[4];

//判断这个文件是否存在
bool fileExist(const char* name) {
	FILE* fp = fopen(name, "r");
	if (fp == NULL) {
		return false;
	}
	else {
		fclose(fp);
		return true;
	}
}

//游戏场景
void gameInit() {
	//加载游戏背景图片
	//把字符集修改为“多字节字符集”
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar, "res/bar5.png");

	/*初始化*/
	memset(imgZhiWu, 0, sizeof(imgZhiWu));
	memset(map, 0, sizeof(map));

	killCount = 0;
	zmCount = 0;
	gameStatus = GOING;

	//初始化植物卡牌
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		//生成植物卡牌的文件名
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);

		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);
			//先判断这个文件是否存在
			if (fileExist(name)) {
				imgZhiWu[i][j] = new IMAGE;
				loadimage(imgZhiWu[i][j], name);
			}
			else {
				break;
			}
		}
	}

	curZhiWu = 0;
	sunshine = 50;

	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	//配置随机种子
	srand(time(NULL));

	//创建游戏的图形窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);

	//设置字体
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;//抗锯齿效果
	settextstyle(&f);
	setbkmode(TRANSPARENT);//设置背景模式
	setcolor(BLACK);

	//初始化僵尸数据
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZM[i], name);
	}

	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));

	//初始化豌豆子弹的帧图片数组
	loadimage(&imgBullBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++) {
		float k = (i + 1) * 0.2;
		loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png",
			imgBullBlast[3].getwidth() * k,
			imgBullBlast[3].getheight() * k, true);
	}

	for (int i = 0; i < 20; i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZMDead[i], name);
	}

	for (int i = 0; i < 21; i++) {
		sprintf_s(name, "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZMEat[i], name);
	}
}

//创建僵尸图片，实现僵尸状态的判断
void drawZM() {
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used) {
			IMAGE* img = NULL;
			if (zms[i].dead) img = imgZMDead;
			else if (zms[i].eating)img = imgZMEat;
			else img = imgZM;

			img += zms[i].frameIndex;

			putimagePNG(zms[i].x, zms[i].y - img->getheight(),img);
		}
	}
}

void drawSunshines() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used ) {
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y, img);
		}
	}
}

//实时更新数据
void updateWindow() {
	BeginBatchDraw();//开始缓冲

	putimage(0, 0, &imgBg);
	putimagePNG(250,0, &imgBar);

	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		int x = 338 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				int zhiWuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				putimagePNG(map[i][j].x, map[i][j].y, imgZhiWu[zhiWuType][index]);
			}
		}
	}
	//渲染 拖动过程中的植物
	if (curZhiWu > 0) {
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}

	drawSunshines();

	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(276, 67, scoreText);//输出分数

	drawZM();
	
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++) {
		if (bullets[i].used) {
			if (bullets[i].blast) {
				IMAGE* img = &imgBullBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else {
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}

	EndBatchDraw();//结束双缓冲
}

//实现阳光的收集
void collectSunshine(ExMessage* msg) {
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgSunshineBall[0].getwidth();
	int h = imgSunshineBall[0].getheight();
	for (int i = 0; i < count; i++) {
		if (balls[i].used) {
			int x = balls[i].pCur.x;
			int y = balls[i].pCur.y;
			if (msg->x > x && msg->x<x + w &&
				msg->y>y && msg->y < y + h) {
				balls[i].status = SUNSHINE_COLLECT;
				mciSendString("play res/sunshine.mp3", 0, 0, 0);/*实现收集阳光时的声音*/
				balls[i].p1 = balls[i].pCur;
				balls[i].p4 = vector2(262, 0);
				balls[i].t = 0;
				float distance = dis(balls[i].p1 - balls[i].p4);
				float off = 8;
				balls[i].speed = 1.0 / (distance / 8);
				break;
			}
		}
	}
}

//实现用户点击时进行反馈
void userClick() {
	ExMessage msg;
	static int status = 0;
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96) {
				int index = (msg.x - 338) / 65;
				
				if (sunshine >= 100 && index == 0) {
					status = 1;
					curZhiWu = index + 1;
				}
				else if (sunshine >= 50 && index == 1) {
					status = 1;
					curZhiWu = index + 1;
				}
			}
			else {
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE&&status==1) {
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP) {
			if (msg.x > 256 && msg.y > 179 && msg.y < 489) {
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;

				if (map[row][col].type == 0) {
					map[row][col].type = curZhiWu;
					if (map[row][col].type == 1 ) {
						sunshine -= 100;

					}
					else if (map[row][col].type == 2 ) {
						sunshine -= 50;

					}

				}

					map[row][col].frameIndex = 0;

					map[row][col].x = 256 + 81 * col;
					map[row][col].y = 179 + 102 * row + 14;
				

			}

			curZhiWu = 0;
			status = 0;
		}
	}
}

//实现阳光的创建
void creatSunshine() {
	static int count = 0;
	static int fre = 400;
	count++;
	if (count >= fre) {
		fre = 200 + rand() % 200;
		count = 0;

		//从阳光池中取一个可以使用的
		int ballMax = sizeof(balls) / sizeof(balls[0]);
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax)return;

		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].timer = 0;


		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		balls[i].p1 = vector2(260 + rand() % (900 - 260), 60);
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);
		int off = 2;
		float distance = balls[i].p4.y - balls[i].p1.y;
		balls[i].speed = 1.0 / (distance / off);
	}

	//向日葵生产阳光
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == XIANG_RI_KUI + 1) {
				map[i][j].timer++;
				if (map[i][j].timer > 200) {
					map[i][j].timer = 0;

					int k;
					for (k = 0; k < ballMax && balls[k].used; k++);
					if (k >= ballMax) return;

					balls[k].used = true;
					balls[k].p1 = vector2(map[i][j].x, map[i][j].y);
					int w = (100 + rand() % 50)*(rand() % 2 ? 1 : -1);
					balls[k].p4 = vector2(map[i][j].x + w,
						map[i][j].y + imgZhiWu[XIANG_RI_KUI][0]->getheight() - 
						imgSunshineBall[0].getheight());
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);
					balls[k].status = SUNSHINE_PRODUCT;
					balls[k].speed = 0.05;
					balls[k].t = 0;
				}
			}
		}
	}

}

//实现阳光的更新
void updateSunshine() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			if (balls[i].status == SUNSHINE_DOWN) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND) {
				balls[i].timer++;
				if (balls[i].timer > 100) {
					balls[i].used = false;
					balls[i].timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_COLLECT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->timer > 1) {
					sun->used = false;
					sunshine += 25;
				}
			}
			else if (balls[i].status == SUNSHINE_PRODUCT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}



			//balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			//if (balls[i].timer == 0) {
			//	balls[i].y += 2;
			//}
			//if (balls[i].y >= balls[i].destY) {
			//	balls[i].timer++;
			//	if (balls[i].timer > 100) {
			//		balls[i].used = false;
			//	}
			//}
		}
		//else if (balls[i].xoff) {
		//	float destY = 0;
		//	float destX = 262;
		//	float angle = atan(balls[i].y - destY) / (balls[i].x - destX);
		//	balls[i].xoff = 4 * cos(angle);
		//	balls[i].yoff = 4 * sin(angle);

		//	balls[i].x -= balls[i].xoff;
		//	balls[i].y -= balls[i].yoff;
		//	if (balls[i].y < 0 || balls[i].x < 262) {
		//		balls[i].xoff = 0;
		//		balls[i].yoff = 0;
		//		sunshine += 25;
		//	}
		//}
	}
	
}

//实现僵尸的创建
void creatZM() {
	if (zmCount >= ZM_MAX) return;

	static int zmFre = 200;
	static int count = 0;
	count++;
	if (count > zmFre) {
		count = 0;
		zmFre = rand() % 200 + 300;

		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);
		for (i = 0; i < zmMax && zms[i].used; i++);
			if (i < zmMax) {
				memset(&zms[i], 0, sizeof(zms[i]));
				zms[i].used = true;
				zms[i].x = WIN_WIDTH;
				zms[i].row = rand() % 3;
				zms[i].y = 172 + 100 * (zms[i].row + 1);
				zms[i].speed = 1;
				zms[i].blood = 100;
				zms[i].dead = false;
				zmCount++;
			}
		
	}
}

//实现僵尸状态的更新
void updateZM() {
	int zmMax = sizeof(zms) / sizeof(zms[0]);

	static int count = 0;
	count++;
	if (count > 2) {
		count = 0;
		//更新僵尸的位置
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed;
				if (zms[i].x < 170) {
					gameStatus = FAIL;
				}
			}
		}
	}
	static int count2= 0;
	count2++;
	if (count2 > 4) {
		count2 = 0;
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				if (zms[i].dead) {
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20) {
						zms[i].used = false;
						killCount++;
						if (killCount == ZM_MAX) {
							gameStatus = WIN;
						}
					}
				}
				else if (zms[i].eating) {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21; 
					/*更新其动画帧索引，确保在0到20之间循环*/
				}
				else {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
					/*更新其动画帧索引，确保在0到21之间循环*/
				}
			}
		}
	}
}

//创建子弹
void shoot() {
	int lines[3] = { 0 };
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX = WIN_WIDTH - imgZM[0].getwidth();
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used && zms[i].x < dangerX) {
			lines[zms[i].row] = 1;
		}
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == WAN_DOU + 1 && lines[i]) {
				static int count = 0;
				count++;
				if (count > 20) {
					count = 0;

					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);
					if (k < bulletMax) {
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 6;

						bullets[k].blast = false;
						bullets[k].frameIndex = 0;

						int zwX = 256 + j * 81;
						int zwY = 179 + i * 102 + 14;
						bullets[k].x = zwX + imgZhiWu[map[i][j].type - 1][0]->getwidth()-10;
						bullets[k].y = zwY + 5;
					}
				}
			}
		}
	}
}

//更新子弹的状态
void updateBullets() {
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++) {
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {
				bullets[i].used = false;
			}

			if (bullets[i].blast) {
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4) {
					bullets[i].used = false;
				}
			}
		}
	}
}

//对子弹和僵尸的碰撞检测
void checkBullet2Zm() {
	int bCount = sizeof(bullets) / sizeof(bullets[0]);
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < bCount; i++) {
		if (bullets[i].used == false || bullets[i].blast)continue;

		for (int k = 0; k < zCount; k++) {
			if (zms[k].used == false)continue;
			int x1 = zms[k].x + 80;
			int x2 = zms[k].x + 110;
			int x = bullets[i].x;
			if (zms[k].dead == false && bullets[i].row == zms[k].row && x > x1 && x < x2) {
				zms[k].blood -= 10;
				bullets[i].blast = true;
				bullets[i].speed = 0;

				if (zms[k].blood <= 0) {
					zms[k].dead = true;
					zms[k].speed = 0;
					zms[k].frameIndex = 0;
				}
				break;
			}

		}
	}
}

//对僵尸吃植物的检测
void checkZm2ZhiWu() {
	int count = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < count; i++) {
		if (zms[i].dead) continue;

		int row = zms[i].row;
		for (int k = 0; k < 9; k++) {
			if (map[row][k].type == 0) continue;

			int zhiWuX = 256 + 81 * k;
			int x1 = zhiWuX + 10;
			int x2 = zhiWuX + 60;
			int x3 = zms[i].x + 80;
			if (x3 > x1 && x3 < x2) {
				if (map[row][k].catched) {
					map[row][k].deadTime++;
					if (map[row][k].deadTime > 100) {
						map[row][k].deadTime = 0;
						map[row][k].type = 0;
						zms[i].eating = false;
						zms[i].frameIndex = 0;
						zms[i].speed = 1;
					}
				}
				else {
					map[row][k].catched = true;
					map[row][k].deadTime = 0;
					zms[i].eating = true;
					zms[i].speed = 0;
					zms[i].frameIndex = 0;
				}
			}
		}
	}
}

void collisionCheck() {
	checkBullet2Zm();//子弹和僵尸的碰撞检测
	checkZm2ZhiWu();//僵尸和植物的碰撞检测
}

//实时更新游戏数据
void updateGame() {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				map[i][j].frameIndex++;
				int zhiWuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				if (imgZhiWu[zhiWuType][index] == NULL) {
					map[i][j].frameIndex = 0;
				}
			}
		}
	}

	creatSunshine();//创建阳光
	updateSunshine();//更新阳光的状态

	creatZM();//创建僵尸
	updateZM();//更新僵尸的状态
	shoot();//发射豌豆子弹
	updateBullets();//更新豌豆子弹

	collisionCheck();//实现豌豆子弹和僵尸的碰撞检测
}
 
//游戏场景的创建
void startUI() {
	IMAGE imgBg,imgMenu1,imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");
	int flag = 0;

	while (1) {
		BeginBatchDraw();

		putimage(0, 0, &imgBg);
		putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);

		ExMessage msg;
		if(peekmessage(&msg)){
			if (msg.message == WM_LBUTTONDOWN
				&&msg.x>474&&msg.x<474+300
				&&msg.y>75&&msg.y<75+140) {
				flag = 1;
			}
			else if (msg.message == WM_LBUTTONUP&&flag) {
				return;
			}
		}

		EndBatchDraw();
	}
}

//检查游戏是否结束
bool checkOver() {
	int vet = false;
	if (gameStatus == WIN) {
		Sleep(2000);
		loadimage(0, "res/win2.png");
		vet = true;
	}
	else if (gameStatus == FAIL) {
		Sleep(2000);
		loadimage(0, "res/fail2.png");
		vet = true;
	}
	return vet;
}

int main(void) {
	gameInit();

	startUI();

	int timer = 0;
	int flag = true;
	while (1) {
		userClick();
		timer += getDelay();
		if (timer > 20) {
			flag = true;
			timer = 0;
		}
		
		if (flag) {
			flag = false;
			updateWindow();
			updateGame();
			if (checkOver())break;
		}

	}

	system("pause");
	return 0;
}