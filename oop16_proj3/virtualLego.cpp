////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: 박창현 Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>

#include <tchar.h>
#include <string>

#include <cstdio>
#include <cassert>
namespace std {
#include <cstdlib>
}


IDirect3DDevice9* Device = NULL;

// window size
const int Width = 1024;
const int Height = 768;

static int spacecheck = 0;
float levelpower;
int count = 0;

int ballIntersected = 0;
int ballIntersected_inf = 0;
int currentLife = 4;
bool isCleared = false;

bool isMenuOn = true;
int selectedLevel = 0;

// There are four balls
// initialize the position (coordinate) of each ball (ball0 ~ ball3)
const float spherePos[54][2] = { {-1.575f,3} , {-1.125f,3} , {-0.675f,3} ,{-0.225,3} ,{0.225f,3} ,{0.675f,3} ,{1.125f,3} ,{1.575f,3} ,{2.000f,2.55f} ,{-2.000f,2.55f} ,{-2.4f,2.1f} ,{2.4f,2.1f} ,
{-2.4f,1.65f},{-2.4f,1.2f},{-2.4f,0.75f},{-2.4f,0.3f},{-2.4f,-0.15f},{-2.4f,-0.6f},{-2.4f,-1.05f},{-2.4f,-1.5f},{-2.4f,-1.95f},{-2.0f,-2.3f},
{2.4f,1.65f},{2.4f,1.2f},{2.4f,0.75f},{2.4f,0.3f},{2.4f,-0.15f},{2.4f,-0.6f},{2.4f,-1.05f},{2.4f,-1.5f},{2.4f,-1.95f},{2.0f,-2.3f},
{-1.575f,-2.75f} , {-1.125f,-2.75f} , {-0.675f,-2.75f} ,{-0.225,-2.75f} ,{0.225f,-2.75f} ,{0.675f,-2.75f} ,{1.125f,-2.75f} ,{1.575f,-2.75f},
{-1.125f,1.65f},{1.125f,1.65f},{-1.125f,1.2f},{1.125f,1.2f},{-0.225f,0.75f},{-0.225f,0.3f},
{-1.575f,-0.15f},{-1.125f,-0.6f},{-0.675f,-1.05f},{-0.225f,-1.05f},{0.225f,-1.05f},{0.675f,-1.05f},{1.125f,-0.6f},{1.575f,-0.15f},
};
// initialize the color of each ball (ball0 ~ ball3)
const D3DXCOLOR sphereColor[5] = { d3d::YELLOW, d3d::RED, d3d::WHITE,d3d::CYAN,d3d::GREEN };

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------



class CSphere {
private:
	float					center_x, center_y, center_z;
	float					bf_center_x, bf_center_y, bf_center_z;
	float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;
	int intersect;


	D3DXCOLOR color_sphere;

public:

	CSphere(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_radius = 0;
		m_velocity_x = 0;
		m_velocity_z = 0;
		intersect = 0;
		m_pSphereMesh = NULL;
	}
	~CSphere(void) {}

public:
	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;
		color_sphere = color;
		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
			return false;
		return true;
	}

	D3DXCOLOR getcolor() {
		return color_sphere;

	}

	void changecolor(D3DXCOLOR color) {
		color_sphere = color;
		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;

	}

	void destroy(void)
	{
		if (m_pSphereMesh != NULL) {
			m_pSphereMesh->Release();
			m_pSphereMesh = NULL;
		}
	}

	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		if (m_pSphereMesh) { //prevent error when Lifeball disappears
			m_pSphereMesh->DrawSubset(0);
		}
	}
	void Update() {
		bf_center_x = center_x, bf_center_z = center_z;
	}

	bool hasIntersected(CSphere& ball)
	{
		//check two ball's radius
		// Insert your code here.

		float distancebtwballs = (pow(center_x - ball.center_x, 2) + pow(center_y - ball.center_y, 2) + pow(center_z - ball.center_z, 2));


		if (distancebtwballs <= pow(M_RADIUS * 2, 2) && ball.intersect == 0) {
			ball.intersect = 1;
			ball.setCenter(ball.bf_center_x, center_y, ball.bf_center_z);
			return true;
		}
		else {
			ball.Update();
		}
		ball.intersect = 0;

		return false;
	}

	void hitBy(CSphere& ball)
	{
		D3DXVECTOR3 targetpos = this->getCenter();
		D3DXVECTOR3 redpos = ball.getCenter();

		double theta = acos(sqrt(pow(targetpos.x - redpos.x, 2)) / sqrt(pow(targetpos.x - redpos.x, 2) +
			pow(targetpos.z - redpos.z, 2)));		// 기본 1 사분면
		if (targetpos.z - redpos.z <= 0 && targetpos.x - redpos.x >= 0) { theta = -theta; }	//4 사분면
		if (targetpos.z - redpos.z >= 0 && targetpos.x - redpos.x <= 0) { theta = PI - theta; } //2 사분면
		if (targetpos.z - redpos.z <= 0 && targetpos.x - redpos.x <= 0) { theta = PI + theta; } // 3 사분면
		ball.setCenter(redpos.x - cos(theta) * 0.02, redpos.y, redpos.z - sin(theta) * 0.02);
		ball.setPower(-levelpower * cos(theta), -levelpower * sin(theta));




		//normal vector

		/*if (ball.intersect == 1) {
			float temp = ball.m_velocity_x;
			ball.m_velocity_x = ball.m_velocity_z;
			ball.m_velocity_z = -temp;
		}*/
		// Insert your code here.

	}

	void ballUpdate(float timeDiff)
	{
		const float TIME_SCALE = 3.3;
		D3DXVECTOR3 cord = this->getCenter();
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		if (vx > 0.01 || vz > 0.01)
		{
			float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
			float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;

			//correction of position of ball
			// Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
			/*if(tX >= (4.5 - M_RADIUS))
				tX = 4.5 - M_RADIUS;
			else if(tX <=(-4.5 + M_RADIUS))
				tX = -4.5 + M_RADIUS;
			else if(tZ <= (-3 + M_RADIUS))
				tZ = -3 + M_RADIUS;
			else if(tZ >= (3 - M_RADIUS))
				tZ = 3 - M_RADIUS;*/

			this->setCenter(tX, cord.y, tZ);
		}
		else { this->setPower(0, 0); }
		//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);

		double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400;
		if (rate < 0)
			rate = 0;
		this->setPower(getVelocity_X() * 1, getVelocity_Z() * 1);
	}

	double getVelocity_X() { return this->m_velocity_x; }
	double getVelocity_Z() { return this->m_velocity_z; }

	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x = x;	center_y = y;	center_z = z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getRadius(void)  const { return (float)(M_RADIUS); }
	const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	D3DXVECTOR3 getCenter(void) const
	{
		D3DXVECTOR3 org(center_x, center_y, center_z);
		return org;
	}

private:
	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pSphereMesh;

};



// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:

	float					m_x;
	float					m_z;
	float                   m_width;
	float                   m_depth;
	float					m_height;


public:
	CWall(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_width = 0;
		m_depth = 0;
		m_pBoundMesh = NULL;
	}
	~CWall(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;

		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		m_width = iwidth;
		m_depth = idepth;

		if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
			return false;
		return true;
	}
	void destroy(void)
	{
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release();
			m_pBoundMesh = NULL;
		}
	}
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
	}

	bool hasIntersected(CSphere& ball)
	{

		D3DXVECTOR3 b = ball.getCenter();
		float wx1 = 3.5;
		float wx2 = -3.5;
		float wz1 = -5.12f;
		float wz2 = 5.12f;

		if (sqrt(pow(wx1 - b.x, 2)) <= M_RADIUS || sqrt(pow(wx2 - b.x <= M_RADIUS, 2)) || sqrt(pow(wz2 - b.z, 2)) <= M_RADIUS || sqrt(pow(wz1 - b.z, 2)) <= M_RADIUS) { return true; }
		return false;
	}

	void hitBy(CSphere& ball)
	{
		D3DXVECTOR3 rb = ball.getCenter();
		float vx = ball.getVelocity_X();
		float vz = ball.getVelocity_Z();
		float wx1 = 3.5;
		float wx2 = -3.5;
		float wz1 = -5.12f;
		float wz2 = 5.12f;


		if (sqrt(pow(wx1 - rb.x, 2)) <= M_RADIUS && vx > 0) { vx = -vx; } // wall[0]의 경우
		if (sqrt(pow(wx2 - rb.x, 2)) <= M_RADIUS && vx < 0) { vx = -vx; } // wall[1]의 경우
		if (sqrt(pow(wz2 - rb.z, 2)) <= M_RADIUS && vz > 0) { vz = -vz; } // wall[2]의 경우

		if (sqrt(pow(wz1 - rb.z, 2)) <= M_RADIUS && vz < 0) {
			ball.destroy();

			spacecheck = 2;
			count++;
			vx = 0;
			vz = 0;
		}
		ball.setPower(vx, vz);

	}

	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getHeight(void) const { return M_HEIGHT; }
	D3DXVECTOR3 getCenter(void) const
	{
		D3DXVECTOR3 org(m_x, m_height, m_z);
		return org;
	}


private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pBoundMesh;
};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
	CLight(void)
	{
		static DWORD i = 0;
		m_index = i++;
		D3DXMatrixIdentity(&m_mLocal);
		::ZeroMemory(&m_lit, sizeof(m_lit));
		m_pMesh = NULL;
		m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_bound._radius = 0.0f;
	}
	~CLight(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
	{
		if (NULL == pDevice)
			return false;
		if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
			return false;

		m_bound._center = lit.Position;
		m_bound._radius = radius;

		m_lit.Type = lit.Type;
		m_lit.Diffuse = lit.Diffuse;
		m_lit.Specular = lit.Specular;
		m_lit.Ambient = lit.Ambient;
		m_lit.Position = lit.Position;
		m_lit.Direction = lit.Direction;
		m_lit.Range = lit.Range;
		m_lit.Falloff = lit.Falloff;
		m_lit.Attenuation0 = lit.Attenuation0;
		m_lit.Attenuation1 = lit.Attenuation1;
		m_lit.Attenuation2 = lit.Attenuation2;
		m_lit.Theta = lit.Theta;
		m_lit.Phi = lit.Phi;
		return true;
	}
	void destroy(void)
	{
		if (m_pMesh != NULL) {
			m_pMesh->Release();
			m_pMesh = NULL;
		}
	}
	bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return false;

		D3DXVECTOR3 pos(m_bound._center);
		D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
		D3DXVec3TransformCoord(&pos, &pos, &mWorld);
		m_lit.Position = pos;

		pDevice->SetLight(m_index, &m_lit);
		pDevice->LightEnable(m_index, TRUE);
		return true;
	}

	void draw(IDirect3DDevice9* pDevice)
	{
		if (NULL == pDevice)
			return;
		D3DXMATRIX m;
		D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
		pDevice->SetTransform(D3DTS_WORLD, &m);
		pDevice->SetMaterial(&d3d::WHITE_MTRL);
		m_pMesh->DrawSubset(0);
	}

	D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
	DWORD               m_index;
	D3DXMATRIX          m_mLocal;
	D3DLIGHT9           m_lit;
	ID3DXMesh* m_pMesh;
	d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[3];
CSphere	g_sphere_yellow[54];
//CSphere	g_target_blueball;

CSphere	g_lifeBall[5];


CSphere	whiteball;
CSphere	redball;
CLight	g_light;

//font variables
ID3DXFont* g_pFontSmall = NULL;
ID3DXFont* g_pFontBig = NULL;


double g_camera_pos[3] = { 0.0, 5.0, -8.0 };

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}


float* getrandompos(int num) {
	float* row_col = new float[2];
	row_col[0] = -2.925f + (num % 14) * 0.45f;
	row_col[1] = 3.0f - float(num / 14) * 0.45f;

	return row_col;
}

void getrandommap() {


	int data[196], i, sub_i;
	int data_color[196];

	srand((unsigned int)time(NULL));
	for (i = 0; i < 196; i++) {
		data[i] = rand() % 196;//0~195
		for (sub_i = 0; sub_i < i; sub_i++) {
			if (data[i] == data[sub_i]) {
				i--;
				break;
			}
		}
	}
	for (i = 0; i < 196; i++) {
		data_color[i] = rand() % 196;//0~195
		for (sub_i = 0; sub_i < i; sub_i++) {
			if (data_color[i] == data_color[sub_i]) {
				i--;
				break;
			}
		}
	}

	for (int i = 0; i < 54; i++) {
		if (data_color[i] > 150) {
			if (false == g_sphere_yellow[i].create(Device, sphereColor[4])) return;
		}
		else if (data_color[i] > 90) {
			if (false == g_sphere_yellow[i].create(Device, sphereColor[3])) return;
		}
		else {
			if (false == g_sphere_yellow[i].create(Device, sphereColor[0])) return;
		}


		g_sphere_yellow[i].setCenter(getrandompos(data[i])[0], (float)M_RADIUS, getrandompos(data[i])[1]);
		g_sphere_yellow[i].setPower(0, 0);
	}
}


// initialization
bool Setup()
{
	int i;
	int j;

	D3DXMatrixIdentity(&g_mWorld);
	D3DXMatrixIdentity(&g_mView);
	D3DXMatrixIdentity(&g_mProj);

	// create plane and set the position
	if (false == g_legoPlane.create(Device, -1, -1, 7, 0.03f, 10.26, d3d::GREEN)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0].create(Device, -1, -1, 7, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0.12f, 5.06f);
	//if (false == g_legowall[1].create(Device, -1, -1, 0, 0.3f, 0.12f, d3d::DARKRED)) return false;
	//g_legowall[1].setPosition(0.0f, 0.12f, -4.06f);
	if (false == g_legowall[1].create(Device, -1, -1, 0.12f, 0.3f, 10.24f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(3.56f, 0.12f, 0.0f);
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 10.24f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(-3.56f, 0.12f, 0.0f);

	// create four balls and set the position
	for (i = 0; i < 54; i++) {
		if (false == g_sphere_yellow[i].create(Device, sphereColor[0])) return false;
		g_sphere_yellow[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
		g_sphere_yellow[i].setPower(0, 0);
	}
	// create life balls max 5 balls
	for (j = 0; j < 5; j++) {
		if (false == g_lifeBall[j].create(Device, sphereColor[2])) return false;
		g_lifeBall[j].setCenter(-8.8f + 0.5f * j, 3.7f, 7.1f);
	}


	// create red, white ball for set direction
	if (false == redball.create(Device, d3d::RED)) return false;
	redball.setCenter(-3.7f, (float)M_RADIUS, -4.5f);

	if (false == whiteball.create(Device, d3d::WHITE)) return false;
	whiteball.setCenter(-3.7f, (float)M_RADIUS, -4.95f);

	// light setting 
	D3DLIGHT9 lit;
	::ZeroMemory(&lit, sizeof(lit));
	lit.Type = D3DLIGHT_POINT;
	lit.Diffuse = d3d::WHITE;
	lit.Specular = d3d::WHITE * 0.9f;
	lit.Ambient = d3d::WHITE * 0.9f;
	lit.Position = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
	lit.Range = 100.0f;
	lit.Attenuation0 = 0.0f;
	lit.Attenuation1 = 0.9f;
	lit.Attenuation2 = 0.0f;
	if (false == g_light.create(Device, lit))
		return false;

	// Position and aim the camera.
	D3DXVECTOR3 pos(0.0f, 9.0f, -12.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);

	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
		(float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

	// Set render states.
	Device->SetRenderState(D3DRS_LIGHTING, TRUE);
	Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	g_light.setLight(Device, g_mWorld);

	D3DXCreateFont(Device, 20, 10, 1000, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, "신명조", &g_pFontSmall);
	D3DXCreateFont(Device, 40, 20, 1000, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, "신명조", &g_pFontBig);


	return true;
}

void Cleanup(void)
{
	g_legoPlane.destroy();
	for (int i = 0; i < 3; i++) {
		g_legowall[i].destroy();
	}
	destroyAllLegoBlock();
	g_light.destroy();
	g_pFontSmall->Release();
	g_pFontBig->Release();
}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
	int i = 0;
	int j = 0;


	if (Device)
	{
		//LPD3DXFONT pFont = NULL;
		//LPD3DXSPRITE pTextSprite = NULL;



		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();

		RECT rc;
		//D3DXCreateSprite(Device, &pTextSprite);

		//pTextSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);


		SetRect(&rc, 10, 10, 0, 0);
		g_pFontSmall->DrawText(NULL, _T("Life: "), -1, &rc, DT_NOCLIP, 0xffffff00);


		std::string scoreStr1 = "Score: " + std::to_string(ballIntersected);
		LPCSTR scoreLpcStr1 = scoreStr1.c_str();
		SetRect(&rc, 200, 10, 0, 0);
		g_pFontSmall->DrawText(NULL, _T(scoreLpcStr1), -1, &rc, DT_NOCLIP, 0xffffff00);


		std::string scoreStr2 = (!isMenuOn && spacecheck == 0) ? (currentLife == -1 || isCleared ? "Press Spacebar To Try Again" : "Press Spacebar To Throw A Ball") : "";
		LPCSTR scoreLpcStr2 = scoreStr2.c_str();
		SetRect(&rc, 390, 680, 0, 0);
		g_pFontSmall->DrawText(NULL, _T(scoreLpcStr2), -1, &rc, DT_NOCLIP, 0xff000000);

		std::string scoreStr3 = currentLife == -1 && isMenuOn ? "Game Over" : "";
		LPCSTR scoreLpcStr3 = scoreStr3.c_str();
		SetRect(&rc, 400, 300, 0, 0);
		g_pFontBig->DrawText(NULL, _T(scoreLpcStr3), -1, &rc, DT_NOCLIP, 0xffff0000);

		std::string scoreStr4 = currentLife == -1&& isMenuOn ? ("Score: " + std::to_string(ballIntersected)) : "";
		LPCSTR scoreLpcStr4 = scoreStr4.c_str();
		SetRect(&rc, 455, 360, 0, 0);
		g_pFontSmall->DrawText(NULL, _T(scoreLpcStr4), -1, &rc, DT_NOCLIP, 0xffff0000);

		std::string scoreStr5 = isCleared&&isMenuOn ? "Congratulation!" : "";
		LPCSTR scoreLpcStr5 = scoreStr5.c_str();
		SetRect(&rc, 350, 300, 0, 0);
		g_pFontBig->DrawText(NULL, _T(scoreLpcStr5), -1, &rc, DT_NOCLIP, 0xffff0000);


		std::string menuStr1 = isMenuOn ? "Select Level" : "";
		LPCSTR menuLpcStr1 = menuStr1.c_str();
		SetRect(&rc, 50, 680, 0, 0);
		g_pFontSmall->DrawText(NULL, _T(menuLpcStr1), -1, &rc, DT_NOCLIP, 0xff000000);

		std::string menuStr2 = (isMenuOn && !(selectedLevel == 0)) ? "Normal" : "";
		LPCSTR menuLpcStr2 = menuStr2.c_str();
		SetRect(&rc, 300, 680, 0, 0);
		g_pFontSmall->DrawText(NULL, _T(menuLpcStr2), -1, &rc, DT_NOCLIP, 0xff000000);

		std::string menuStr3 = (isMenuOn && !(selectedLevel == 1)) ? "Hard" : "";
		LPCSTR menuLpcStr3 = menuStr3.c_str();
		SetRect(&rc, 500, 680, 0, 0);
		g_pFontSmall->DrawText(NULL, _T(menuLpcStr3), -1, &rc, DT_NOCLIP, 0xff000000);

		std::string menuStr4 = (isMenuOn && !(selectedLevel == 2)) ? "Infinity" : "";
		LPCSTR menuLpcStr4 = menuStr4.c_str();
		SetRect(&rc, 700, 680, 0, 0);
		g_pFontSmall->DrawText(NULL, _T(menuLpcStr4), -1, &rc, DT_NOCLIP, 0xff000000);

		std::string menuStr5 = (isMenuOn && (selectedLevel == 0)) ? "Normal" : "";
		LPCSTR menuLpcStr5 = menuStr5.c_str();
		SetRect(&rc, 300, 680, 0, 0);
		g_pFontSmall->DrawText(NULL, _T(menuLpcStr5), -1, &rc, DT_NOCLIP, 0xffff0000);

		std::string menuStr6 = (isMenuOn && (selectedLevel == 1)) ? "Hard" : "";
		LPCSTR menuLpcStr6 = menuStr6.c_str();
		SetRect(&rc, 500, 680, 0, 0);
		g_pFontSmall->DrawText(NULL, _T(menuLpcStr6), -1, &rc, DT_NOCLIP, 0xffff0000);

		std::string menuStr7 = (isMenuOn && (selectedLevel == 2)) ? "Infinity" : "";
		LPCSTR menuLpcStr7 = menuStr7.c_str();
		SetRect(&rc, 700, 680, 0, 0);
		g_pFontSmall->DrawText(NULL, _T(menuLpcStr7), -1, &rc, DT_NOCLIP, 0xffff0000);

		//pTextSprite->End();

		// update the position of each ball. during update, check whether each ball hit by walls.

		g_sphere_yellow[i].ballUpdate(timeDelta);
		g_lifeBall[currentLife].ballUpdate(timeDelta);

		whiteball.ballUpdate(timeDelta);
		redball.ballUpdate(timeDelta);



		//level1
		//level2
		//level3
		// check whether any two balls hit together and update the direction of balls



		for (i = 0; i < 54; i++) {
			if (g_sphere_yellow[i].hasIntersected(redball) == true) {

				g_sphere_yellow[i].hitBy(redball);

				ballIntersected++;


				if (ballIntersected > 53 && selectedLevel != 2) {
					isCleared = true;
					isMenuOn = true;
				}
				if (selectedLevel != 2) {
					g_sphere_yellow[i].destroy();
					if (false == g_sphere_yellow[i].create(Device, sphereColor[0])) return false;
					g_sphere_yellow[i].setCenter(222, (float)M_RADIUS, 222);
				}

				if (selectedLevel == 2) {

					//color에 따른 비교

					if (g_sphere_yellow[i].getcolor() == d3d::YELLOW) {
						g_sphere_yellow[i].destroy();
						if (false == g_sphere_yellow[i].create(Device, sphereColor[0])) return false;
						g_sphere_yellow[i].setCenter(222, (float)M_RADIUS, 222);
						ballIntersected_inf++;

					}
					else if (g_sphere_yellow[i].getcolor() == d3d::CYAN) {
						g_sphere_yellow[i].changecolor(d3d::YELLOW);

					}
					else if (g_sphere_yellow[i].getcolor() == d3d::GREEN) {

						g_sphere_yellow[i].changecolor(d3d::CYAN);

					}
				}

			}
		}
		if (selectedLevel == 2) {
			levelpower = 2.0 + float(ballIntersected) / 30.0;

			if (levelpower > 4) {
				levelpower = 4;
			}
			if (ballIntersected_inf > 53) {
				getrandommap();
				ballIntersected_inf = 0;
			}
		}

		if (whiteball.hasIntersected(redball) == true) {


			whiteball.hitBy(redball);


		}

		//check if the redball has intersected the wall
		if (g_legowall->hasIntersected(redball)) {

			g_legowall->hitBy(redball);
			if (spacecheck == 2) {


				//g_lifeBall[currentLife].setCenter(222, (float)M_RADIUS, 222);
				//g_lifeBall[currentLife].setPower(0, 0);

				if (currentLife > -1) {
					g_lifeBall[currentLife].destroy();
					currentLife--;

				}
				if (currentLife == -1) {
					isMenuOn = true;
				}


				redball.create(Device, d3d::RED);
				redball.setCenter(whiteball.getCenter().x, (float)M_RADIUS, -4.5f);
				spacecheck = 0;
			}

		}

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);
		for (i = 0; i < 3; i++) {
			g_legowall[i].draw(Device, g_mWorld);
		}

		for (i = 0; i < 54; i++) {
			g_sphere_yellow[i].draw(Device, g_mWorld);
		}
		for (i = 0; i < 5; i++) {
			g_lifeBall[i].draw(Device, g_mWorld);
		}
		redball.draw(Device, g_mWorld);
		whiteball.draw(Device, g_mWorld);
		g_light.draw(Device);

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture(0, NULL);
	}
	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static bool isReset = true;
	static int old_x = 0;
	static int old_y = 0;
	static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

	switch (msg) {
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_ESCAPE:
			::DestroyWindow(hwnd);
			break;
		case VK_RETURN:
			if (NULL != Device) {
				wire = !wire;
				Device->SetRenderState(D3DRS_FILLMODE,
					(wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
			}
			break;
		case VK_SPACE: //start
			if (isMenuOn) {
				if (selectedLevel == 0) {
					for (int i = 0; i < 54; i++) {
						if (false == g_sphere_yellow[i].create(Device, sphereColor[0])) return false;
						g_sphere_yellow[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
						g_sphere_yellow[i].setPower(0, 0);
					}
					levelpower = 2;
				}
				if (selectedLevel == 1) {
					for (int i = 0; i < 54; i++) {
						if (false == g_sphere_yellow[i].create(Device, sphereColor[0])) return false;
						g_sphere_yellow[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
						g_sphere_yellow[i].setPower(0, 0);
					}
					levelpower = 3.5;
				}
				if (selectedLevel == 2) {
					getrandommap();
					levelpower = 2;
				}
				isMenuOn = false;
			}
			else {

				if (spacecheck == 0) {

					redball.setPower(0, levelpower);
					if (currentLife == -1 || isCleared) {

						isCleared = false;
						currentLife = 4;
						ballIntersected = 0;
						for (int j = 0; j < 5; j++) {
							if (false == g_lifeBall[j].create(Device, sphereColor[2])) return false;
							g_lifeBall[j].setCenter(-8.8f + 0.5f * j, 3.7f, 7.1f);
						}
						if (selectedLevel == 2) {
							ballIntersected_inf = 0;
							//getrandommap();
							levelpower = 2;
						}
						if (selectedLevel != 2) {
							if (selectedLevel == 0) {
								levelpower = 2;
							}
							if (selectedLevel == 1) {
								levelpower = 3.5;
							}
							for (int i = 0; i < 54; i++) {
								if (false == g_sphere_yellow[i].create(Device, sphereColor[0])) return false;
								g_sphere_yellow[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
								g_sphere_yellow[i].setPower(0, 0);
							}
						}

					}
				}


				spacecheck = 1;

				/*double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
					pow(targetpos.z - whitepos.z, 2)));		// 기본 1 사분면
				if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }	//4 사분면
				if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = PI - theta; } //2 사분면
				if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0){ theta = PI + theta; } // 3 사분면
				double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
				g_sphere_yellow[3].setPower(distance * cos(theta), distance * sin(theta));*/

				break;
			}

		case 0x41:
			if (isMenuOn) {
				if (selectedLevel != 0) {
					selectedLevel--;
				}
			}
			break;
		case 0x44:
			if (isMenuOn) {
				if (selectedLevel != 2) {
					selectedLevel++;
				}
			}
			break;
		}

		break;
	}

	case WM_MOUSEMOVE:
	{
		if (!isMenuOn) {
			int new_x = LOWORD(lParam);
			int new_y = HIWORD(lParam);
			float dx;
			float dy;


			dx = (old_x - new_x);// * 0.01f;
			dy = (old_y - new_y);// * 0.01f;
			D3DXVECTOR3 coord3d_white = whiteball.getCenter();
			whiteball.setCenter(coord3d_white.x + dx * (-0.007f), coord3d_white.y, coord3d_white.z);

			if (spacecheck == 0) {


				D3DXVECTOR3 coord3d_red = redball.getCenter();
				redball.setCenter(coord3d_white.x + (dx) * (-0.007f), coord3d_red.y, coord3d_red.z);

			}
			old_x = new_x;
			old_y = new_y;




			if (LOWORD(wParam) & MK_LBUTTON) {

				if (isReset) {
					isReset = false;
				}
				else {
					D3DXVECTOR3 vDist;
					D3DXVECTOR3 vTrans;
					D3DXMATRIX mTrans;
					D3DXMATRIX mX;
					D3DXMATRIX mY;

					switch (move) {
					case WORLD_MOVE:
						dx = (old_x - new_x) * 0.01f;
						dy = (old_y - new_y) * 0.01f;
						D3DXMatrixRotationY(&mX, dx);
						D3DXMatrixRotationX(&mY, dy);
						g_mWorld = g_mWorld * mX * mY;

						break;
					}
				}

				old_x = new_x;
				old_y = new_y;

			}
			else {
				isReset = true;

				if (LOWORD(wParam) & MK_RBUTTON) {
					dx = (old_x - new_x);// * 0.01f;
					dy = (old_y - new_y);// * 0.01f;

					//D3DXVECTOR3 coord3d=g_target_blueball.getCenter();
					//g_target_blueball.setCenter(coord3d.x+dx*(-0.007f),coord3d.y,coord3d.z+dy*0.007f );
				}
				old_x = new_x;
				old_y = new_y;

				move = WORLD_MOVE;
			}

		}
		break;
	}

	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	srand(static_cast<unsigned int>(time(NULL)));

	if (!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	Device->Release();

	return 0;
}