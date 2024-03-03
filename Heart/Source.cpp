#include<iostream>
using namespace std;
#include <windows.h>		
#include<GL/glut.h>
#include<GL/GL.h>
#include<math.h>
#include<time.h>

#define PI 3.1415926535
#define RATE (PI/180)
#define OFFSET 100
#define IN_HEART_NUM 33   //内圈爱心数量
#define OUT_HEART_NUM 47   //外圈爱心数量
#define BEAT_TIME 2  //心脏变大或者变小持续的时间

int is_swelling = -1;   //内部爱心是否在膨胀，控制心跳状态   1变大   -1变小
int move_cnt = 0;
int pre_change_time = 0;

//每个爱心由多少个点组成 放大比例 心跳时的1放缩比例（每个爱心移动步长不同）
int num_in[IN_HEART_NUM] = { 1000, 900, 800, 600, 500, 360, 300, 230, 250, 200, 190, 180, 170, 170, 160, 180, 200, 180, 170, 160, 160, 150, 130, 120, 100, 95, 90, 80, 80, 80, 70, 60, 50  };
float scale_in[IN_HEART_NUM] = { 8.00, 7.9, 7.8, 7.7, 7.6, 7.5, 7.4, 7.3, 7.2, 7.1, 7.0, 6.95, 6.85, 6.95, 6.8, 6.7, 6.69, 6.63, 6.58, 6.5, 6.44, 6.38, 6.3, 6.25, 6.2, 6.1, 6.0, 5.9, 5.8, 5.7, 5.6, 5.5, 5.4 };

int num_out[OUT_HEART_NUM] = { 60, 70, 70, 70, 80, 80, 80, 100, 90, 80, 70, 90, 20, 100, 20, 90, 20, 30, 20, 80, 75, 80, 80, 75, 75, 70, 70, 65, 65, 63, 60, 60, 58, 57, 56, 55, 53, 50, 45, 40, 39, 35, 33, 30, 27, 20, 10 };
float scale_out[OUT_HEART_NUM] = { 10.2, 10.0, 9.8, 9.6, 9.5, 9.4, 9.2, 9, 8.8, 8.6, 8.4, 8.3, 8.25, 8.2, 8.15, 8.1, 8.09, 8.08, 8.07, 8, 7.9, 7.83, 7.74, 7.65, 7.59, 7.5, 7, 6.3, 5.9, 5.4, 5.1, 5.0, 4.9, 4.8, 4.7, 4.5, 4.0, 3.5, 3.3, 3.1, 2.8, 2.5, 2.3, 1.5, 1 };

void init();
void reshape(int w, int h);
void display();
void shrink(float& x, float& y, float ratio);
void cal_pos1(float& x, float& y, float ratio);
void cal_pos2(float& x, float& y, float ratio);

typedef struct {
	bool active;   //存活状态
	float life_time, exist_time;   //生命周期，存在时间    
	float x, y;   //位置  
	unsigned int r, g, b;   //颜色
	float size; //尺寸
}praticle;

class Praticle {
public:
	praticle data;
	Praticle();
	~Praticle();
	void draw();
	void set_position(float x, float y);
	void set_color(unsigned int r, unsigned int g, unsigned b);
	void set_size(float size);
};

class Heart
{
public:
	int point_num;
	int is_inner;  //是内部的爱心1还是外部的-1
	float scale;   //爱心缩放比率
	float swell_rate;  //心跳时的移动比率
	Praticle* p_praticles;

	static void cal_heart_pos(float& x, float& y, float angle, float scale);
	
	Heart(int point_num, int is_inner, float scale, float swell_rate);
	~Heart();

	void init_heart();
	void draw_heart();
	void move_heart();
};

Heart* heart_in[IN_HEART_NUM];
Heart* heart_out[OUT_HEART_NUM];

//***********************************************************************************
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(1000, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Heart");

	init();
	glutDisplayFunc(display);
	glutIdleFunc(display);
	//glutReshapeFunc(reshape);

	glutMainLoop();
	return 0;
}
//************************************************************************************

Praticle::Praticle() 
{
	data.active = true;

	data.exist_time = 0;
	data.life_time = 5;

	data.x = 0;
	data.y = 0;

	data.r = 242;
	data.g = 155;
	data.b = 177;

	data.size = 1;
}

Praticle::~Praticle()
{
}

void Praticle::draw()
{
	glColor3ub(data.r, data.g, data.b);
	glBegin(GL_QUADS);
	glVertex2f(data.x - data.size / 2, data.y - data.size / 2);
	glVertex2f(data.x + data.size / 2, data.y - data.size / 2);
	glVertex2f(data.x + data.size / 2, data.y + data.size / 2);
	glVertex2f(data.x - data.size / 2, data.y + data.size / 2);
	glEnd();
}

void Praticle::set_position(float x, float y)
{
	data.x = x;
	data.y = y;
}

void Praticle::set_color(unsigned int r, unsigned int g, unsigned b)
{
	data.r = r;
	data.g = g;
	data.b = b;
}

void Praticle::set_size(float size)
{
	data.size = size;
}

void init()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glDisable(GL_DEPTH_TEST);   //二维不需要深度测试
	//glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);//glMatrixMode：告诉计算机“我接下来要做什么”，GL_PROJECTION：要对投影相关进行操作
	glLoadIdentity();//将当前的用户坐标系的原点移到了屏幕中心
	gluOrtho2D(-320, 320.0, -240.0, 240.0);//投影类型：正交投影。不会因为离屏幕的远近而产生大小的变换的情况   裁剪视窗

	for (int i = 0; i < IN_HEART_NUM; i++) {    //内圈心跳比例设置取1/4个周期sin函数， 外圈取0.5个周期
		/*if (i == 29) {
			printf("666\n");
		}
		float a = sin(PI / 2.0 / IN_HEART_NUM * i);*/
		/*heart_in[i] = new Heart(num_in[i], 1, scale_in[i], sin(PI / 2.0 / IN_HEART_NUM * i + PI / 2));*/
		//heart_in[i] = new Heart(num_in[i], 1, scale_in[i], sin(PI / 2.0 / IN_HEART_NUM * i ));
		heart_in[i] = new Heart(num_in[i], 1, scale_in[i], 10 * sin(IN_HEART_NUM / 10 * PI));
	}
	for (int i = 0; i < OUT_HEART_NUM; i++) {
		//heart_out[i] = new Heart(num_out[i], -1, scale_out[i], sin(PI / OUT_HEART_NUM * i));
		/*heart_out[i] = new Heart(num_out[i], -1, scale_out[i], sin(PI / 2.0 / IN_HEART_NUM * i + PI / 2));*/
		heart_out[i] = new Heart(num_out[i], -1, scale_out[i], 10 * sin(IN_HEART_NUM / 10 * PI));
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*Praticle pp[point_num];
	float angle = 180;
	float step = 360 / point_num;
	for (int i = 0; i < point_num; i++) {
		srand(i);
		//angle = angle + step + sin(rand()) * 0.5;
		angle = angle + step;
		float x, y;
		Heart::cal_heart_pos(x, y, angle);
		pp[i].set_color(242, 155, 177);
		pp[i].set_position(x, y);
		pp[i].set_size(1);
		pp[i].draw();
	}*/
	for (int i = 0; i < IN_HEART_NUM; i++) {    

		heart_in[i]->draw_heart();
		heart_in[i]->move_heart();
	}
	for (int i = 0; i < OUT_HEART_NUM; i++) {
		heart_out[i]->draw_heart();
		heart_out[i]->move_heart();
	}

	glFlush();
}

void Heart::cal_heart_pos(float& x, float& y, float angle, float scale)
{
	x = 16 * pow(sin(angle * RATE), 3);// +sin(angle * RATE) * 1;
	//y = -1 * (13 * cos(angle * RATE) - 5 * cos(angle * 2 * RATE) - cos(4 * angle * RATE));
	y = 13 * cos(angle * RATE) - 5 * cos(angle * 2 * RATE) - 2 * cos(3 * angle * RATE)- 1 * cos(4 * angle * RATE);



	x *= scale;
	y *= scale;

	y += OFFSET;

}

Heart::Heart(int point_num, int is_inner, float scale, float swell_rate)
{
	this->point_num = point_num;
	this->is_inner = is_inner;
	this->scale = scale;
	this->swell_rate = swell_rate;
	init_heart();
}

Heart::~Heart()
{
	delete[]p_praticles;
}

void Heart::draw_heart()
{
	for (int i = 0; i < point_num; i++) {
		p_praticles[i].draw();
	}
}

void Heart::init_heart()
{
	p_praticles = new Praticle[point_num];
	//srand(rand());
	//float angle = 360 * sin(rand());
	//srand(1);
	float angle = rand() % 360;
	float step = 360 / point_num > 1 ? 360 / point_num : 1;
	for (int i = 0; i < point_num; i++) {
		srand(i);
		angle = angle + step * sin(rand()) + step;
		//angle = angle + step;
		float x, y;
		cal_heart_pos(x, y, angle, scale);
		//shrink(x, y, 4 + 6 * (1 + sin(30 / 10 * PI)));
		shrink(x, y, 20 * is_swelling * (pow(5, swell_rate * 1.1)));
		p_praticles[i].set_color(242, 80, 160);
		p_praticles[i].set_position(x, y);
		p_praticles[i].draw();
	}
}

void Heart::move_heart()
{
	//printf("%.3f	%.3f\n", p_praticles[0].data.x, p_praticles[0].data.y);
	//for (int i = 0; i < point_num; i++) {
	//	/*p_praticles[i].data.x = p_praticles[i].data.x + swell_rate * MOVE_X * is_swelling;
	//	p_praticles[i].data.y = p_praticles[i].data.y + swell_rate * MOVE_Y * is_swelling;*/
	//	p_praticles[i].data.x *= (swell_rate+is_swelling);
	//	p_praticles[i].data.y *= (swell_rate+is_swelling);
	//	if (p_praticles[i].data.x > 320 || p_praticles[i].data.x < -320) {
	//		is_swelling *= -1;
	//	}
	//}
	if (move_cnt % 30000 > 15000) {
		is_swelling = -1;
	}
	else {
		is_swelling = 1;
	}
	/*printf("%f	%f\n", heart_in[0]->p_praticles->data.x, heart_in[0]->p_praticles->data.y);
	if (heart_in[0]->p_praticles->data.y > 196 || heart_in[0]->p_praticles->data.y < 183.5)
		is_swelling *= -1;*/

	/*time_t now = time(NULL);
	tm* tm_t = localtime(&now);
	int cur_sec = tm_t->tm_sec;*/
	/*SYSTEMTIME sysTime = { 0 };
	GetSystemTime(&sysTime);
	int cur_sec = sysTime.wSecond;
	if (cur_sec - pre_change_time >= BEAT_TIME) {
		is_swelling *= -1;
		printf("%d	%d\n", cur_sec, pre_change_time);
		pre_change_time = cur_sec;
	}*/

	for (int i = 0; i < point_num; i++) {
		//cal_pos1(p_praticles[i].data.x, p_praticles[i].data.y, 4 * is_swelling * (pow(5, swell_rate * 1.1)) * is_inner);
		cal_pos1(p_praticles[i].data.x, p_praticles[i].data.y, 9 * is_swelling * (pow(5, swell_rate * 1.1)) * pow(2, is_inner - 1));
		//cal_pos1(p_praticles[i].data.x, p_praticles[i].data.y, is_swelling*(pow(5.5,swell_rate * 1.4 )) * pow(2, is_inner-1) );
		//cal_pos1(p_praticles[i].data.x, p_praticles[i].data.y, is_swelling*(pow(5,swell_rate*1.2 )) * is_inner );
		//cal_pos1(p_praticles[i].data.x, p_praticles[i].data.y, is_swelling * (swell_rate * 6) * is_inner);
		//cal_pos2(p_praticles[i].data.x, p_praticles[i].data.y, is_swelling == 1?(swell_rate+0.5):swell_rate);
	}
	move_cnt = (move_cnt + 1) % 60000;
}

void cal_pos1(float& x, float& y, float ratio)
{
	float force = 1 / (pow(x, 2) + pow(y - 300, 2));
	float dx = ratio * force * x;
	float dy = ratio * force * (y - 300);
	x = x - dx * 4;
	y = y - dy;
}
void cal_pos2(float& x, float& y, float ratio)
{
	x = x * ratio;
	y = y * ratio;
}

void shrink(float& x, float& y, float ratio)
{
	float force = 1 / pow((pow(x, 2) + pow(y - 300, 2)), 0.6);
	float dx = ratio * force * x;
	float dy = ratio * force * (y - 300);
	x = x - dx;
	y = y - dy;
}