/*
	���ߣ�������
	��ʦ��bilibili
	��ʼʱ�䣺2024��1��17��
	�깤ʱ�䣺2024��2��2��
	������־��
		1����������Ŀ
		2�������ز�
		3��ʵ���ʼ����Ϸ����
		4��ʵ����Ϸ�����Ĺ�����
		5��ʵ�ֹ������е�ֲ�￨��
		6��ʵ�����ϵ���������
		7��ʵ��ֲ�����ֲ
		8��������ʬ��ʵ�ֽ�ʬ������
		9��ʵ���㶹�ķ���
		10��ʵ���㶹�ӵ��ͽ�ʬ����ײ
		11��ʵ�ֽ�ʬ��ֲ��ĳ���
		12��������ʬ��Ѫ����Ѫ����Ϊ0ʱʵ������
		13������ֲ���Ѫ����Ѫ����Ϊ0ʱֲ����ʧ
		14��ʵ�����տ���������
		15���ж���Ϸ�Ľ�����������ʬ���ݺͽ�ʬȫ������

	��Ϸ˵����
		1���ܽ�ʬ��Ϊ10��ȫ��������Ϸ��ʤ
		2����������䵽�����ٵ���Ż��ռ��ɹ�������
*/

#include<stdio.h>
#include<graphics.h>
#include"tools.h"/*ͷ�ļ���ʵ��ͼƬ���ϴ�*/
#include"vector2.h"/*ͷ�ļ������������ߣ�����ʵ�����տ���������ĳ���*/
#include<time.h>
#include<math.h>
#include<mmsystem.h>
#pragma comment(lib, "winmm.lib")/*Ԥ����ָ����ڲ�������*/
//�����ĳ��ȺͿ��
#define WIN_WIDTH 900
#define WIN_HEIGHT 600
#define ZM_MAX 10/*��ʬ������*/

enum{WAN_DOU,XIANG_RI_KUI,ZHI_WU_COUNT};/*ֲ������*/

IMAGE imgBg;//��ʾ����ͼƬ
IMAGE imgBar;/*��ʾ�˵���*/
IMAGE imgCards[ZHI_WU_COUNT];/*��ʾֲ�￨Ƭ*/
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];/*�����γ�ֲ��Ķ�̬Ч��*/

enum {GOING,WIN,FAIL};/*��Ϸ��״̬*/
int killCount;//�Ѿ�ɱ����ʬ�ĸ���
int zmCount;//��ǰ���ֽ�ʬ�ĸ���
int gameStatus;/*��Ϸ��״̬*/


int curX, curY;//��ǰѡ�е�ֲ����ƶ������е�λ��
int curZhiWu;//0:û��ѡ��1��ѡ���˵�һ��ֲ�����������

struct zhiwu {
	int type;//0:û��ѡ��1��ѡ���˵�һ��ֲ�����������
	int frameIndex;//����֡�����

	bool catched;//�Ƿ񱻽�ʬ����
	int deadTime;//����������

	int timer;/*���տ���������ļ�ʱ��*/
	int x, y;/*����*/
};

struct zhiwu map[3][9];

enum {SUNSHINE_DOWN,SUNSHINE_GROUND,SUNSHINE_COLLECT,SUNSHINE_PRODUCT};/*�����״̬*/

struct sunshineBall {
	int x, y;//��������Ʈ������е�����λ�ã�x���䣩
	int frameIndex;//��ǰ��ʾ��ͼƬ֡�����
	int destY;//Ʈ���Ŀ��λ�õ�y����
	bool used;//�Ƿ���ʹ��
	int timer;/*����������䵽���ϵ���������ʧ��ʱ��*/

	//float xoff;
	//float yoff;

	float t;//���������ߵ�ʱ���
	vector2 p1, p2, p3, p4;/*���������߿��Ƶ�*/
	vector2 pCur;//��ǰʱ���������λ��
	float speed;
	int status;/*�����״̬*/
};
struct sunshineBall balls[10];
//��������ͼƬ
IMAGE imgSunshineBall[29];
int sunshine;

//������ʬ��
struct zm {
	int x, y;
	int frameIndex;//����֡�����
	bool used;/*�жϽ�ʬ�Ƿ�ʹ��*/
	int speed;
	int row;
	int blood;
	bool dead;/*�жϽ�ʬ�Ƿ�����*/

	bool eating;/*�жϽ�ʬ�Ƿ��ڳ�ֲ���״̬*/
};
struct zm zms[10];
//���뽩ʬͼƬ
IMAGE imgZM[22];
IMAGE imgZMDead[20];
IMAGE imgZMEat[21];

//�ӵ�����������
struct bullet {
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;//�Ƿ�����ը
	int frameIndex;//֡���
};
struct bullet bullets[30];
//�����ӵ�ͼƬ
IMAGE imgBulletNormal;
IMAGE imgBullBlast[4];

//�ж�����ļ��Ƿ����
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

//��Ϸ����
void gameInit() {
	//������Ϸ����ͼƬ
	//���ַ����޸�Ϊ�����ֽ��ַ�����
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar, "res/bar5.png");

	/*��ʼ��*/
	memset(imgZhiWu, 0, sizeof(imgZhiWu));
	memset(map, 0, sizeof(map));

	killCount = 0;
	zmCount = 0;
	gameStatus = GOING;

	//��ʼ��ֲ�￨��
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		//����ֲ�￨�Ƶ��ļ���
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);

		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);
			//���ж�����ļ��Ƿ����
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

	//�����������
	srand(time(NULL));

	//������Ϸ��ͼ�δ���
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);

	//��������
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;//�����Ч��
	settextstyle(&f);
	setbkmode(TRANSPARENT);//���ñ���ģʽ
	setcolor(BLACK);

	//��ʼ����ʬ����
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZM[i], name);
	}

	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));

	//��ʼ���㶹�ӵ���֡ͼƬ����
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

//������ʬͼƬ��ʵ�ֽ�ʬ״̬���ж�
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

//ʵʱ��������
void updateWindow() {
	BeginBatchDraw();//��ʼ����

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
	//��Ⱦ �϶������е�ֲ��
	if (curZhiWu > 0) {
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}

	drawSunshines();

	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(276, 67, scoreText);//�������

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

	EndBatchDraw();//����˫����
}

//ʵ��������ռ�
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
				mciSendString("play res/sunshine.mp3", 0, 0, 0);/*ʵ���ռ�����ʱ������*/
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

//ʵ���û����ʱ���з���
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

//ʵ������Ĵ���
void creatSunshine() {
	static int count = 0;
	static int fre = 400;
	count++;
	if (count >= fre) {
		fre = 200 + rand() % 200;
		count = 0;

		//���������ȡһ������ʹ�õ�
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

	//���տ���������
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

//ʵ������ĸ���
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

//ʵ�ֽ�ʬ�Ĵ���
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

//ʵ�ֽ�ʬ״̬�ĸ���
void updateZM() {
	int zmMax = sizeof(zms) / sizeof(zms[0]);

	static int count = 0;
	count++;
	if (count > 2) {
		count = 0;
		//���½�ʬ��λ��
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
					/*�����䶯��֡������ȷ����0��20֮��ѭ��*/
				}
				else {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
					/*�����䶯��֡������ȷ����0��21֮��ѭ��*/
				}
			}
		}
	}
}

//�����ӵ�
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

//�����ӵ���״̬
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

//���ӵ��ͽ�ʬ����ײ���
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

//�Խ�ʬ��ֲ��ļ��
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
	checkBullet2Zm();//�ӵ��ͽ�ʬ����ײ���
	checkZm2ZhiWu();//��ʬ��ֲ�����ײ���
}

//ʵʱ������Ϸ����
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

	creatSunshine();//��������
	updateSunshine();//���������״̬

	creatZM();//������ʬ
	updateZM();//���½�ʬ��״̬
	shoot();//�����㶹�ӵ�
	updateBullets();//�����㶹�ӵ�

	collisionCheck();//ʵ���㶹�ӵ��ͽ�ʬ����ײ���
}
 
//��Ϸ�����Ĵ���
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

//�����Ϸ�Ƿ����
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