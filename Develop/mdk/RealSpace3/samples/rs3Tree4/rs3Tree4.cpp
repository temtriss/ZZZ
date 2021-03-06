#include <windows.h>

#define _CRT_SECURE_NO_DEPRECATE

#include "../sampleApp.h"
#include "RDeviceD3D.h"
#include "RSceneManager.h"

#include "RTreeSceneNode.h"
#include "RTreeResourceMgr.h"
#include "RTreePassRenderer.h"

#include "RRenderHelper.h"
//#include "../../../../CSCommon/include/CSTalentInfo.h"

//#pragma comment(lib, "../../../RealSpace3/lib/RealSpace3d.lib")
//#pragma comment(lib, "../../../cml2/lib/cml2d.lib")
//#pragma comment(lib, "../../../../cscommon/lib/CSCommond.lib")

using namespace rs3;

#include "d3dx9.h"

struct Vert
{
	float x,y,z;
	DWORD color;
};


class myApplication : public sampleApp 
{
public:
	myApplication() : sampleApp() {}

	//RGrassSystem*		Grass;

	RTreeSceneNode*		Tree[1000];
	float				lod;

	Vert				Ground[4];

	float				xx, yy;

	RTextSceneNode*		status;

	char *GetName() { return "rs3 tree4 sample application"; }
	void OnUpdate();
	bool OnCreate();
	void OnRender();
	void OnDestroy();

	virtual bool OnWndMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pReturn);

} g_App;
	
bool myApplication::OnWndMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pReturn)
{
	static bool bCollisionView(false);
	RMatrix m;
	RVector v;
	//CSpeedGrassRT::SBlade b;
	
	switch (message)
	{
	case WM_CHAR:
		if(wParam==VK_ESCAPE) PostMessage(hWnd,WM_CLOSE,0,0);
		switch (wParam)
		{
// 		case 'q':
// 			for (int i=0; i<100; i++)
// 			{
// 				b.m_afBottomColor[0] = b.m_afBottomColor[1] = b.m_afBottomColor[2] = 1.0f;
// 				b.m_afTopColor[0] = b.m_afTopColor[1] = b.m_afTopColor[2] = 1.0f;
// 				b.m_afNormal[0] = 1;
// 				b.m_afNormal[1] = 0;
// 				b.m_afNormal[2] = 0;
// 				b.m_afPos[0] = RandomNumber(0.0f, 1000.0f);
// 				b.m_afPos[1] = RandomNumber(0.0f, 1000.0f);
// 				b.m_afPos[2] = 0;
// 				b.m_fNoise = 15;
// 				b.m_fSize = RandomNumber(15.0f, 20.0f);
// 				b.m_fThrow = 1.0f;
// 				b.m_ucWhichTexture = 0;
// 				Grass->AddBlade(b);
// 			}
// 			Grass->UpdateDXBuffers();
// 			break;
// 		case 'e':
// 			Grass->DeleteBlade(0, 0, 300);
// 			Grass->UpdateDXBuffers();
// 			break;
		case 'e':
			m.SetRotationZ(0.1f);
			v = Tree[0]->GetDirection();
			m.TransformVect(v);
			Tree[0]->SetDirection(v);
			break;
// 		case 't':
// 			lod += 0.01;
// 			Tree->SetLOD(lod);
// 			break;
// 		case 'g':
// 			lod -= 0.01;
// 			Tree->SetLOD(lod);
// 			break;
		case 'v':
			bCollisionView = !bCollisionView;
			GetSceneManager()->GetPassRenderer< RTreePassRenderer >()->SetRenderCollisionObject( bCollisionView );
			break;

		// by pok, GPU / CPU 변환시에 이용하자
		case 'r':
			//GetSceneManager()->m_pTreeSystem->SetUseGPU(true, true);
			break;
		case 'f':
			//GetSceneManager()->m_pTreeSystem->SetUseGPU(true, false);
			break;
		case 'u':
			yy += 1.0f;
			break;
		case 'j':
			yy -= 1.0f;
			break;
		case 'h':
			xx += 1.0f;
			break;
		case 'k':
			xx -= 1.0f;
			break;
		case '1':
			GetSceneManager()->GetResourceMgr<RTreeResourceMgr>()->SetWindStrength(0);
			break;
		case '2':
			GetSceneManager()->GetResourceMgr<RTreeResourceMgr>()->SetWindStrength(0.5);
			break;
		case '3':
			GetSceneManager()->GetResourceMgr<RTreeResourceMgr>()->SetWindStrength(1);
			break;
		}
		break;
	case WM_KEYDOWN:
		break;
	};

	return sampleApp::OnWndMessage(hWnd, message, wParam, lParam, pReturn);
}



bool myApplication::OnCreate()
{	
	//GetSceneManager()->m_pTreeSystem->SetUseGPU(true, true);
	lod = 1.0f;
 	
	RVector a, b, c;
	RVector4 d;

	m_pCamera->SetDirection(RVector(1, 0, 0));
	m_pCamera->SetPosition(RVector(0, 0, 0));
	m_pCamera->SetNearFarZ(10, 2000);

	m_pCamera->UpdateViewFrustrum();

	RMatrix vm = m_pCamera->GetViewProjectionMatrix();

	a.Set(10, 0, 0);
	b.Set(50, 0, 0);
	c.Set(2000, 0, 0);

	vm.TransformVect(a, d);
	vm.TransformVect(b, d);
	vm.TransformVect(c, d);


	xx = yy = 0;

	const char* TreeName[] =
	{
		"k_teress_Agarwood_RT_2.spt",
		"k_teress_CaliforniaBuckeye_RT.spt",
		"k_teress_CommonOlive_RT.spt",
//		"ice_CommonJuniper.spt",
//		"SugarPine_RT_Snow_k01.spt",
	};

	for (int i=0; i<10; i++)
	{
		Tree[i] = static_cast<RTreeSceneNode*>(GetSceneManager()->CreateSceneNode( RSID_TREE ));
		Tree[i]->Build( TreeName[2] );

		GetSceneManager()->AddSceneNode(Tree[i], true);

		Tree[i]->SetPosition(100.f, i*100.f, 0.f);
 		Tree[i]->SetUserLOD(true);
 		Tree[i]->SetLOD(1.0f);
		Tree[i]->SetScale(RVector(10, 10, 10));
	}

	Ground[0].x = 0;
	Ground[0].y = 0;

	Ground[1].x = 1000;
	Ground[1].y = 0;

	Ground[2].x = 1000;
	Ground[2].y = 1000;

	Ground[3].x = 0;
	Ground[3].y = 1000;

	Ground[0].z = Ground[1].z = Ground[2].z = Ground[3].z = 0;

	Ground[0].color = Ground[1].color = Ground[2].color = Ground[3].color = 0xFFFFFFFF;

	SetClearColor(0x000000ff);

	GetSceneManager()->GetResourceMgr<RTreeResourceMgr>()->SetWindDirection(RVector(0, 0, 0));
	GetSceneManager()->GetResourceMgr<RTreeResourceMgr>()->SetWindStrength(0);

	status = new RTextSceneNode;
	status->SetFont(m_pFont);
	status->SetPosition(10, 20);
	status->SetText("");
	status->SetColor(0xffff0000);
	GetSceneManager()->AddSceneNode(status);

	return true;
}

void myApplication::OnRender()
{
	m_pDevice->SetShader(0);
	m_pDevice->SetAlphaTestEnable(false);
	m_pDevice->SetBlending(RBLEND_NONE);
	m_pDevice->SetFvF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	m_pDevice->SetTransform(RST_WORLD, RMatrix::IDENTITY);
	RMatrix mat =  m_pDevice->GetTransform(RST_VIEW);
	mat =  m_pDevice->GetTransform(RST_PROJECTION);
	m_pDevice->SetCullMode(RCULL_NONE);
	m_pDevice->SetColorVertex(true);
	m_pDevice->SetLighting(false);
	m_pDevice->DrawPrimitiveUP(RPT_TRIANGLEFAN, 2, Ground, sizeof(Vert));
}


void myApplication::OnUpdate()
{
	sampleApp::OnUpdate();

}


void myApplication::OnDestroy()
{
	status->RemoveFromParent();
	delete status;
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	return g_App.Run();
}
