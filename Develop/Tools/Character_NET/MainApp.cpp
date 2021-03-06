#include "stdafx.h"
#include <windows.h>
#include <MMSystem.h>
#include <shlwapi.h>
#include <math.h>
#include <algorithm>
#include "MainApp.h"
#include "RConfiguration.h"
#include "ToolConfig.h"
#include "RDeviceD3D.h"
#include "RActor.h"
#include "RActorCollision.h"
#include "RMesh.h"
#include "RMeshNode.h"
#include "MDebug.h"
#include "RMeshMgr.h"
#include "RSceneManager.h"
#include "RApplication.h"
#include "RRenderHelper.h"
#include "RMeshUtil.h"
#include "CSItemData.h"
#include "RFont.h"
#include "RMaterial.h"
#include "REffectInstance.h"
#include "RSimpleRotationController.h"
#include "RTimer.h"
#include "RMaterialResource.h"
#include "RMaterialResourceMgr.h"
#include "RAnimationResource.h"
#include "RRenderingStrategy.h"

#include "MFileSystem.h"

#include "THitAreaRenderer.h"
#include "TMeshInfo.h"
#include "TPlayMotion.h"
#include "TNPCInfo.h"
#include "TNaming.h"
#include "TTalentInfo.h"
#include "TDef.h"
#include "TTalentEffectMgr.h"
#include "TTalentEventMgr.h"
#include "TTalentHelper.h"

#include "XMotion.h"
#include "XMotionMgr.h"
#include "Xmlseral_Talent.h"

//#include "MPhysX.h"
//#include "MPhysX4R3/MPhysX4R3.h"

#include "MMemoryLeak.h"
#include "TEquipment.h"

#include "TTalentActAnimationTime.h"

#include "TRealSoundLoader.h"

#include "EffectSelectForm.h"
#include "TEffectCharacterToolInvoker.h"

#include "OverlayAnimationTable.h"
#include "AnimationBlendTable.h"
#include "ReActionAnimationTable.h"
#include "NPCAnimationInfoSetting.h"

#include "SearchCustomEffectInfoName.h"

//#define VLD_CHECK_LEAK

#ifdef _DEBUG
	#ifdef VLD_CHECK_LEAK
		#define VLD_MAX_DATA_DUMP		0xFF		// vld에서 데이터 덤프는 FF Byte만 해랑.
		#include "../../../sdk/vld/vld.h"
		#pragma comment(lib,"vldmt.lib")
	#endif
#endif

using namespace rs3;
using namespace System;

CMainApplication *g_pMainApp = NULL;

RDeviceD3D*  GetDevice()
{
	return g_pMainApp->GetDevice();
}

TreeNode^ SearchNode(TreeNodeCollection^ nodes, String^ str)
{
	TreeNode^ tn = nullptr;
	System::Collections::IEnumerator^ myNodes = (safe_cast<System::Collections::IEnumerable^>(nodes))->GetEnumerator();
	while ( myNodes->MoveNext() )
	{
		tn = safe_cast<TreeNode^>(myNodes->Current);
		String^ tnstr = (safe_cast<TreeNode^>(myNodes->Current))->Text;
		//if( String::Compare(tnstr, str) == 0)
		if( str->StartsWith(tnstr) &&
			str->EndsWith(tnstr))
		{
			return tn;
		}
		else
		{
			tn = SearchNode( tn->Nodes, str);
			if(tn != nullptr)
				return tn;
		}
	}

	return tn;
}

//////////////////////////////////////////////////////////////////////////
CMainApplication::CMainApplication()
: m_pDocument(NULL)
, m_pCmdBuffer(NULL)
, m_pDevice(NULL)
, m_pActor(NULL)
//, m_pPhx(NULL)
, m_pSelectedActorNode(NULL)
//, m_bShowBones(false)
//, m_bVertexNormal(false)
//, m_bShowSpecular(true)
//, m_bShowAABB(false)
//, m_bShowWire(false)
//, m_pTextFont(NULL)
//, m_pTextNode(NULL)
//, m_FillMode(RFILL_SOLID)
, m_bLightToCamerapos(false)
, m_time(10000)
, m_pRotationControllerHorizonal(new RRotationController[2])
, m_pRotationControllerVertical(new RRotationController[2])
, m_bRenderActorCollision(false)
, m_bFromMoveCheck(false)
, m_pMotion(NULL)
, m_pCamera(NULL)
, m_pLight(NULL)
, m_pTalentInfo(NULL)
, m_bLighting(false)
, m_Ambient(0.3f, 0.3f, 0.3f, 1.f)
, m_bLoopAnimation(false)
, m_vTop(0.f, 0.f, 0.f)
, m_vBottom(0.f, 0.f, 0.f)
, m_pTalentEffectMgr(NULL)
, m_pEventMgr(NULL)
, m_pEffectInfoMgr(NULL)
//, m_pPaneMgr(NULL)
, m_pFont(NULL)
, m_bMoveDummy(true)
, m_bShowEffect(true)
, m_bShowPartsHitCapsule(false)
, m_bShowPartsHitCapsuleLinkedHitEffect(false)
, m_bShowMeshColCapsule(false)
, m_bShowMeshHitEffectCapsule(false)
, m_SelectedTab(TAB_MTRLSTATE_NONE)
, m_SelectedTab2(TAB_TABSTATE_NONE)
, m_bShowGrid(true)
, m_pEffectManager(NULL)
, m_bTransitionBlending(false)
, m_bTalentAnimation(false)
, m_bCameraMove(false)
, m_pSampleActor(NULL)
, m_pActorMgr(NULL)
, m_bPlayPauseButtonStateByShourtCut(false)
, m_bShowCenterAxis(false)
, m_bShowWaterEffect(false)
, m_nWeaponEffectBuffID(0)
, m_nPreFrame(0)
, m_bShowNPCHitCapsuleGroup(false)
{
	//MInitLog(MLOG_LEVEL_INFORMATION, MLOG_FILE|MLOG_DEBUG);
	//	MInitLog(MLOG_LEVEL_DEBUG, MLOG_FILE|MLOG_DEBUG | MLOG_PROG,McvLog);

	//	g_pDefaultLogger->SetCustomOutputCallback(McvLog);

	//_CrtSetBreakAlloc(19896);	
}

CMainApplication::~CMainApplication()
{
	delete []m_pRotationControllerHorizonal;
	delete []m_pRotationControllerVertical;
	//	MFinalLog();
}

void CMainApplication::Destroy()
{
	CheckCustomMotion();

	if(!GlobalObjects::g_SaveNotifier.IsSaveDone())
	{
		if (System::Windows::Forms::MessageBox::Show("작업한내용이 저장되지 않았습니다. 저장하시겠습니까?", "저장"
			,MessageBoxButtons::YesNo) == System::Windows::Forms::DialogResult::Yes)
		{
			SaveAll();
		}
		else
		{
			GlobalObjects::g_SaveNotifier.ResetSaveAll();
		}
	}

	//------------------------------------------------------------------------
	// 저장 할거 하기
	// 탤런트 이펙트, 이벤트, 충돌 정보 저장
	//DestroySaveTalentExt();

	//------------------------------------------------------------------------
	m_vItemModelNames.clear();

	GlobalObjects::g_pMainForm->m_TalentTabPage->m_TalentHitTabController->ClearQueue();

	// 검광 삭제
	m_SwordTrailSampling.DestroySwordTrailSampling();
	//if( m_pTextNode )
	//{
	//	m_pTextNode->RemoveFromParent();
	//	SAFE_DELETE(m_pTextNode);
	//}

	//if( m_pTextFont)
	//{
	//	//m_pTextFont->Finalize(); //Finalize는 RSceneManager에서 한번 호출됩니다.
	//	SAFE_DELETE(m_pTextFont);
	//}

	m_MagicArea.OnDestroy();

	SAFE_DELETE(m_pEffectManager);

	if( m_ReferenceMesh.m_pReferenceMesh )
	{
		GetSceneMgr()->RemoveSceneNode(m_ReferenceMesh.m_pReferenceMesh);
		SAFE_DELETE(m_ReferenceMesh.m_pReferenceMesh);
	}
	

	//m_pFont->Finalize(); //Finalize는 RSceneManager에서 한번 호출됩니다.
	SAFE_DELETE( m_pFont );
	SAFE_DELETE(m_pMotion);

	//for(ANIINFOVECTOR::iterator it = m_vecAniIfo.begin(); it != m_vecAniIfo.end(); it++)
	//{
	//	SAFE_DELETE(*it);
	//}
	//m_vecAniIfo.clear();

	//REffectMgr::GetInstance()->Destroy();

	DelProjectile();

	//m_Camera.RemoveFromParent();
	if( m_pCamera)
	{
		m_pCamera->RemoveFromParent();
		SAFE_DELETE(m_pCamera);
	}
	if (m_pLight)
	{
		SAFE_DELETE(m_pLight);
	}

	m_CameraEffect.DeleteCameraEffect();

	DeleteActor();
	SAFE_DELETE(m_pActorMgr);

	m_ItemMgr.Destroy();

	RDirectionalLightSceneNode* pDirLight = m_pSceneManager->GetDefaultLight();
	if(pDirLight)
	{
		pDirLight->RemoveFromParent();
		SAFE_DELETE(pDirLight);
	}

	//m_pSceneManager->RemoveChild(m_pTextNode);
	//SAFE_DELETE(m_pTextNode);
	//m_pFont->Finalize();
	//SAFE_DELETE(m_pFont);

	//if(m_pWeapon)
	//{
	//	m_pWeapon->RemoveFromParent();
	//	m_pWeapon->Destroy();
	//}
	//SAFE_DELETE(m_pWeapon);

	// Material Resource 삭제
	//m_MaterialTable.clear();
	//	for(int i = 0; i < (int)m_MaterialTable.size(); ++i)
	//	{
	//		m_pSceneManager->ReleaseResource( m_MaterialTable[i] );
	//	}

// 	m_pSceneManager->GetResourceMgr<REffectResourceMgr>()->ClearCache();

	//if (m_pPhx)
	//{
	//	m_pPhx->Destroy();
	//	delete m_pPhx;
	//	m_pPhx = NULL;
	//}

	m_TMenu.Destroy();

	DestroySceneManager();

	//	RMeshMgr::GetInstance()->DelAll();
	//	RMeshMgr::GetInstance()->Destroy();

	if(m_pDevice)
	{
		m_pDevice->Destroy();
	}
	SAFE_DELETE(m_pDevice);
	REngine::RegisterDevice(NULL);

	//SAFE_DELETE(m_pBreakablePartsManager);
	SAFE_DELETE(m_pTalentEffectMgr);
	SAFE_DELETE(m_pEventMgr);
	SAFE_DELETE(m_pEffectInfoMgr);
}

void CMainApplication::DestroySceneManager()
{
	if(m_pSceneManager)
	{
		REngine::DestroySharedResource();
		m_pSceneManager->Destroy();
	}
	SAFE_DELETE(m_pSceneManager);
	REngine::RegisterSceneManager(NULL);
}

string CMainApplication::SetCurrentDir()
{
	// Current Directory를 맞춘다.
	TCHAR szModuleFileName[_MAX_DIR] = {0,};
	GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);
	PathRemoveFileSpec(szModuleFileName);
	SetCurrentDirectory(szModuleFileName);

	return szModuleFileName;
}

bool CMainApplication::Init(HWND hWnd)
{
	DWORD dwTime, dwFullTime;
	CHECK_LOADING_TIME_START(dwFullTime);
	MInitLog(MLOG_LEVEL_DEBUG, MLOG_FILE|MLOG_DEBUG | MLOG_PROG,McvLog);

	// Initialize file system
	if( rs3::REngine::IsDevelopVersion() )
	{
		MCreateFileSystem( MFILEACCESS_GENERIC, "../../EngineResDev;../../Data/");
	}
	else
	{
		MCreateFileSystem( MFILEACCESS_GENERIC, "../../EngineRes;../../Data/");
	}

	TStrings::Init();

	m_pMotion	=	new TPlayMotion();

	string strCurrentDir = SetCurrentDir();

	//load config
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Load Config");
	CToolConfig::GetInstance().LoadConfig();
	CToolConfig::GetInstance().LoadSysConfig();

	CHECK_LOADING_TIME_START(dwTime);
	if(!InitDevice(hWnd))
	{
		GlobalObjects::g_pMainForm->m_InitLoadingWindow.Close();
		::MessageBox(NULL,"디바이스 혹은 공유자원 생성 실패.","에러",MB_OK);
		return false;
	}
	CHECK_LOADING_TIME_END(dwTime, "InitDevice");

	CHECK_LOADING_TIME_START(dwTime);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Init Model List");
	m_TabModel.InitModelList();
	GlobalObjects::g_pMainForm->m_ModelTabPage->SetModelTreeView();
	CHECK_LOADING_TIME_END(dwTime, "m_TabModel.InitModelList()");

	CHECK_LOADING_TIME_START(dwTime);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Init Item List");
	m_TabItem.InitItemMap();
	GlobalObjects::g_pMainForm->m_ItemTabPage->InitItemList();
	CHECK_LOADING_TIME_END(dwTime, "m_TabItem.InitItemMap()");

	SetActorCollision();

	CHECK_LOADING_TIME_START(dwTime);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Load Talent Info");
	m_TTalentInfoMgr.Load();
	//XTalentInfoMgr::GetInstance().Load(FILENAME_TALENT_TOOL
	//, FILENAME_TALENT_EXT_TOOL, FILENAME_TALENT_HIT_INFO);
	CHECK_LOADING_TIME_END(dwTime, "m_TTalentInfoMgr.Load()");

	CHECK_LOADING_TIME_START(dwTime);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Create RealSound3");
	m_pSoundMgr.Create();
	CHECK_LOADING_TIME_END(dwTime, "m_pSoundMgr.Create()");

	CHECK_LOADING_TIME_START(dwTime);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Load Sound Info");
	TRealSoundLoader _loader;
	_loader.LoadSamples( FILENAME_SOUNDLIST_TOOL);
	CHECK_LOADING_TIME_END(dwTime, "TRealSoundLoader::LoadSamples()");

	CHECK_LOADING_TIME_START(dwTime);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Load Mesh Info");
	TMeshInfoMgr::GetInstance().LoadMeshInfo();
	CHECK_LOADING_TIME_END(dwTime, "MeshInfoMgr::GetInstance().LoadMeshInfo()");

	CHECK_LOADING_TIME_START(dwTime);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Load NPC Info");
	TNPCInfoMgr::GetInstance().Load();
	CHECK_LOADING_TIME_END(dwTime, "TNPCInfoMgr::GetInstance().Load()");

	//InitTalentList();

	CHECK_LOADING_TIME_START(dwTime);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Load Buff Info");
	m_BuffInfo.Init();
	CHECK_LOADING_TIME_END(dwTime, "InitBuffList()");

	CHECK_LOADING_TIME_START(dwTime);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Load Motion Info");
	InitMotionList();
	CHECK_LOADING_TIME_END(dwTime, "InitMotionList()");

	//오른쪽에 필요한 탭만 나오게 하려구 오른쪽탭을 한번씩 선택해 준다.
	GlobalObjects::g_pMainForm->tabControl1->SelectedIndex = 1;
	GlobalObjects::g_pMainForm->tabControl1->SelectedIndex = 0;

	// 탤런트 이펙트 정보로딩
	// 미리 저장하고 있는다.
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Load Talent Effect Info");
	m_pTalentEffectMgr = new TTalentEffectMgr(&m_TTalentInfoMgr);

	m_pEventMgr = new TTalentEventMgr;

	GlobalObjects::g_pMainForm->InitLoadingInfoText("Load Effect Info");
	m_pEffectInfoMgr = new TEffectInfoMgr;

	m_pEffectManager = new TEffectManager;

	m_EffectViewControl.SetEffectManager(m_pTalentEffectMgr);

	GlobalObjects::g_pMainForm->InitLoadingInfoText("Load Camera Effect Info");
	m_CameraEffect.LoadCameraEffect(PATH_SFX_TOOL_CAMERA);

	GlobalObjects::g_pMainForm->InitLoadingInfoText("Init Talent Event Controller");
	GlobalObjects::g_pMainForm->m_EventTabPage->m_TalentEventController->Init(m_pEventMgr);

	GlobalObjects::g_pMainForm->InitLoadingInfoText("Init Talent Hit TabController");
	GlobalObjects::g_pMainForm->m_TalentTabPage->m_TalentHitTabController->Init();

	GlobalObjects::g_pMainForm->InitLoadingInfoText("Init Mesh Hit TabController");
	GlobalObjects::g_pMainForm->m_MeshHitTabController->Init(&m_MeshColInfoViewControl, &m_PartsHitInfoViewControl, &m_MeshHitEffectInfoViewControl);

	GlobalObjects::g_pMainForm->InitLoadingInfoText("Init Talent Effect TabController");
	GlobalObjects::g_pMainForm->m_TalentEffectTabController->Init(m_pTalentEffectMgr, &m_EffectViewControl);

	//CHECK_LOADING_TIME_START(dwTime);
	//GlobalObjects::g_pMainForm->InitLoadingInfoText("Load Sword Trail Sampling Info");
	//m_SwordTrailSampling.Load(FILENAME_SWORD_TRAIL_SAMPLING_TOOL);
	//CHECK_LOADING_TIME_END(dwTime, "m_SwordTrailSampling.Load()");

	m_TalentAnimation.Init(m_pMotion);
	m_NormalAnimation.Init(m_pMotion);

	CHECK_LOADING_TIME_START(dwTime);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Create TActorMgr");
	m_pActorMgr = new TActorMgr(m_pSceneManager, &m_eventListener);
	CHECK_LOADING_TIME_END(dwTime, "new TActorMgr");

	CHECK_LOADING_TIME_START(dwTime);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Init Ani Info");
	m_TabAnimation.InitAniInfo();
	CHECK_LOADING_TIME_END(dwTime, "m_TabAnimation.InitAniInfo()");

	m_EquipItems.SetEquipItemData(&m_ItemMgr, &m_TabModel, &m_ClothMgr);

	CHECK_LOADING_TIME_END(dwFullTime, "--------- Full Init Time");

	TCONST::Init();

	return true;
}

void CMainApplication::InitMotionList()
{
	m_pMotion->Clear();

	//bool bLoad = XMotionMgr::GetInstance().Load("../../Data/System/Motion.xml", NULL);
	//if(!bLoad) return;
	SetCurrentDirectory("../../");

	bool bLoad = m_pMotion->Init();
	if( !bLoad ) return;

	SetCurrentDir();
}

//애니메이션이 존재하는 지 유무(가지고 있는지 or 리스트에 있는지.
bool CMainApplication::IsAnimation(System::String^ str)
{
	if( m_pActor && m_pActor->m_pMesh)
	{
		RMeshAnimationMgr* pAMgr = &m_pActor->m_pMesh->m_AniMgr;

		for(RAnimationHashList_Iter it = pAMgr->m_NodeHashList.begin(); it != pAMgr->m_NodeHashList.end();it++)
		{
			RAnimation* pAnimation = *it;
			if(pAnimation)
			{
				String^ Name = gcnew String( pAnimation->GetAliasName().c_str() );
				if( Name->StartsWith(str))
					return true;
			}
		}
	}

	return false;
}

bool CMainApplication::InitSceneManager()
{
	if (m_pDevice == NULL) return false;

	m_pSceneManager = new RSceneManager;
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Register RSceneManager");
	REngine::RegisterSceneManager(m_pSceneManager);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Create RSceneManager");
	bool bRet = m_pSceneManager->Create(m_pDevice);
	if(!bRet) return false;

	// 기본 광원 추가
	RDirectionalLightSceneNode* pDirLight = m_pSceneManager->GetDefaultLight();
	if(!pDirLight)
	{
		pDirLight = new RDirectionalLightSceneNode();
		m_pSceneManager->AddSceneNode(pDirLight);

		pDirLight->SetLightAmbient(1,1,1,1);
		pDirLight->SetLightDiffuse(0,0,0,0);
 		pDirLight->SetSkyColor(RVector3(0,0,0));
 		pDirLight->SetGroundColor(RVector3(0,0,0));
		m_bLighting = false;
	}

	if( m_pSceneManager->GetRenderingStrategy())
		m_pSceneManager->GetRenderingStrategy()->SetBackgroundBufferDivideSize(1);

	return true;
}

bool CMainApplication::InitDevice(HWND hWnd)
{
	RECT rect;
	GetClientRect(hWnd,&rect);

	int nWidth = (int)(rect.right - rect.left);
	int nHeight = (int)(rect.bottom - rect.top);

	REngine::GetConfig().m_nWidth = nWidth;
	REngine::GetConfig().m_nHeight = nHeight;

	DWORD dwTest;
	CHECK_LOADING_TIME_START(dwTest);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Create RDeviceD3D");
	m_pDevice = new RDeviceD3D();
	bool bRet = m_pDevice->Create(hWnd);
	_ASSERT(bRet);
	CHECK_LOADING_TIME_END(dwTest, "  -- m_pDevice->Create");
	CHECK_LOADING_TIME_START(dwTest);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Register RDeviceD3D");
	REngine::RegisterDevice(m_pDevice);
	CHECK_LOADING_TIME_END(dwTest, "  -- RegisterDevice()");

	CHECK_LOADING_TIME_START(dwTest);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Init SceneManager");
	bRet = InitSceneManager();
	if(!bRet) return false;
	CHECK_LOADING_TIME_END(dwTest, "  -- InitSceneManager()");

	CHECK_LOADING_TIME_START(dwTest);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Init Shared Resource");
	bRet = REngine::InitSharedResource();
	if(!bRet) return false;
	CHECK_LOADING_TIME_END(dwTest, "  -- REngine::InitSharedResource()");

// 	m_pSceneManager->GetResourceMgr<REffectResourceMgr>()->SetCacheEnable(true);

	CHECK_LOADING_TIME_START(dwTest);
	InitCamera();
	CHECK_LOADING_TIME_END(dwTest, "  -- InitCamera()");

	// TODO: 이건 뭐지....
	CHECK_LOADING_TIME_START(dwTest);
	m_pDevice->SetAmbient(D3DCOLOR_COLORVALUE( 0.5f, 0.5f, 0.5f, 1.0f ));
	m_pDevice->SetTextureFilter(0,RTF_BILINEAR);
	m_pDevice->SetTextureMipmapLodBias(0,-0.5f);
	CHECK_LOADING_TIME_END(dwTest, "  -- 이건 뭐지()");

	CHECK_LOADING_TIME_START(dwTest);
	InitFont();
	CHECK_LOADING_TIME_END(dwTest, "  -- InitFont()");

	THitAreaRenderer::GetInstance().m_pDevice = m_pDevice;

	CHECK_LOADING_TIME_START(dwTest);
	InitPhysx();
	CHECK_LOADING_TIME_END(dwTest, "  -- InitPhysx()");

	return true;
}

void CMainApplication::InitPhysx()
{
	// init physx
	//m_pPhx = new physx::MPhysX;
	//m_pPhx->Init(10.f, 0, "localhost");
	//m_pPhx->SetSceneGravity(-1400.f);
}

void CMainApplication::InitCamera()
{
	DWORD dwCheck;
	CHECK_LOADING_TIME_START(dwCheck);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Create Camera");
	m_pCamera = new TCamera;
	
	m_pCamera->SetPostEffect();
	
	m_pSceneManager->AddSceneNode(m_pCamera);
	CHECK_LOADING_TIME_END(dwCheck, "  -- -- m_pSceneManager->AddSceneNode()");

	CHECK_LOADING_TIME_START(dwCheck);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Set Primary Camera");
	m_pSceneManager->SetPrimaryCamera(m_pCamera);
	CHECK_LOADING_TIME_END(dwCheck, "  -- -- m_pSceneManager->SetPrimaryCamera()");

	CHECK_LOADING_TIME_START(dwCheck);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Reset Camera");
	m_pCamera->Reset();
	CHECK_LOADING_TIME_END(dwCheck, "  -- -- m_pCamera->Reset()");

	CHECK_LOADING_TIME_START(dwCheck);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("ResetSize Camera");
	m_pCamera->ResetSize();
	CHECK_LOADING_TIME_END(dwCheck, "  -- -- m_pCamera->ResetSize()");

	//m_pSceneManager->AddChild(&m_Camera);
	//m_pSceneManager->SetPrimaryCamera(&m_Camera);
	SetClearClolor( D3DCOLOR_ARGB(255, 128, 128, 128));

	//m_pCamera->Reset();

	CHECK_LOADING_TIME_START(dwCheck);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Create Light");
	m_pLight = new TCamera;
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Reset Light");
	m_pLight->Reset();
	CHECK_LOADING_TIME_END(dwCheck, "  -- -- m_pLight->Reset()");
	CHECK_LOADING_TIME_START(dwCheck);
	GlobalObjects::g_pMainForm->InitLoadingInfoText("ResetSize Light");
	m_pLight->ResetSize();
	CHECK_LOADING_TIME_END(dwCheck, "  -- -- m_pLight->ResetSize()");
}

void CMainApplication::InitFont()
{
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Create Font");
	m_TMenu.InitFont();

	// 폰트
	m_pFont = new RFont;
	m_pFont->Create(GetDevice(),L"굴림",24);
}

void CMainApplication::OnResetDevice()
{
	m_pDevice->ResetDevice();
}

void CMainApplication::SetLightAmbient(const RVector4& light)
{
	m_Ambient	= light;

	RDirectionalLightSceneNode* pDirLight = m_pSceneManager->GetDefaultLight();
	if(pDirLight)
	{
		pDirLight->SetLightAmbient(m_Ambient);
	}
}

void CMainApplication::OnOffLight()
{
	m_bLighting = !m_bLighting;

	if( m_bLighting )
		OnDirLight();
	else
		OffDirLight();
}

void CMainApplication::OnDirLight()
{
	GlobalObjects::g_pMainForm->InitLoadingInfoText("On Dir Light");

	RDirectionalLightSceneNode* pDirLight = m_pSceneManager->GetDefaultLight();
	if(!pDirLight)
		return;

	pDirLight->SetLightAmbient(m_Ambient);
	pDirLight->SetLightDiffuse(1,1,1,1);

	// 랜더링 전략에 라이팅 사용 유무를 설정
	m_pSceneManager->UseLighting( true);

	GlobalObjects::g_pMainForm->lightToolStripMenuItem->Checked = true;
	GlobalObjects::g_pMainForm->LighrtoolStripButton->Checked = true;
}

void CMainApplication::OffDirLight()
{
	GlobalObjects::g_pMainForm->InitLoadingInfoText("Off Dir Light");
	RDirectionalLightSceneNode* pDirLight = m_pSceneManager->GetDefaultLight();
	if(!pDirLight)
		return;

	pDirLight->SetLightAmbient(1,1,1,1);
	pDirLight->SetLightDiffuse(0,0,0,0);

	// 랜더링 전략에 라이팅 사용 유무를 설정
	m_pSceneManager->UseLighting( false);

	GlobalObjects::g_pMainForm->lightToolStripMenuItem->Checked = false;
	GlobalObjects::g_pMainForm->LighrtoolStripButton->Checked = false;

	SetLightToCamera(false);
}

bool CMainApplication::CorrectXml(tstring& str)
{
	MXml xml;

	if ( !xml.LoadFile(str.c_str()))
	{
		_ASSERT(0);			// 일단 디버그모드에선 assert 걸어둠
		return false;
	}

	return true;
}

// 특정 액터노드를 선택한다
bool CMainApplication::SelectActorNode(RActorNode* pActorNode)
{
	if(pActorNode == NULL)
		return false;

	int nID = pActorNode->m_pMeshNode->m_nID;

	TreeNode^ node = SearchNode(GlobalObjects::g_pMainForm->m_ModelInfoTabPage->BiptreeView->Nodes, nID);
	GlobalObjects::g_pMainForm->m_ModelInfoTabPage->BiptreeView->SelectedNode = node;

	m_pSelectedActorNode = pActorNode;

	return true;
}

void CMainApplication::SelectItemInItemList(int slot)
{
	m_TabItem.SelectItem(slot);
}

bool CMainApplication::SetAnimation( string strAniName )
{
	if(m_pActor)
	{
		m_nPreFrame = 0;
		m_bPlayPauseButtonStateByShourtCut = true;
		return m_pActor->SetAnimation(strAniName, true);
	}

	return false;
}

void CMainApplication::SetHDREnable(bool bEnable)
{
	if( m_pActor )
	{
		rs3::REngine::GetSceneManagerPtr()->SetHDREnable(bEnable);
	}
}

void CMainApplication::CameraReset()
{
	if( m_pActor )
	{
		m_pCamera->Reset();
		//m_pActor->OnUpdateAABB();
		m_pActor->UpdateBoundingVolume();
		rs3::RBoundingBox box = m_pActor->GetAABB();
		m_pCamera->m_fDist = box.maxz + (box.maxz/2);
		m_pCamera->m_vTargetPos.z = (box.maxz/2);
		m_pCamera->UpdateData();
	}
}

void CMainApplication::Reset()
{
	if( m_pActor )
	{
		RMatrix mat;
		mat.MakeIdentity();
		m_pActor->SetTransform(mat);
	}

	CameraReset();
}

void CMainApplication::InitState(int nWidth, int nHeight, bool bReset)
{
	if(nWidth == 0 || nHeight == 0) return;

	// TODO: fov 맞추기
	//m_pCamera->SetFov(CToolConfig::GetInstance().CAMERA_FOV);
	m_pCamera->SetFov(CToolConfig::GetInstance().CAMERA_FOV,
		(float)nWidth/(float)nHeight);

	//m_pCamera->UpdateData();

	//	if(bReset)	m_Camera.Reset();

	//assert(m_lpd3ddevice!=NULL);

	//D3DXMATRIX matProj;
	//D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI/4.0f, (float)nWidth/(float)nHeight, 3.0f, 5000.0f);

	//D3DXMATRIX matIdentity;
	//D3DXMatrixIdentity(&matIdentity);

	//m_lpd3ddevice->SetTransform( D3DTS_PROJECTION,&matProj);
	//m_lpd3ddevice->SetTransform( D3DTS_VIEW,&matIdentity);
}

void CMainApplication::OnLostDevice()
{
}

string CMainApplication::GetTalentAniName( string szName, int nTalentID )
{
	// PC 이면
	if(nTalentID < MIN_NPC_TALENT_ID)
	{
		//int vv = szName.find("_");	// 파일자체에 무기명이 있느냐?
		//if (vv == -1)
		{
			WEAPON_TYPE weapon_type = WEAPON_NONE;

			if (GetActor())
			{
				weapon_type = GetActor()->GetCurrentWeaponType();
			}

			return string(TStrings::WeaponType(WEAPON_TYPE(weapon_type))) + string("_") + string(szName);
		}
	}

	//if (pTalentInfo && pTalentInfo->m_setNPCID.size() != 0)
	//{
	//	return szName;
	//}

	return szName;
}

void CMainApplication::EventMessage(int index, System::String^ str1 /* = nullptr */)
{
	if(m_pActor == NULL) return;

	switch(index)
	{
	case EVENT_MSG_STOP_ANIMATION:
		{
			//m_pActor->Stop();
			OnStopAnimation();
		}
		break;
	case EVENT_MSG_PLAY_ANIMATION:
		{
			m_pActor->ReSetAnimation();

			UpdateModelScale();
		}
		break;
	case EVENT_MSG_LIGHT_TO_CAMERA:
		{
			SetLightToCamera(!m_bLightToCamerapos);
		}
		break;
	default:
		{
			_ASSERT(0);
		}
		break;
	}
}

void CMainApplication::SetLightToCamera(bool bLightToCamera)
{
	m_bLightToCamerapos = bLightToCamera;

	if(m_bLightToCamerapos)
	{
		OnDirLight();

		GlobalObjects::g_pMainForm->lightToCameraposToolStripMenuItem->Checked = true;
		GlobalObjects::g_pMainForm->LightToCameratoolStripButton1->Checked = true;
	}
	else
	{
		GlobalObjects::g_pMainForm->lightToCameraposToolStripMenuItem->Checked = false;
		GlobalObjects::g_pMainForm->LightToCameratoolStripButton1->Checked = false;
	}
}

RActorNode*	CMainApplication::GetActorNode(string strBoneName)
{
	if(m_pActor == NULL)
	{
		return NULL;
	}

	RActorNode* node = m_pActor->GetActorNode(strBoneName.c_str());
	return node;
}

/************************************************************************/
/* 노드에서 str 노드 찾기                                               */
/************************************************************************/
TreeNode^ CMainApplication::SearchNode(TreeNodeCollection^ nodes, String^ str)
{
	TreeNode^ tn = nullptr;
	System::Collections::IEnumerator^ myNodes = (safe_cast<System::Collections::IEnumerable^>(nodes))->GetEnumerator();
	while ( myNodes->MoveNext() )
	{
		tn = safe_cast<TreeNode^>(myNodes->Current);
		String^ tnstr = (safe_cast<TreeNode^>(myNodes->Current))->Text;
		//if( String::Compare(tnstr, str) == 0)
		if( str->StartsWith(tnstr) &&
			str->EndsWith(tnstr))
		{
			return tn;
		}
		else
		{
			tn = SearchNode( tn->Nodes, str);
			if(tn != nullptr)
				return tn;
		}
	}

	return tn;
}

/************************************************************************/
/* tree에서 id로 노드 찾기                                               */
/************************************************************************/
TreeNode^ CMainApplication::SearchNode(TreeNodeCollection^ nodes, int nID)
{
	TreeNode^ tn = nullptr;
	System::Collections::IEnumerator^ myNodes = (safe_cast<System::Collections::IEnumerable^>(nodes))->GetEnumerator();
	while ( myNodes->MoveNext() )
	{
		tn = safe_cast<TreeNode^>(myNodes->Current);
		int nNodeID = (int)tn->Tag;

		if( nNodeID == nID)
		{
			return tn;
		}
		else
		{
			tn = SearchNode( tn->Nodes, nID);
			if(tn != nullptr)
				return tn;
		}
	}

	return tn;
}

/************************************************************************/
/* 현재 액트를 해제하면서 player(만)의 장착 아이템을 저장.              */
/************************************************************************/
void CMainApplication::DeleteActor()
{
	if(m_pActor)
	{
		m_pSelectedActorNode = NULL;

		string name = GetPureFileName(m_pActor->m_pMesh->GetName());
		if( name == HUMAN_FEMALE || name == HUMAN_MALE )
		{
			m_EquipItems.SaveEquipItems();
		}

		BasicMesh();

		m_pActor = NULL;
	}
}

/*
* elu 파일 로드	
*/
RActor* CMainApplication::eluFileOpen(const char* FileName)
{
	RActor* pActor = new RActor;
	//	pActor->SetUsingPassRenderer(false);
	if( !pActor->Create(FileName))
	{
		SAFE_DELETE(pActor);
	}

	return pActor;
}

bool CMainApplication::CreateActor( const char* FileName )
{
	m_pActor = m_pActorMgr->ModelLoad(FileName);

	m_EquipItems.SetActor(m_pActor);
	m_pMotion->SetActor(m_pActor);
	m_NormalAnimation.SetActor(m_pActor);
	m_TalentAnimation.SetActor(m_pActor);
	m_AnimationBledTable.SetActor(m_pActor);

	if(m_pActor == NULL)
	{
		return false;
	}

	m_EquipItems.CreateClothMenuForNPC();

	// 브레이커블 파츠 셋팅
	//CheckBreakableParts();

	// 페이크 비튼 셋팅
	tstring strBeatenAniName = TCONST::HIT_FAKE_BEATEN_DEFAULT_ANI_NAME;
	m_pActor->InitFakeBeatenAnimation(strBeatenAniName, TCONST::HIT_FAKE_BEATEN_ANIMATION_BLEND_WEIGHT, TCONST::HIT_FAKE_BEATEN_ANIMATION_BLEND_SPEED);

	return true;
}

bool CMainApplication::CharFileOpen(const char* FileName)
{
	m_eventListener.StopAnimationEffect();

	DWORD dwStartTime = GetTickCount();

	SetProgressBarValue();

	DeleteActor();

	ShowHideSamplePlayerModel(false);
	
	m_TabAnimation.Init();

	if (CreateActor(FileName) == false) return false;

	SetProgressBarValue(30);

	mlog("정보 : Mesh파일(%s) 로드 성공 \r", FileName);

	m_pActor->SetPosition(RVector(0,0,0));
	//by pok, 씬매니저 변경중
	//m_pSceneManager->AddSceneNode(m_pActor);

	//CameraReset();

	GlobalObjects::g_pMainForm->m_ModelInfoTabPage->InitModelInfoTab(GetRemovePathName(FileName).c_str());

	SetProgressBarValue(50);

	m_TabAnimation.InitAnitreeView();

	SetProgressBarValue(70);

	// [3/23/2007 madduck] - 애니메이션 리스트를 가지고 파일이 있냐 없냐.
	// [4/18/2007 madduck] - 없던걸로 그냥 두자...
	if( GlobalObjects::g_pMainForm->m_ParentName != "")
		m_TabAnimation.SetAniInfo(GlobalObjects::g_pMainForm->m_ParentName);


	SetProgressBarValue(90);

	m_EquipItems.LoadEquipItems(m_pActor);

	SetProgressBarValue(100);

	return true;
}

bool CMainApplication::MeshFileOpen(const char* FileName)
{
	int len = strlen(FileName);

	if(!ExistFile(FileName))
	{
		mlog("에러 : 파일이 존재하지 않습니다. - %s \r", &FileName[0]);
		return false;
	}

	if(GetActor() && GetActor()->m_pMesh)
	{
		if(GetActor()->m_pMesh->GetName() == FileName)
			return true;
	}

	if(strncmp(&FileName[len-4],".elu",4)==0) {
		return CharFileOpen(FileName);
	}
	else if(strncmp(&FileName[len-4],".ani",4)==0) {
		return m_TabAnimation.LoadAniFile(FileName);
	}

	mlog("에러 : 지원하지 않는 파일 형식입니다. - %s \r", &FileName[len-4]);

	return false;
}

bool CMainApplication::MeshFileOpen(System::String^ strFileName)
{
	SetProgressBarValue();

	m_pTalentInfo = NULL;

	int size = strFileName->Length;

	const char* filename= MStringToCharPointer(strFileName);
	bool bLoad = MeshFileOpen(filename);
	MFreeCharPointer(filename);

	if( !bLoad )
	{
		return false;
	}

	m_TabModelInfo.SetBipTreeView();

	// 재질 조합
	m_Mtrl.SetMeshTreeView();
	m_Mtrl.SetMtrlList();

	SetProgressBarValue(100);

	return true;
}

void CMainApplication::DrawBoxSelectedNode()
{
	// 원점 표시
	if(m_pActor && m_bShowCenterAxis)
	{
		RRenderHelper::SetRenderState();

		RBoundingBox box = RBoundingBox(RVector(-5,-5,-5),RVector(5,5,5));
		RMatrix matTM = m_pActor->GetWorldTransform();
		RRenderHelper::RenderBox(matTM,box,0xffffffff);
		RRenderHelper::RenderAxis(matTM,30,0.5f);

		RenderAxisText(matTM, 30);
	
		RRenderHelper::EndRenderState();
	}

	//RActorNode* pDummyActorNode = GetActorNode("dummy_loc");
	//if(pDummyActorNode)
	//{
	//	RRenderHelper::SetRenderState();

	//	//m_pDevice->SetDepthEnable(false,false);
	//	pDummyActorNode->ForceUpdateAnimation();
	//	RBoundingBox box = pDummyActorNode->m_pMeshNode->m_boundingBox;
	//	if(!pDummyActorNode->m_pMeshNode->isRenderAble())
	//		box = RBoundingBox(RVector(-5,-5,-5),RVector(5,5,5));
	//	RMatrix matTM = pDummyActorNode->GetLocalTransform();
	//	RRenderHelper::RenderBox(matTM,box,0xffffffff);
	//	RRenderHelper::RenderAxis(matTM,30,0.5f);

	//	RRenderHelper::EndRenderState();
	//}

	if(m_pSelectedActorNode)
	{
		if(m_pSelectedActorNode->m_pMeshNode)
		{
			RRenderHelper::SetRenderState();

			//m_pDevice->SetDepthEnable(false,false);
			RBoundingBox box = m_pSelectedActorNode->m_pMeshNode->m_boundingBox;
			if(!m_pSelectedActorNode->m_pMeshNode->isRenderAble())
				box = RBoundingBox(RVector(-5,-5,-5),RVector(5,5,5));

			RMatrix matTM = RMatrix::IDENTITY;
			if(m_pSelectedActorNode->GetWorldTransform())
				matTM = m_pSelectedActorNode->GetWorldTransform();

			RRenderHelper::RenderBox(matTM,box,0xffffffff);
			RRenderHelper::RenderAxis(matTM,30,0.5f);

			RenderAxisText(matTM, 30);

			RRenderHelper::EndRenderState();
		}
	}

	if (!m_TabModelInfo.m_NodeList.empty())
	{
		for (int i =0; i < (int)m_TabModelInfo.m_NodeList.size(); i++)
		{
			RActorNode* pActorNode = m_TabModelInfo.m_NodeList[i];
			if (pActorNode == NULL) continue;

			if (pActorNode->m_pMeshNode)
			{
				RRenderHelper::SetRenderState();

				//m_pDevice->SetDepthEnable(false,false);
				RBoundingBox box = pActorNode->m_pMeshNode->m_boundingBox;
				if(!pActorNode->m_pMeshNode->isRenderAble())
					box = RBoundingBox(RVector(-5,-5,-5),RVector(5,5,5));
				RMatrix matTM = pActorNode->GetWorldTransform();
				RRenderHelper::RenderBox(matTM,box,0xffff00ff);
				RRenderHelper::RenderAxis(matTM,30,0.5f);

				RenderAxisText(matTM, 30);
			
				RRenderHelper::EndRenderState();
			}
		}
	}
}

//그리드랑 등등
void CMainApplication::DrawPlatform()
{
	RRenderHelper::SetRenderState();

	RMatrix matCenter;

	matCenter.MakeIdentity();
	matCenter.SetTranslation(RVector( -500.f, -500.f, 0.f ));
	RRenderHelper::RenderGrid(matCenter, 10.f, 100, 100, 0xff898989);

	matCenter.MakeIdentity();
	matCenter.SetTranslation(RVector( -2000.f, -2000.f, 0.f ));
	RRenderHelper::RenderGrid(matCenter, 100.f, 40, 40, 0xff8f8f8f);

	RRenderHelper::RenderCenter(RMatrix::IDENTITY, 4000.f, 4000.f, 0xff88ff88);

	RRenderHelper::EndRenderState();
}

void CMainApplication::DeviceSetting()
{
	m_TMenu.SetDeviceFillMode();
}

void CMainApplication::PreOrderRender()
{
	if(m_bShowGrid)
		DrawPlatform();

	m_TMenu.DrawBone();

	m_TMenu.DrawWire();
}


void CMainApplication::PostOrderRender()
{
	if (m_ReferenceMesh.m_bUseRefMesh && m_ReferenceMesh.m_pReferenceMesh)
	{
		m_ReferenceMesh.m_pReferenceMesh->SetVisible(true);

		RVector vDir(0,1,0);
		RVector vPos(0,0,0);
		static RVector oldPos(0,0,0);
		if(m_pActor)
		{
			if (m_pTalentInfo)
			{
				if ( m_pTalentInfo->m_HitInfo.m_vSegments.empty() )
				{
					vPos = oldPos;
				}
				else
				{
					vDir = - m_pActor->GetDirection();

					const MCapsule& aCap = m_pTalentInfo->m_HitInfo.m_vSegments[0].m_vCapsules[0];
					vPos = aCap.bottom;
					vPos += ( vDir * aCap.radius );
					vPos.z = 0;

					oldPos = vPos;
				}
			}
			else
			{
				// 적절히 두배
				vPos.y += ( m_pActor->GetAABB().vmin.y * 4);
				oldPos = vPos;
			}
		}
		m_ReferenceMesh.m_pReferenceMesh->SetTransform(vPos, vDir);
		m_ReferenceMesh.m_pReferenceMesh->UpdateTransform();
		
		//m_pSceneManager->UpdateAndRender(m_ReferenceMesh.m_pReferenceMesh, m_pCamera);
	}
	else if( NULL != m_ReferenceMesh.m_pReferenceMesh )
	{
		m_ReferenceMesh.m_pReferenceMesh->SetVisible(false);
	}

	m_pDevice->SetFillMode(RFILL_SOLID);

	//m_SwordTrailSampling.RenderSwordTrailSampling();
	// [4/6/2007 madduck] - 이게 없으니 collision이랑 aabb랑 본등이 이상하게 나오넹.
	// TODO: 희한한네.
	m_pCamera->UpdateData();
	m_pLight->UpdateData();

	RenderMeshCollisionIndex(); //분홍색

	m_TMenu.DrawNormal();

	DrawBoxSelectedNode();

	m_TMenu.Render();

	m_TMenu.DrawActorNodeInfo(m_pSelectedActorNode);

	// pane manager render
	//PANE_MANEUVER::g_billboardManeuver.Maneuver();

	DrawHitEffectPosDir();
	
	DrawHitArea();

	DrawPartsHitCapsule();

	m_MagicArea.OnRender();

	if(m_bTalentAnimation)
		m_TalentAnimation.Render();

	GlobalObjects::g_pMainForm->CameratoolStripStatusLabel->Text = 
		String::Format("Camera pos : {0}, {1}, {2} ; At : {3}, {4}, {5}",
		m_pCamera->GetPosition().x, m_pCamera->GetPosition().y, m_pCamera->GetPosition().z,
		m_pCamera->m_vTargetPos.x, m_pCamera->m_vTargetPos.y, m_pCamera->m_vTargetPos.z);
}


bool CMainApplication::OnDraw()
{
	if( m_pDevice == NULL) return false;

 	m_pDevice->Clear(true, true, false, m_pCamera->GetClearColor());
 	m_pDevice->BeginScene();
 
// 	m_pCamera->BeginRender();
// 	m_pCamera->OnClearBuffer();
// 
// 	// ... 
// 	// PostOrderRender
// 
// 	m_pSceneManager->Cull(m_pCamera);
// 	m_pCamera->Render();
// 
// 	// ... 
// 	// PostOrderRender
// 
// 	m_pCamera->EndRender();
// 	m_pCamera->OnEndRender();
// 

	m_pSceneManager->RenderPrimaryCamera( this);

	m_pDevice->EndScene();
	m_pDevice->Flip();

	DrawExistDummyLoc();

	Sleep(10);

	return true;
}

void CMainApplication::OnUpdate()
{
	m_Timer.Update();

	float fsp = m_Timer.GetElapsedTime() * 0.001f;

	if( GlobalObjects::g_pMainForm != nullptr)
	{
		if(GlobalObjects::g_pMainForm->ActiveForm)
			UpdateLightToCamera(fsp);
	}

	Update(fsp);

	if(m_pMotion)
		m_pMotion->OnUpdate(0.0f);

	if(m_bTalentAnimation)
		m_TalentAnimation.Update(fsp);
	else
		m_NormalAnimation.Update(fsp);

	//////////////////////////////////////////////////////////////////////////
	// 업데이트 순서 바꾸지 마세요(번호로 된것...)
	// 1. 애니메이션 업데이트
	m_pActorMgr->Update(fsp);

	if(m_pActor)
	{
		int nCurFrame = m_pActor->GetCurFrame();

		if(nCurFrame < m_nPreFrame && (m_pActor->IsAnimationLoop() || m_bLoopAnimation))
			m_pActor->StopFaceAnimation();

		m_nPreFrame = nCurFrame;

		if(m_pActor->IsPlayDone())
		{
			if( m_bLoopAnimation )	OnPlayAnimation();
		}
	}

	GetAnimationPos(m_bMoveDummy);

	PartsColorPreview();
	AmbientColorPreview();

	//RotateUpdate(fsp);
	vec3 _pos = m_pCamera->GetPosition();
	vec3 _vel = vec3::ZERO;
	vec3 _dir = m_pCamera->GetDirection();
	vec3 _up = vec3::AXISZ;
	m_pSoundMgr.SetListener( _pos, _vel, _dir, _up);
	m_pSoundMgr.Update();

	// 2. 이펙트 업데이트
	m_pEffectManager->OnUpdate(fsp);
}

void CMainApplication::RotateUpdate(float fDelta)
{
	if( GetActor())
	{
		vec3 vDir = vec3::ZERO;
		vec3 vStartDir = GetActor()->GetDirection();

		float fRotateDegree = -1.f * fDelta/1000.f;
		vDir.x = vStartDir.x * cos(fRotateDegree) - vStartDir.y * sin(fRotateDegree);
		vDir.y = vStartDir.y * cos(fRotateDegree) + vStartDir.x * sin(fRotateDegree);

		GetActor()->SetDirection(vDir);
	}
}

//RTimer		 tmp_timer; 
static float fSecPerFrame = 1.0f / 60.0f; 
static float fTimer = 0;
void CMainApplication::Update(float fElapsed)
{
	m_pSceneManager->UpdateTimer();
	m_pSceneManager->Update();

	// 라이팅 적용을 Actor가 가지고 있는 SceneNodeController에서 하기 때문에 강제 업데이트 해줍니다.
	TCharacter* pActor = GetActor();
	if( pActor )
	{
		RSceneNodeUpdateVisitor visitor;
		if (visitor.OnPreOrderVisit(pActor))
			visitor.OnPostOrderVisit(pActor);
		//pActor->Update();
	}

	//////////////////////////////////////////////////////////////////////////
	// 프레임 제한 60 FPS
	//tmp_timer.Update();
	//float fElapsed = tmp_timer.GetElapsedTime() * 0.001f;

	static float fTotElapsed = 0;
	fTotElapsed += fElapsed;

	fTimer += fElapsed;
	// 주석 풀면 프레임 제한
	/*if( fTimer < 0 )	
	continue;
	*/
	fTimer -= fSecPerFrame; 

	//if (m_pPhx)
	//{
	//	m_pPhx->Update(fTotElapsed);
	//}
	fTotElapsed = 0;

	m_SwordTrailSampling.UpdateSwordTrailSampling(fElapsed);
	//REffectMgr::GetInstance()->OnUpdate();

	UpdateProjectile(fElapsed);
	UpdateMagicArea(fElapsed);
}

void CMainApplication::UpdateLightToCamera(float time)
{
	if(m_bLightToCamerapos)
	{
		RDirectionalLightSceneNode* pDirLight = m_pSceneManager->GetDefaultLight();
		if( pDirLight )
		{
			TCamera* pCamera = m_pCamera;
			if(IsKeyDown(VK_CONTROL))
			{
				pCamera = m_pLight;
			}
			pDirLight->SetLightPosition(pCamera->GetPosition());
			pDirLight->SetLightDirection((pCamera->m_vTargetPos - pCamera->GetPosition()));
		}
	}
}

void CMainApplication::GetAnimationPos(bool bMoveDummy /* = false */)
{
	if( !m_pActor) return;

	if(bMoveDummy)
	{
		if(m_bTalentAnimation)
		{
			if(m_TalentAnimation.CalAnimaitonPos())
				return;
		}
		else
		{
			if(m_NormalAnimation.CalAnimaitonPos())
				return;
		}
	}

	m_pActor->SetPosition(vec3::ZERO);
	return;
}

void CMainApplication::MovePartSegmentBottom(int dx, int dy, int dz)
{
	MCapsule* pCapsule = m_PartsHitInfoViewControl.GetCurrentSelectCapsule();
	if (pCapsule == NULL)
	{
		return;
	}

	RVector v = RVector(0.f, 1.f, 0.f);// -m_pCamera->GetDirection();//m_pActor->GetDirection();

	v.z = 0;
	v.Normalize();

	RVector side;

	side = v.CrossProduct(RVector::AXISZ);

	RVector mov;

	mov = dy * 3.0f * v;
	mov += dx * 3.0f * -side;
	mov += dz * 3.0f * RVector::AXISZ;

	pCapsule->bottom += mov;
	pCapsule->CalcHeight();

	GlobalObjects::g_SaveNotifier.SetSaveMeshInfo(true);
}

void CMainApplication::MovePartSegmentTop(int dx, int dy, int dz)
{
	MCapsule* pCapsule = m_PartsHitInfoViewControl.GetCurrentSelectCapsule();
	if (pCapsule == NULL)
	{
		return;
	}

	RVector v = RVector(0.f, 1.f, 0.f);
	v.z = 0;
	v.Normalize();

	RVector side;
	side = v.CrossProduct(RVector::AXISZ);

	RVector mov;
	mov = dy * 3.0f * v;
	mov += dx * 3.0f * -side;
	mov += dz * 3.0f * RVector::AXISZ;

	pCapsule->top += mov;
	pCapsule->CalcHeight();

	GlobalObjects::g_SaveNotifier.SetSaveMeshInfo(true);
}

void CMainApplication::OnMouseMoveColEdit(int stat, RVector2 &pos, int delta)
{
	if(IsKeyDown(VK_CONTROL))
	{
		// 캡슐 편집
		if(GlobalObjects::g_SelInfo.m_nMeshColID < COLLISION_BOX_START_INDEX)
		{
			if(GlobalObjects::g_pMainForm->Top_toolStripButton->Checked)
			{
				if( stat == 1)
				{
					if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
						MoveColSegmentBottom(m_nMousePrevX - (int)pos.x, 0, 0);
					if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
						MoveColSegmentBottom(0, m_nMousePrevY - (int)pos.y, 0);
					if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
						MoveColSegmentBottom(0, 0, m_nMousePrevY - (int)pos.y);
					if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
						MoveColSegmentBottom(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
				}
			}
			else if(GlobalObjects::g_pMainForm->Bottom_toolStripButton->Checked)
			{
				if (stat == 1)
				{
					if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
						MoveColSegmentTop(m_nMousePrevX - (int)pos.x, 0, 0);
					if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
						MoveColSegmentTop(0, m_nMousePrevY - (int)pos.y, 0);
					if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
						MoveColSegmentTop(0, 0, m_nMousePrevY - (int)pos.y);
					if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
						MoveColSegmentTop(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
				}
			}
			else if(GlobalObjects::g_pMainForm->TopBottom_toolStripButton->Checked)
			{
				if (stat == 1)
				{
					if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
					{
						MoveColSegmentBottom(m_nMousePrevX - (int)pos.x, 0, 0);
						MoveColSegmentTop(m_nMousePrevX - (int)pos.x, 0, 0);
					}
					if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
					{
						MoveColSegmentBottom(0, m_nMousePrevY - (int)pos.y, 0);
						MoveColSegmentTop(0, m_nMousePrevY - (int)pos.y, 0);
					}
					if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
					{
						MoveColSegmentBottom(0, 0, m_nMousePrevY - (int)pos.y);
						MoveColSegmentTop(0, 0, m_nMousePrevY - (int)pos.y);
					}
					if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
					{
						MoveColSegmentBottom(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
						MoveColSegmentTop(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
					}
				}
			}
		}
		else
		{
			if(m_MeshColInfoViewControl.CheckMousePick(stat, pos, vec2(m_nMousePrevX, m_nMousePrevY)) == false)
				OnCameraMouseMove(stat, pos, delta);
		}
	}
	else
	{
		if(m_MeshColInfoViewControl.CheckMousePick(stat, pos, vec2(m_nMousePrevX, m_nMousePrevY)) == false)
			OnCameraMouseMove(stat, pos, delta);
	}
}

void CMainApplication::OnMouseMovePartEdit(int stat, RVector2 &pos, int delta)
{
	if(IsKeyDown(VK_CONTROL))
	{
		if(GlobalObjects::g_pMainForm->Top_toolStripButton->Checked)
		{
			if( stat == 1)
			{
				if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
					MovePartSegmentBottom(m_nMousePrevX - (int)pos.x, 0, 0);
				if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
					MovePartSegmentBottom(0, m_nMousePrevY - (int)pos.y, 0);
				if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
					MovePartSegmentBottom(0, 0, m_nMousePrevY - (int)pos.y);
				if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
					MovePartSegmentBottom(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
			}
		}
		else if(GlobalObjects::g_pMainForm->Bottom_toolStripButton->Checked)
		{
			if (stat == 1)
			{
				if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
					MovePartSegmentTop(m_nMousePrevX - (int)pos.x, 0, 0);
				if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
					MovePartSegmentTop(0, m_nMousePrevY - (int)pos.y, 0);
				if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
					MovePartSegmentTop(0, 0, m_nMousePrevY - (int)pos.y);
				if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
					MovePartSegmentTop(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
			}
		}
		else if(GlobalObjects::g_pMainForm->TopBottom_toolStripButton->Checked)
		{
			if (stat == 1)
			{
				if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
				{
					MovePartSegmentBottom(m_nMousePrevX - (int)pos.x, 0, 0);
					MovePartSegmentTop(m_nMousePrevX - (int)pos.x, 0, 0);
				}
				if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
				{
					MovePartSegmentBottom(0, m_nMousePrevY - (int)pos.y, 0);
					MovePartSegmentTop(0, m_nMousePrevY - (int)pos.y, 0);
				}
				if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
				{
					MovePartSegmentBottom(0, 0, m_nMousePrevY - (int)pos.y);
					MovePartSegmentTop(0, 0, m_nMousePrevY - (int)pos.y);
				}
				if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
				{
					MovePartSegmentBottom(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
					MovePartSegmentTop(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
				}
			}
		}
		else
			OnCameraMouseMove(stat, pos, delta);
	}
	else
		OnCameraMouseMove(stat, pos, delta);
}

void CMainApplication::OnMouseMoveTalentEdit( int stat, RVector2 &pos, int delta )
{
	if(IsKeyDown(VK_CONTROL))
	{
		TTalentHitTabController* pHitTab = GlobalObjects::g_pMainForm->m_TalentTabPage->m_TalentHitTabController;

		if(GlobalObjects::g_pMainForm->Bottom_toolStripButton->Checked)
		{
			if( stat == 1)
			{
				if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
					pHitTab->MoveSegmentBottom(m_nMousePrevX - (int)pos.x, 0, 0);
				if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
					pHitTab->MoveSegmentBottom(0, m_nMousePrevY - (int)pos.y, 0);
				if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
					pHitTab->MoveSegmentBottom(0, 0, m_nMousePrevY - (int)pos.y);
				if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
					pHitTab->MoveSegmentBottom(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
			}
		}
		else if(GlobalObjects::g_pMainForm->Top_toolStripButton->Checked)
		{
			if (stat == 1)
			{
				if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
					pHitTab->MoveSegmentTop(m_nMousePrevX - (int)pos.x, 0, 0);
				if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
					pHitTab->MoveSegmentTop(0, m_nMousePrevY - (int)pos.y, 0);
				if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
					pHitTab->MoveSegmentTop(0, 0, m_nMousePrevY - (int)pos.y);
				if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
					pHitTab->MoveSegmentTop(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
			}
		}
		else if(GlobalObjects::g_pMainForm->TopBottom_toolStripButton->Checked)
		{
			if (stat == 1)
			{
				if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
				{
					pHitTab->MoveSegmentBottom(m_nMousePrevX - (int)pos.x, 0, 0);
					pHitTab->MoveSegmentTop(m_nMousePrevX - (int)pos.x, 0, 0);
				}
				if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
				{
					pHitTab->MoveSegmentBottom(0, m_nMousePrevY - (int)pos.y, 0);
					pHitTab->MoveSegmentTop(0, m_nMousePrevY - (int)pos.y, 0);
				}
				if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
				{
					pHitTab->MoveSegmentBottom(0, 0, m_nMousePrevY - (int)pos.y);
					pHitTab->MoveSegmentTop(0, 0, m_nMousePrevY - (int)pos.y);
				}
				if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
				{
					pHitTab->MoveSegmentBottom(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
					pHitTab->MoveSegmentTop(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
				}
			}
		}
		else
			OnCameraMouseMove(stat, pos, delta);
	}
	else
		OnCameraMouseMove(stat, pos, delta);
}

void CMainApplication::OnMouseMoveBoneNode( int stat, RVector2& pos, int delta )
{
	if(stat != 0)
		return;

	m_TabModelInfo.CheckMousePick(pos);
}

void CMainApplication::OnMouseMove(int stat, RVector2 pos, int delta)
{
	if(m_TMenu.CheckDrawBone(SBS_SIMPLE))
		OnMouseMoveBoneNode(stat, pos, delta);

	// 판정 편집
	//if (GlobalObjects::g_pMainForm->PartsJudgment->Visible)
	if(GlobalObjects::g_pMainForm->tabControl2->SelectedTab == GlobalObjects::g_pMainForm->PartsJudgment &&
		GlobalObjects::g_pMainForm->PartsHitCapsuleAtt->Visible)
	{
		OnMouseMovePartEdit(stat, pos, delta);
	}
	// 충돌 편집
	else if(GlobalObjects::g_pMainForm->tabControl2->SelectedTab == GlobalObjects::g_pMainForm->PartsJudgment &&
		GlobalObjects::g_pMainForm->CollisionCapsuleAtt->Visible)
	{
		OnMouseMoveColEdit(stat, pos, delta);
	}
	//탈렌트편집
	else if (GlobalObjects::g_pMainForm->tabControl2->SelectedTab == GlobalObjects::g_pMainForm->Talenttab)
		//else if (GlobalObjects::g_pMainForm->Talenttab->Visible)
	{
		OnMouseMoveTalentEdit(stat, pos, delta);
	}
	// 판정 이펙트 편집
	else if(GlobalObjects::g_pMainForm->tabControl2->SelectedTab == GlobalObjects::g_pMainForm->PartsJudgment &&
		GlobalObjects::g_pMainForm->HitEffect_propertyGrid->Visible)
	{
		OnMouseMoveHitEffectEdit(stat, pos, delta);
	}
	else
	{
		OnCameraMouseMove(stat, pos, delta);
	}

	m_nMousePrevX = (int)pos.x;
	m_nMousePrevY = (int)pos.y;
}

void CMainApplication::OnCameraMouseMove(int stat, RVector2 pos, int delta)
{
	if(IsKeyDown(VK_CONTROL) == false)
	{
		m_bCameraMove = false;
		if(m_pCamera->OnMouseMove(stat, pos, delta))
		{
			if(stat != 0)
				m_bCameraMove = true;
		}
	}

	m_pLight->OnMouseMove(stat, pos, delta);
}

void CMainApplication::AniTreeView_AnimationPlay(System::String^ Aniname)
{
	if(m_pActor == NULL) return;

	const char* name= MStringToCharPointer(Aniname);
	AniTreeView_AnimationPlay(name);
	MFreeCharPointer(name);
}

void CMainApplication::AniTreeView_AnimationPlay(const char* name)
{
	m_bTalentAnimation = false;
	m_TalentAnimation.InitTalentAnimation();
	m_NormalAnimation.InitNormalAnimation();

	m_pActor->SetTransform(MMatrix::IDENTITY);

	// TODO : npc 이면 아님 아래 함수 안에서 검사 ?
	UpdateModelScale();

	// 탤런트 이펙트 삭제
	m_TalentAnimation.TalentAnimationEnd();

	m_NormalAnimation.SetAnimation(name);

	// 이전과 같은 애니?
	bool bSameName = false;
	RAnimation* pAnimation = m_pActor->GetCurAnimation();
	if(pAnimation)
	{
		if(pAnimation->GetAliasName() == name)
			bSameName = true;
	}

	if(SetAnimation((char*)name))
	{
		m_TabAnimation.InitEventList();
		m_TabAnimation.InitAnimationInfo();
		m_TabAnimation.InitHitSoundInfo();

		m_eventListener.StopAnimationEffect();

		GlobalObjects::g_pMainForm->m_FrameSliderController->pressPlayButton();
		GlobalObjects::g_pMainForm->m_AnimationTabPage->SetPauseButtonDown_AnimationLinkTest();

		if(bSameName == false)
		{
			// 애니메이션 갱신하면서... 
			m_TabModelInfo.SetBipTreeView();
		}
	}
}

CSItemData* CMainApplication::GetItemData(int index)
{
	return m_TabItem.GetItemData(index);
}

void CMainApplication::SetClearClolor(DWORD col)
{
	m_pCamera->SetClearColor(col);
}

bool CMainApplication::SaveAnimationXml(System::String^ FileName)
{
	const char* cstr1 = MStringToCharPointer(FileName);
	bool bSave = SaveAnimationXml(cstr1);
	if( !bSave)
	{
		mlog("에러 : 저장이 되지 않았습니다. - %s \r", cstr1);
	}
	else
	{
		mlog("정보 : %s 저장 완료 \r", cstr1);
	}
	MFreeCharPointer(cstr1);

	return true;
}

// TODO: 
bool CMainApplication::SaveAnimationXml(const char* szFileName)
{
	RMeshAnimationMgr* pAMgr = &GetActor()->m_pMesh->m_AniMgr;

	//string s = GetActor()->m_pMesh->GetFileName();
	string ss = GetActor()->m_pMesh->GetName();

	// TODO: 저장전에 소트를 하게 되면 소트된것이 저장되고
	// 그것을 그대로 읽으니 소트된듯 보인다. - 계획은...
	//std::sort(pAMgr->m_NodeHashList.begin(), pAMgr->m_NodeHashList.end(), _eval_);

	return pAMgr->SaveToXml(szFileName);
}

void CMainApplication::SetActorSpineTarget(float fAngle1,float fAngle2)
{
	m_pActor->SetActorSpineTarget(fAngle1,fAngle2);
}

bool CMainApplication::SaveAs()
{
	SaveFileDialog^ saveFileDialog1 = gcnew SaveFileDialog;

	string strEluFileName = string(GetActor()->m_pMesh->GetName()) + ".animation.xml";
	ReplaceSlashToBackSlash(strEluFileName);

	String^ strFileName = gcnew String(strEluFileName.c_str());

	//saveFileDialog1->InitialDirectory = strPathName;
	saveFileDialog1->FileName = strFileName;
	saveFileDialog1->Filter = "xml files (*.xml)|*.xml|All files (*.*)|*.*";
	saveFileDialog1->FilterIndex = 1;
	saveFileDialog1->RestoreDirectory = true;

	if(saveFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
	{
		SaveAnimationXml(saveFileDialog1->FileName);
		return true;
	}

	return false;
}


void CMainApplication::SetActorCollision()
{
	m_bRenderActorCollision = GlobalObjects::g_pMainForm->collisionObjectToolStripMenuItem->Checked;
}

void CMainApplication::SetCurrentTalentInfo(CSTalentInfo* pTalentInfo)
{
	if (pTalentInfo == NULL) return;

	m_pTalentInfo = pTalentInfo;

	GlobalObjects::g_pMainForm->m_TalentTabPage->TalentIDtextbox->Text = String::Format("{0}", pTalentInfo->m_nID);
	GlobalObjects::g_pMainForm->m_TalentTabPage->m_TalentHitTabController->Serialize();
	GlobalObjects::g_pMainForm->m_TalentEffectTabController->Serialize();

	//판정탭
	GlobalObjects::g_pMainForm->m_TalentTabPage->groupBox7->Visible = true;
	GlobalObjects::g_pMainForm->m_TalentTabPage->groupBox8->Visible = true;
	GlobalObjects::g_pMainForm->m_TalentTabPage->Capsule_propertyGrid->Visible = true;
}

bool CMainApplication::OnPlayMotion(System::String^ str1 /* = nullptr */)
{
	const char* cstr = MStringToCharPointer(str1);
	bool bplay = m_pMotion->ChangeMotion(cstr);
	MFreeCharPointer(cstr);

	return bplay;
}

void CMainApplication::SetTalent(CSTalentInfo* pTalentInfo)
{
	if (pTalentInfo == NULL)
	{
		// 초기화

		// 이벤트
		GlobalObjects::g_pMainForm->m_EventTabPage->m_TalentEventController->Clear();

		return;
	}

	SetTalentEffect(pTalentInfo);
	GlobalObjects::g_pMainForm->m_EventTabPage->m_TalentEventController->SetTalentEvent(pTalentInfo);
}

void CMainApplication::SetTalentEffect(CSTalentInfo* pTalentInfo)
{
	string strTalentID = TStrings::TalentIDString_WithMode(pTalentInfo->m_nID, pTalentInfo->m_nMode);

}



/************************************************************************/
/* mlog에 콜백함수로 연결되어 있음.										*/
/* mlog로 사용할것.                                                     */
/************************************************************************/
void CMainApplication::McvLog(const char* szText, MLogLevel nLevel)
{
	if (nLevel == MLOG_LEVEL_INFORMATION)
	{
	}
	else if (nLevel == MLOG_LEVEL_ERROR)
	{
		GlobalObjects::g_pMainForm->LogtextBox->ForeColor = System::Drawing::Color::Red;
	}

	GlobalObjects::g_pMainForm->LogtextBox->Text += gcnew String(szText);
	GlobalObjects::g_pMainForm->LogtextBox->Text += System::Environment::NewLine;

	//스크롤 내리기.
	GlobalObjects::g_pMainForm->LogtextBox->SelectionStart = GlobalObjects::g_pMainForm->LogtextBox->TextLength;
	GlobalObjects::g_pMainForm->LogtextBox->ScrollToCaret();
}

void CMainApplication::HideAllTabs()
{
	//Enable가 따로 없어서 제거했다가 Add한다.
	GlobalObjects::g_pMainForm->tabControl2->Controls->Remove(GlobalObjects::g_pMainForm->ModelInfotab);
	GlobalObjects::g_pMainForm->tabControl2->Controls->Remove(GlobalObjects::g_pMainForm->Itemtab);
	GlobalObjects::g_pMainForm->tabControl2->Controls->Remove(GlobalObjects::g_pMainForm->Animationtab);
	GlobalObjects::g_pMainForm->tabControl2->Controls->Remove(GlobalObjects::g_pMainForm->Talenttab);
	GlobalObjects::g_pMainForm->tabControl2->Controls->Remove(GlobalObjects::g_pMainForm->Effecttab);
	GlobalObjects::g_pMainForm->tabControl2->Controls->Remove(GlobalObjects::g_pMainForm->TalentEvent);
	GlobalObjects::g_pMainForm->tabControl2->Controls->Remove(GlobalObjects::g_pMainForm->ItemPreviewtabPage);
	GlobalObjects::g_pMainForm->tabControl2->Controls->Remove(GlobalObjects::g_pMainForm->PartsJudgment);
	GlobalObjects::g_pMainForm->tabControl2->Controls->Remove(GlobalObjects::g_pMainForm->SubMtrltab);
	GlobalObjects::g_pMainForm->tabControl2->Controls->Remove(GlobalObjects::g_pMainForm->MeshInfotab);
}

void CMainApplication::ShowTabs(int index)
{
	HideAllTabs();

	m_SelectedTab = (TAB_MTRLSTATE)index;

	switch(index)
	{
	case 0:
		{
			array<Control^>^controlArray = {
				GlobalObjects::g_pMainForm->Animationtab,
				GlobalObjects::g_pMainForm->ModelInfotab, GlobalObjects::g_pMainForm->Itemtab,
				GlobalObjects::g_pMainForm->ItemPreviewtabPage,
				GlobalObjects::g_pMainForm->PartsJudgment,
				GlobalObjects::g_pMainForm->SubMtrltab,
				GlobalObjects::g_pMainForm->MeshInfotab
			};
			GlobalObjects::g_pMainForm->tabControl2->Controls->AddRange(controlArray);
			GlobalObjects::g_pMainForm->groupBox4->Enabled = true;
			GlobalObjects::g_pMainForm->tabControl2->SelectedTab = GlobalObjects::g_pMainForm->Animationtab;
			GlobalObjects::g_pMainForm->m_ModelTabPage->ModeltreeView->Focus();
		}
		break;
	case 1:
		{
			array<Control^>^controlArray = {
				GlobalObjects::g_pMainForm->Itemtab, GlobalObjects::g_pMainForm->Animationtab,
				GlobalObjects::g_pMainForm->ModelInfotab,
				GlobalObjects::g_pMainForm->Talenttab, 
				GlobalObjects::g_pMainForm->Effecttab,
				GlobalObjects::g_pMainForm->TalentEvent,
				GlobalObjects::g_pMainForm->SubMtrltab
			};
			GlobalObjects::g_pMainForm->tabControl2->Controls->AddRange(controlArray);
			GlobalObjects::g_pMainForm->groupBox4->Enabled = false;
		}
		break;
	case 2:
		{
			array<Control^>^controlArray = {
				GlobalObjects::g_pMainForm->Talenttab,
				GlobalObjects::g_pMainForm->Effecttab,
				GlobalObjects::g_pMainForm->TalentEvent
			};
			GlobalObjects::g_pMainForm->tabControl2->Controls->AddRange(controlArray);
			GlobalObjects::g_pMainForm->groupBox4->Enabled = false;

			GlobalObjects::g_pMainForm->m_PCTalenttreeView->Init();
		}
		break;
	}
}

bool CMainApplication::IsDefaultPlayerModelLoad( )
{
	if(!m_NPC.IsValidNPCID(0) || m_pActor == NULL)
	{
		return true;
	}

	return false;
}

void CMainApplication::PlayerModelLoad( String^ strSex )
{
	if (String::IsNullOrEmpty(strSex) == true)
	{
		mlog("is null or empty");
		return;
	}

	TreeNode^ node = SearchNode(GlobalObjects::g_pMainForm->m_ModelTabPage->GetNodes(), strSex);
	System::Windows::Forms::TreeViewEventArgs^  e;
	e = gcnew System::Windows::Forms::TreeViewEventArgs(node, TreeViewAction::ByMouse);
	ModeltreeView_AfterSelect(e);

	// 모델리스트 선택
	GlobalObjects::g_pMainForm->m_ModelTabPage->ModeltreeView->SelectedNode = node;
}

bool CMainApplication::OnPlayAnimation()
{
	if( GetActor() == NULL) return false;

	m_pActor->StopFaceAnimation();

	bool bPause = false;
	bool bPlayDone = false;

	if( m_pActor->GetAnimationState() == RPS_PAUSE)
	{
		bPause = true;
	}
	else if(m_pActor->IsPlayDone() ||
		m_pActor->GetAnimationState() == RPS_STOP)
	{
		bPlayDone = true;
	}

	bool ret = false;
	if(m_bTalentAnimation)
	{
		// 탤런트가 있다면...
		ret = m_TalentAnimation.TalentAnimationPlay(bPause);
	}
	else
		ret = m_NormalAnimation.AnimationPlay(bPause, bPlayDone);

	if(ret)
	{
		m_bPlayPauseButtonStateByShourtCut = true;
	}
	// 에러...
	else
	{
		// 루프상태이면 푼다.
		if(m_bLoopAnimation)
			OnLoopAnimation();
	}

	return ret;
}

void CMainApplication::OnStopAnimation()
{
	if( !GetActor() ) return;

	m_pActor->StopFaceAnimation();
	GlobalObjects::g_pMainForm->AnitrackBar->Value = 0;

	if(m_bTalentAnimation)
	{
		m_TalentAnimation.TalentAnimationStop();
	}
	else
	{
		m_NormalAnimation.AnimationStop();
	}

	m_eventListener.StopAnimationEffect();
}

void CMainApplication::OnPauseAnimation()
{
	if(m_bTalentAnimation)
	{
		// 탤런트가 있다면...
		m_TalentAnimation.TalentAnimationPause();
	}
	else if(m_pActor->GetAnimationState() == RPS_PLAY &&
		!m_pActor->IsPlayDone())
	{
		m_NormalAnimation.AnimationPause();
	}
}

void CMainApplication::OnLoopAnimation()
{
	m_bLoopAnimation = !m_bLoopAnimation;

	if( m_bLoopAnimation)
	{
		GlobalObjects::g_pMainForm->LoopPlaycheckBox->Checked = true;
		GlobalObjects::g_pMainForm->animationLoopPlayToolStripMenuItem->Checked = true;
	}
	else
	{
		GlobalObjects::g_pMainForm->LoopPlaycheckBox->Checked = false;
		GlobalObjects::g_pMainForm->animationLoopPlayToolStripMenuItem->Checked = false;
	}
}

void CMainApplication::OnTransitionBlending()
{
	m_bTransitionBlending = !m_bTransitionBlending;
	if( m_bTransitionBlending)
		GlobalObjects::g_pMainForm->transitionBlendingToolStripMenuItem->Checked = true;
	else
		GlobalObjects::g_pMainForm->transitionBlendingToolStripMenuItem->Checked = false;
}

void CMainApplication::OnFullScreen()
{
	if( GlobalObjects::g_pMainForm->MainView->Parent == GlobalObjects::g_pMainForm->splitContainer2->Panel1)
	{
		GlobalObjects::g_pMainForm->FormBorderStyle = FormBorderStyle::None;
		GlobalObjects::g_pMainForm->WindowState = FormWindowState::Maximized;
		GlobalObjects::g_pMainForm->MainView->Parent = GlobalObjects::g_pMainForm;
		GlobalObjects::g_pMainForm->splitContainer1->Visible = false;
		GlobalObjects::g_pMainForm->groupBox3->Visible = false;
		GlobalObjects::g_pMainForm->groupBox4->Visible = false;
		GlobalObjects::g_pMainForm->LogtextBox->Visible = false;
		GlobalObjects::g_pMainForm->toolStrip1->Visible = false;
		GlobalObjects::g_pMainForm->MainmenuStrip->Visible = false;
		GlobalObjects::g_pMainForm->statusStrip->Visible = false;
	}
	else
	{
		GlobalObjects::g_pMainForm->MainView->Parent = GlobalObjects::g_pMainForm->splitContainer2->Panel1;
		GlobalObjects::g_pMainForm->splitContainer1->Visible = true;
		GlobalObjects::g_pMainForm->groupBox3->Visible = true;
		GlobalObjects::g_pMainForm->groupBox4->Visible = true;
		GlobalObjects::g_pMainForm->LogtextBox->Visible = true;
		GlobalObjects::g_pMainForm->toolStrip1->Visible = true;
		GlobalObjects::g_pMainForm->MainmenuStrip->Visible = true;
		GlobalObjects::g_pMainForm->statusStrip->Visible = true;
		GlobalObjects::g_pMainForm->WindowState = FormWindowState::Normal;
		GlobalObjects::g_pMainForm->FormBorderStyle = FormBorderStyle::Sizable;
	}
}

void CMainApplication::SetReviewItems()
{
	GlobalObjects::g_pMainForm->ItemPreviewtreeView->Nodes->Clear();

	for( int s = 0; s < (int)m_vItemModelNames.size(); s++)
	{
		string filename = m_vItemModelNames[s]; 
		string tagname	= GetPureFileName(m_vItemModelNames[s]);

		TreeNode^ rootnode = GlobalObjects::g_pMainForm->ItemPreviewtreeView->Nodes->Add(
			gcnew String(tagname.c_str()));

		RActor* pActor = eluFileOpen(filename.c_str());
		if( pActor == NULL) continue;

		for(int i = 0; i<pActor->GetActorNodeCount();i++)
		{
			RActorNode *pActorNode = pActor->GetActorNodes()[i];
			String^ nodename = gcnew String(pActorNode->m_pMeshNode->m_Name.c_str());

			if( nodename->StartsWith("Bip") || nodename->StartsWith("dummy") )
				continue;

			if( pActorNode->GetParentActorNode() == NULL)
				rootnode->Nodes->Add(nodename);
			else if( pActorNode->GetParentActorNode() != NULL)
			{
				RActorNode *parentActorNode = pActorNode->GetParentActorNode();
				String^ parentnodename = gcnew String(parentActorNode->m_pMeshNode->m_Name.c_str());

				TreeNode^ tn = SearchNode(GlobalObjects::g_pMainForm->ItemPreviewtreeView->Nodes, parentnodename);
				if( tn != nullptr)
					tn->Nodes->Add(nodename);
			}
		}

		SAFE_DELETE(pActor);

		int value = ((s+1) / m_vItemModelNames.size()) * 100;
		SetProgressBarValue(value);
	}
}

void CMainApplication::RefreshModels()
{
	SetProgressBarValue();

	vector<tstring>	PathList;
	PathList.push_back(GetPathName(m_strModelFullPathName));
	//PathList.push_back(PATH_WEAPON_TOOL);

	m_vItemModelNames.clear();

	HANDLE hDir;
	WIN32_FIND_DATA DirData = {0};
	BOOL bRet = TRUE;
	tstring subDir;
	for( int i = 0; i < (int)PathList.size(); ++i)
	{
		bRet = TRUE;
		subDir = PathList[i]+"*.elu";

		hDir = FindFirstFile(subDir.c_str(), &DirData);
		while( (hDir != INVALID_HANDLE_VALUE) && bRet )
		{
			if (DirData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) 
			{
				tstring str = DirData.cFileName;
				m_vItemModelNames.push_back(PathList[i] + str);
			}
			bRet = FindNextFile(hDir, &DirData);
		}
		FindClose(hDir);
	}

	PathList.clear();
}

SH_ITEM_SLOT CMainApplication::GetNodetoItemslot(string text)
{
	const char* ItemSlotStr[] =
	{
		"hat_item",		//ITEMSLOT_HEAD = 0,	//모자.
		"face",			//ITEMSLOT_FACE,	//선글라스등.
		"hands_item",	//ITEMSLOT_HANDS,
		"feet_item",	//ITEMSLOT_FEET,
		"chest_item",	//ITEMSLOT_BODY,
		"legs_item",	//ITEMSLOT_LEG,
		"",				//ITEMSLOT_LFINGER,
		"",				//ITEMSLOT_RFINGER,
		"",				//ITEMSLOT_NECK,
		"hair",				//ITEMSLOT_CHARM,
		"lweapon",		//ITEMSLOT_LWEAPON,
		"rweapon",		//ITEMSLOT_RWEAPON,
		"lweapon2",		//ITEMSLOT_LWEAPON2,
		"rweapon2",		//ITEMSLOT_RWEAPON2,
		"",			//ITEMSLOT_LOOK_HEAD,	
		"",			//ITEMSLOT_LOOK_HANDS,
		"",				//ITEMSLOT_LOOK_FEET,
		"",				//ITEMSLOT_LOOK_BODY,
		"",				//ITEMSLOT_LOOK_LEG,
		"",
		"",				//ITEMSLOT_MAX 장착불가능한 아이템
		""				//21
	};

	SH_ITEM_SLOT index = ITEMSLOT_MAX;
	for( int i = 0;i < 22; ++i)
	{
		if(strcmp(ItemSlotStr[i], text.c_str()) == 0)
		{
			index = (SH_ITEM_SLOT)i;
			break;
		}
	}

	return index;
}

bool CMainApplication::SaveTalentEffectInfo()
{
	bool bSave = false;
	bSave = GlobalObjects::g_SaveNotifier.SaveTalentEffectInfo();

	return bSave;
}

void CMainApplication::SetReturnAni()
{
	if( GetActor() == NULL || GetActor()->GetCurAnimation() == NULL) return;

	m_returnAnimationList.push_back(GetActor()->GetCurAnimation());
	String^ str = gcnew String( GetActor()->GetCurAnimation()->GetAliasName().c_str() );
	GlobalObjects::g_pMainForm->m_AnimationTabPage->contextMenuStrip1->Items->Add(str);
}

void CMainApplication::ClearReturnAni()
{
	m_returnAnimationList.clear();
	if(GlobalObjects::g_pMainForm->m_AnimationTabPage->contextMenuStrip1->Items->Count > 5)
	{
		int isize = GlobalObjects::g_pMainForm->m_AnimationTabPage->contextMenuStrip1->Items->Count-5;
		for( int i = 0; i < isize; i++)
			GlobalObjects::g_pMainForm->m_AnimationTabPage->contextMenuStrip1->Items->RemoveAt(5);
	}
}

void CMainApplication::SetModelFullPathName(System::String^ ModelName)
{
	const char* name = MStringToCharPointer(ModelName);
	m_strModelFullPathName = name;
	MFreeCharPointer(name);
}

void CMainApplication::SetModelFullPathName( string strModelName )
{
	m_strModelFullPathName = strModelName;
}

void CMainApplication::BasicMesh()
{
	if (GetActor() == NULL) return;

	m_pSelectedActorNode = NULL;
	m_TabModelInfo.m_NodeList.clear();
	m_TabModelInfo.m_PartsColorTestList.clear();

	m_EquipItems.AllUnEquipItem();
}

void CMainApplication::SetProgressBarValue(int value /* = 0 */)
{
	GlobalObjects::g_pMainForm->Status_toolStripProgressBar->Value = value;
	GlobalObjects::g_pMainForm->Status_toolStripStatusLabel->Text = String::Format("{0}%", value);

	if (value == 100)
	{
		GlobalObjects::g_pMainForm->Status_toolStripProgressBar->Visible = false;
		GlobalObjects::g_pMainForm->Status_toolStripStatusLabel->Visible = false;
	}
	else
	{
		GlobalObjects::g_pMainForm->Status_toolStripProgressBar->Visible = true;
		GlobalObjects::g_pMainForm->Status_toolStripStatusLabel->Visible = true;
	}
}

bool CMainApplication::SaveTalentEventInfo()
{
	InitTalentHitInfo();
	SetTalent(NULL);

	bool bSave = false;

	bSave = GlobalObjects::g_SaveNotifier.SaveTalentEventInfo();

	GlobalObjects::g_SaveNotifier.SetSaveTalentEvent(false);

	return bSave;
}

bool CMainApplication::SaveTalentHitInfo()
{
	bool bSave = GlobalObjects::g_SaveNotifier.SaveTalentHitInfo();
	return bSave;
}


void CMainApplication::SaveTalentExt(bool bSaveEvent /* = false */, bool bSaveEffect /* = false */, bool bSaveCol /* = false */)
{
	// 탤런트 이펙트 정보
	if(bSaveEvent)
	{
		if(!SaveTalentEventInfo())
		{
			mlog("에러 : 탈렌트이벤트정보가 저장이 되지않습니다. \r");
		}
	}

	// 탤런트 이벤트 정보
	if(bSaveEffect)
	{
		if(!SaveTalentEffectInfo())
		{
			mlog("에러 : 탈렌트이펙트정보가 저장이 되지않습니다. \r");
		}
	}

	// 탤런트 충돌 정보
	if(bSaveCol)
	{
		if (SaveTalentHitInfo())
		{
			GlobalObjects::g_pMainForm->m_TalentTabPage->m_TalentHitTabController->Serialize();
		}
		else
		{
			mlog("에러 : 히트정보가 저장이 되지않습니다. \r");
		}
	}
}

void CMainApplication::SetMeshHitInfo( System::String^ ModelName )
{
	GlobalObjects::g_pMainForm->m_TalentTabPage->m_TalentHitTabController->Clear();
	m_PartsHitInfoViewControl.ChangePartsHitInfoViewer(ModelName);
}

int CMainApplication::CreateMeshHitCapsuleInfo( int nGroupID )
{
	if(nGroupID == -1)
	{
		return -1;
	}

	int nCapsuleID = TMeshInfoMgr::GetInstance().GetMeshHitInfo()->CreateCapsule(m_strModelFullPathName, nGroupID);

	m_PartsHitInfoViewControl.CreatePartsHitInfoViewer(nGroupID, nCapsuleID);
	m_PartsHitInfoViewControl.NotifyChanging();
	return nCapsuleID;
}

int CMainApplication::GetNextCapsuleIndex( int nGroupID )
{
	return TMeshInfoMgr::GetInstance().GetMeshHitInfo()->GetNextCapsuleIndex(m_strModelFullPathName, nGroupID);
}

int CMainApplication::CreateMeshHitGroupInfo()
{
	int nGroupID = TMeshInfoMgr::GetInstance().GetMeshHitInfo()->CreateGroup(m_strModelFullPathName);

	m_PartsHitInfoViewControl.CreatePartsHitInfoViewer(nGroupID);

	m_PartsHitInfoViewControl.NotifyChanging();
	return nGroupID;
}

void CMainApplication::DelMeshHitGroupInfo( int nGroupID )
{
	TMeshInfoMgr::GetInstance().GetMeshHitInfo()->DelGroup(m_strModelFullPathName, nGroupID);
	m_PartsHitInfoViewControl.Reload();
	m_PartsHitInfoViewControl.NotifyChanging();
}

void CMainApplication::DelMeshHitCapsuleInfo( int nGroupID, int nCapsuleID )
{
	TMeshInfoMgr::GetInstance().GetMeshHitInfo()->DelCapsule(m_strModelFullPathName, nGroupID, nCapsuleID);
	m_PartsHitInfoViewControl.Reload();
	m_PartsHitInfoViewControl.NotifyChanging();
}

void CMainApplication::SetPartsHitCapsuleInfo( int nGroupID, int nCapsuleID )
{
	m_PartsHitInfoViewControl.SetPartsHitCapsuleData(nGroupID, nCapsuleID);
}

void CMainApplication::ChangePartsHitCapsuleInfo(int nGroupID, int nCapsuleID)
{
	m_PartsHitInfoViewControl.ChangePartsHitCapsuleInfo(nGroupID, nCapsuleID);
}

void CMainApplication::CheckShowPartsHitCapsule()
{
	m_bShowPartsHitCapsule = GlobalObjects::g_pMainForm->partsHitCapsuleToolStripMenuItem->Checked;
}

// 메쉬 충돌 인덱스 출력
void CMainApplication::RenderMeshCollisionIndex()
{
	if (!m_bRenderActorCollision || m_pActor == NULL)
		return;

	//m_pActor->RenderMeshCollision();	//분홍색

	RRenderHelper::SetRenderState(false);

	RMatrix mTrans;
	RMatrix tt;

	vector<RActorCollisionObject>::iterator itColl = m_pActor->m_pCollision->m_vCollisions.begin();
	for (int i = 0; itColl != m_pActor->m_pCollision->m_vCollisions.end(); itColl++, i++)
	{
		RActorCollisionObject& obj = *itColl;
		mTrans = obj.m_matTransform * obj.m_pParent->GetWorldTransform();

		RRenderHelper::SetRenderState();

		RRenderHelper::RenderCapsule(mTrans, obj.m_Capsule, 0x66ff00ff);

		MCapsule cap = obj.m_Capsule;

		if (true)
		{
			RVector realCapBottom = cap.bottom * mTrans;
			RVector realCapTop = cap.top * mTrans;
			//RRenderHelper::RenderLine(RMatrix::IDENTITY, cap.bottom, cap.top, 0xFFFF0000);

			RVector t1 = realCapBottom;
			t1.z = -40.f;
			RRenderHelper::RenderLine(RMatrix::IDENTITY, realCapBottom, t1, 0xFFFF00FF);

			if( true)
			{
				// billboard
				rs3::RFont::BeginFont(/*D3DXSPRITE_ALPHABLEND | D3DXSPRITE_OBJECTSPACE*/);

				// DrawTextIn3DSpace를 사용하려면 VIEW를 설정해 주어야 합니다.
				// BeginFont에서 VIEW를 보관하고 End에서 복구해 주기 때문에 BeginFont 이후에 설정 되어야 합니다.

				m_pDevice->SetTransform(RST_VIEW, m_pCamera->GetViewMatrix());

				rs3::RMatrix matInverse;
				m_pCamera->GetViewMatrix().GetInverse( &matInverse );

				rs3::RMatrix matTextReflectAndScale
					(
					5.0f,    0.f,    0.f,   0.f,
					0.f,     -5.0f,  0.f,   0.f,
					0.f,      0.f,    5.0f, 0.f,
					0.f,      0.f,    0.f,   1.f
					);

				matInverse._41 = t1.x;
				matInverse._42 = t1.y;
				matInverse._43 = t1.z;

				RMatrix matT = matTextReflectAndScale * matInverse;

				char szTmp[ 256];
				_itoa( i, szTmp, 10);
				GetFont()->DrawTextIn3DSpace( matT, MLocale::ConvAnsiToUTF16(szTmp).c_str() );

				rs3::RFont::EndFont();
			}
		}
	}

	RRenderHelper::EndRenderState();
}

void CMainApplication::InitTalentHitInfo()
{
	GlobalObjects::g_pMainForm->m_TalentTabPage->TalentIDtextbox->Text = nullptr;

	GlobalObjects::g_pMainForm->m_TalentTabPage->HitSegmenttreeView->ClearTreeView();
	GlobalObjects::g_pMainForm->m_TalentTabPage->Capsule_propertyGrid->SelectedObject = nullptr;
}

WEAPON_TYPE CMainApplication::GetWeaponType(CSTalentInfo* pTalentInfo)
{
	for (int j = 0; j < (int)WEAPON_TYPE_MAX; j++)
	{
		int nSkillID = MIN_TALENT_NORMAL_ATTACK + (j * 1000);
		int nSkillIDEnd = nSkillID+999;

		if( nSkillID < pTalentInfo->m_nID && pTalentInfo->m_nID < nSkillIDEnd)
		{
			return (WEAPON_TYPE)j;
		}
	}

	return WEAPON_NONE;
}

bool CMainApplication::CompareTalentTime(CSTalentInfo* pTalentInfo)
{
	if (pTalentInfo == NULL) return false;

	// 탤런트 시간
	TTalentActAnimationTime TalentTime;
	bool bChange = TalentTime.CalTalentActAnimationTime(pTalentInfo);
	if(bChange)
	{
		mlog("정보 : TalentID = %d : 탈렌트 시간이 변경되었습니다. \r", pTalentInfo->m_nID);
		for(vector<CSTalentActAnimationTimeInfo>::iterator itActAniTime = pTalentInfo->m_ActAnimationTime.vecInfo.begin();
			itActAniTime != pTalentInfo->m_ActAnimationTime.vecInfo.end(); itActAniTime++)
		{
			if((*itActAniTime).fAniTime > 0)
				mlog("--> WeaponType = (%d), 변경시간 %f \r", (int)(*itActAniTime).nWeaponType, (*itActAniTime).fAniTime);
		}

		if(pTalentInfo->m_fExtraMotionTime > 0)
			mlog("--> Extra 변경시간 %f \r", pTalentInfo->m_fExtraMotionTime);
		if(pTalentInfo->m_fExtraMotionTime2 > 0)
			mlog("--> Extra2 변경시간 %f \r", pTalentInfo->m_fExtraMotionTime2);
		if(pTalentInfo->m_fExtraMotionTime3 > 0)
			mlog("--> Extra3 변경시간 %f \r", pTalentInfo->m_fExtraMotionTime3);

		GlobalObjects::g_SaveNotifier.SetSaveTalentTime(true);
		return true;
	}

	return false;
}

float CMainApplication::GetExtraMotionTime(CSTalentInfo* pTalentInfo)
{
	if (pTalentInfo == NULL) return 0.f;
	if (m_pMotion == NULL) return 0.f;

	float time = 0.0f;

	string strMotionName = pTalentInfo->m_szExtraActAnimation;

	if (strMotionName == "") return time;

	if(m_pMotion->ChangeMotion(strMotionName.c_str()))
	{
		time += GetMotionTime();
	}
	else
	{
		m_pMotion->SetMotion(NULL);

		string motionname = GetTalentAniName(strMotionName, pTalentInfo->m_nID);
		if (SetAnimation(motionname.c_str()))
		{
			time += GetMotionTime();
		}
	}

	return time;
}

float CMainApplication::GetMotionTime()
{
	if (m_pMotion == NULL) return 0.f;

	return m_pMotion->GetCurTalentMaxFrametime();
}

/// 모션에 추가해준 애니시퀀스가 있는 탈렌트는 지워주고 끝내기.
void CMainApplication::CheckCustomMotion()
{
	TTalentInfoMgr::iterator beginitor = m_TTalentInfoMgr.begin();
	TTalentInfoMgr::iterator enditor = m_TTalentInfoMgr.end();

	while (beginitor != enditor)
	{
		CSTalentInfo* info = (CSTalentInfo*)((*beginitor).second);

		if (info)
			DeleteCustomMotion(info);

		beginitor++;
	}	
}

void CMainApplication::DeleteCustomMotion( CSTalentInfo* info )
{
	int fndpos = info->m_szUseAnimation.find(";");
	if (fndpos != string::npos)
	{
		string strMotionName = info->m_szUseAnimation.substr(0, fndpos);
		XMotion* pMotion = m_pMotion->GetMotionMgr()->GetMotion(strMotionName.c_str());
		if(pMotion && pMotion->m_vecAniSequence[MT_DEFAULT].size() == 2)
			pMotion->m_vecAniSequence[MT_DEFAULT].pop_back();
	}
}

void CMainApplication::SetPlayer()
{
	char szBuf[_MAX_DIR];
	sprintf_s(szBuf, "hf/hf.elu");

	MeshFileOpen(szBuf);

	MeshInfoRefresh("hf");
}

// TODO : 하나의 탈렌트의 시간을 구하려고 빼보았는 데 잘 안되네.
void CMainApplication::TalentTimeRefresh( CSTalentInfo* pTalentInfo, int &prevNPCID, bool &bPlayer )
{
	CompareTalentTime(pTalentInfo);
}

//모든 탈렌트 타임 구하기
void CMainApplication::AllTalentTimeRefresh()
{
	TTalentInfoMgr::iterator beginitor = m_TTalentInfoMgr.begin();
	TTalentInfoMgr::iterator enditor = m_TTalentInfoMgr.end();

	int		prevNPCID = 0;
	bool	bPlayer = false;
	int		nTalentSize = m_TTalentInfoMgr.size();
	int		nCurTalentCount = 0;

	while (beginitor != enditor)
	{
		CSTalentInfo* pTalentInfo = (CSTalentInfo*)((*beginitor).second);
		if (!pTalentInfo)
		{
			beginitor++;
			continue;
		}

		TalentTimeRefresh(pTalentInfo, prevNPCID, bPlayer);

		nCurTalentCount++;

		float fRate = ((float)nCurTalentCount / (float)nTalentSize) * 100;
		SetProgressBarValue((int)fRate);

		beginitor++;
	}

	DeleteActor();
	//리스트 지워주기
	GlobalObjects::g_pMainForm->m_AnimationTabPage->AnitreeView->Nodes->Clear();
	GlobalObjects::g_pMainForm->m_AnimationTabPage->m_AnimationEventTabPage->Event_checkedListBox->Items->Clear();
}

//메쉬이름으로 구분할수밖에...
bool CMainApplication::IsPlayer()
{
	if(GetActor())
	{
		//string name = GetActor()->GetMeshName();
		//if ((name == "hf.elu") || (name == "hm.elu"))
		//	return true;
		if(m_NPC.GetNPCID() == 0)
			return true;
	}

	return false;
}

void CMainApplication::ModelLoad( String^ nodeName, String^ parentName, String^ modelname )
{
	// 중복 체크
	if(GetActor() &&
		GetActor()->GetMeshName() == TBaseHelper::GetFileName(MStringToCharPointer(modelname)))
	{
		// 이미 로딩이 되어 있으므로 패스
		return;
	}

	m_TabModelInfo.NodeNoneSelectedButtonClick();

	String^ FileName = String::Format("{0}", modelname);

	SetModelFullPathName(modelname);

	m_TabModel.SetModelType(parentName);
	m_TabModel.SetPlayerType(nodeName);

	GlobalObjects::g_pMainForm->m_ParentName = parentName;
	if(MeshFileOpen(FileName))
	{
		SetMeshInfo(nodeName);
		SetMeshHitInfo(nodeName);
		SetMeshColInfo(nodeName);
		SetMeshHitEffectInfo(nodeName);

		m_NPC.SetNPCID(0);
		//if (false == parentName->StartsWith(String::Format("Player")))
		//{
		//	m_NPC.m_NPCID = 10000;
		//}

		m_TabModelInfo.SetBipTreeView();

		const char* cMeshName = MStringToCharPointer(nodeName);
		MeshInfoRefresh(cMeshName);
		MFreeCharPointer(cMeshName);
	}
}

void CMainApplication::treeViewMeshList_NodeMouseClick()
{
	// 현재 선택된 노드의 최상위 노드를 얻는다. (Parent == NULL)
	TreeNode^ treeNode = GlobalObjects::g_pMainForm->treeViewMeshList->SelectedNode;

	if( treeNode == nullptr )
		return;

	if( treeNode->GetNodeCount(false) )
		return;

	while( treeNode->Parent != nullptr )
		treeNode = treeNode->Parent;	// Root Node // 이것이 Mesh Name 이다.

	// 해당하는 MeshNode를 찾고 이것을 SelectedMesh에 담는다.
	m_pSelectedActorNode = m_pActor->GetActorNode( MStringToCharPointer(treeNode->Text) );

	GlobalObjects::g_pMainForm->textBoxCurrentMtrl->Text = gcnew String( m_pSelectedActorNode->GetNodeName().c_str() );

	m_Mtrl.SetMtrlList();
}

void CMainApplication::SetMeshColInfo( System::String^ ModelName )
{
	m_MeshColInfoViewControl.ChangeMeshColInfoViewer(ModelName);
}

void CMainApplication::CheckShowMeshColCapsule()
{
	m_bShowMeshColCapsule = GlobalObjects::g_pMainForm->toolStripMenuItem2->Checked;
}

void CMainApplication::ChangeMeshColInfo( int nCapsuleID )
{
	m_MeshColInfoViewControl.ChangeMeshCoInfo(nCapsuleID);
}

int CMainApplication::CreateMeshColCapsuleInfo()
{
	int nCapsuleID = TMeshInfoMgr::GetInstance().GetMeshColInfo()->CreateCapsule(m_strModelFullPathName);

	if(nCapsuleID == -1)
	{
		System::Windows::Forms::MessageBox::Show(String::Format("{0}에 대한 히트 정보가 없습니다.", gcnew String(TMeshInfoMgr::GetInstance().GetMeshHitInfo()->GetModelName(m_strModelFullPathName).c_str())), "충돌 캡슐 생성 에러");
		return -1;
	}

	m_MeshColInfoViewControl.CreateMeshColCapsuleInfoViewer(nCapsuleID);
	m_MeshColInfoViewControl.NotifyChanging();
	return nCapsuleID;
}

void CMainApplication::DelMeshColCapsuleInfo( int nCapsuleID )
{
	TMeshInfoMgr::GetInstance().GetMeshColInfo()->DelCapsule(m_strModelFullPathName, nCapsuleID);
	m_MeshColInfoViewControl.Reload();
	m_MeshColInfoViewControl.NotifyChanging();
}

void CMainApplication::SetCapsuleRenderState()
{
	RRenderHelper::SetRenderState();

	REngine::GetDevice().SetDepthEnable(true, false);
}

void CMainApplication::EndCapsuleRenderState()
{
	REngine::GetDevice().SetDepthEnable(true);

	RRenderHelper::EndRenderState();
}


void CMainApplication::MoveColSegmentBottom( int dx, int dy, int dz )
{
	MCapsule* pCapsule = m_MeshColInfoViewControl.GetCurrentSelectCapsule();
	if (pCapsule == NULL)
	{
		return;
	}

	RVector v = RVector(0.f, 1.f, 0.f);// -m_pCamera->GetDirection();//m_pActor->GetDirection();

	v.z = 0;
	v.Normalize();

	RVector side;

	side = v.CrossProduct(RVector::AXISZ);

	RVector mov;

	mov = dy * 3.0f * v;
	mov += dx * 3.0f * -side;
	mov += dz * 3.0f * RVector::AXISZ;

	pCapsule->bottom += mov;
	pCapsule->CalcHeight();

	GlobalObjects::g_SaveNotifier.SetSaveMeshInfo(true);
}

void CMainApplication::MoveColSegmentTop( int dx, int dy, int dz )
{
	MCapsule* pCapsule = m_MeshColInfoViewControl.GetCurrentSelectCapsule();
	if (pCapsule == NULL)
	{
		return;
	}

	RVector v = RVector(0.f, 1.f, 0.f);
	v.z = 0;
	v.Normalize();

	RVector side;
	side = v.CrossProduct(RVector::AXISZ);

	RVector mov;
	mov = dy * 3.0f * v;
	mov += dx * 3.0f * -side;
	mov += dz * 3.0f * RVector::AXISZ;

	pCapsule->top += mov;
	pCapsule->CalcHeight();

	GlobalObjects::g_SaveNotifier.SetSaveMeshInfo(true);
}

void CMainApplication::SaveAll()
{
	GlobalObjects::g_SaveNotifier.SaveAll();
}

bool CMainApplication::TalentPosInfoRefresh(CSTalentInfo* pTalentInfo)
{
	if(m_TTalentInfoMgr.TalentPosInfoRefresh(pTalentInfo))
	{
		GlobalObjects::g_SaveNotifier.SetSaveTalentPosInfo(true);
		return true;
	}

	return false;
}

// TODO : 정보가 바뀌었는 지 확인이 필요하다.
bool CMainApplication::MeshInfoRefresh(string strModelName)
{
	if(TMeshInfoMgr::GetInstance().RefreshMeshInfo(strModelName))
	{
		GlobalObjects::g_SaveNotifier.SetSaveMeshInfo(true);
		return true;
	}

	return false;
}

void CMainApplication::SaveModelAnimation()
{
	if( GetActor() == NULL)
	{
		System::Windows::Forms::MessageBox::Show(L"저장할 모델이 없습니다.");
		return;
	}

	//흠 잘 안되면 저장해 놓은 태그로 하자. -- 태그가 저장되어있나요?
	String^ FileName = gcnew String(GetActor()->m_pMesh->GetName().c_str());
	String^ str = String::Format("../../Data/Model/{0}/{1}",
		GlobalObjects::g_pMainForm->m_ParentName,FileName);

	if (FileName->StartsWith("../../") ||
		GlobalObjects::g_pMainForm->tabControl1->SelectedIndex == 1)		//NPC 리스트 탭에서 작업중이면 fileName레 경로가 다 포함되어 있으므로 그걸 사용.
	{
		str = String::Format("{0}", FileName);
	}

	SetCurrentDir();
	SaveAnimationXml(str);

	GlobalObjects::g_SaveNotifier.SetSaveAnimation(false);

	// 샘플링 저장
	//m_SwordTrailSampling.Save();

	// [1/22/2008 isnara] Ctrl + S를 누르면 기본적으로 저장해 주는 내용들...
	// 탤런트 이펙트, 이벤트, 충돌 정보 저장
	//SaveTalentExt(true, true, true);

	// NPC 충돌구 정보 저장
	//OnSaveMeshInfo();

}

void CMainApplication::PartsColorPreview()
{
	if(GlobalObjects::g_pMainForm->m_ColorPickerDialog->Visible)
	{
		DWORD dwColor = GlobalObjects::g_pMainForm->m_ColorPickerDialog->GetColor();
		for (int i =0; i < (int)m_TabModelInfo.m_NodeList.size(); i++)
		{
			RActorNode* pActorNode = m_TabModelInfo.m_NodeList[i];
			if (pActorNode == NULL) continue;

			pActorNode->SetPartsColor(dwColor);
		}

		for(vector<RActorNode*>::iterator it = m_TabModelInfo.m_PartsColorTestList.begin(); it != m_TabModelInfo.m_PartsColorTestList.end(); ++it)
		{
			if ((*it) == NULL) 
				continue;

			(*it)->SetPartsColor(dwColor);
		}
	}
	else
	{
		m_TabModelInfo.m_PartsColorTestList.clear();
	}
}

void CMainApplication::ReloadCharacter()
{
	if( GetActor() == NULL) return;

	string str = GetActor()->m_pMesh->GetName();
	mlog("정보 : %s 파일을 다시 로드합니다. \r", str.c_str());

	DeleteActor();
	m_pActorMgr->ModelDel(str.c_str());
	CharFileOpen(str.c_str());
}

void CMainApplication::SelectTalent(int nTalentID, int nTalentMode)
{
	CSTalentInfo* pTalentInfo = TTalentHelper::GetTalentInfo(nTalentID, nTalentMode);
	if (pTalentInfo == NULL)
	{
		mlog("에러 : 탈렌트가 없습니다. \r");
		return;
	}

	m_bTalentAnimation = true;

	SetCurrentTalentInfo(pTalentInfo );
	SetTalent(pTalentInfo);
	if( m_TalentAnimation.SetTalentAnimation(pTalentInfo, nTalentMode) == false)
	{
		OnStopAnimation();
				
		// 루프상태이면 푼다.
		if(m_bLoopAnimation)
			OnLoopAnimation();
	}

	m_bPlayPauseButtonStateByShourtCut = true;

	if(m_TabModel.GetModelType() != MODEL_TYPE_PLAYER)
	{
		// TTalentInfoMgr::SetupActor()에서 스케일을 적용하도록 되었있는데
		//제대로 안된다 그래서 여기에서 한번더 제대로 수정하자.
		UpdateModelScale();
	}

	// 탤런트 시간 구하기
	CompareTalentTime(pTalentInfo);

	//if(m_TabModel.GetModelType() != MODLE_TYPE::MODEL_TYPE_PLAYER)
	TalentPosInfoRefresh(pTalentInfo);

	GlobalObjects::g_pMainForm->m_FrameSliderController->pressPlayButton();
	GlobalObjects::g_pMainForm->m_TalentTabPage->m_TalentHitTabController->ClearQueue();
	GlobalObjects::g_pMainForm->m_AnimationTabPage->SetPauseButtonDown_AnimationLinkTest();

	GlobalObjects::g_pMainForm->m_TalentTabPage->m_TalentHitTabController->UpdateTalentHitEffectDir(pTalentInfo, true);
}

void CMainApplication::GetEffectInfoList( System::Collections::ArrayList^ effectList )
{
	vector<string> vecEffectList;

	g_pMainApp->m_TabAnimation.GetCustomEffectList(vecEffectList);

	for(vector<string>::iterator itEffectList = vecEffectList.begin(); itEffectList != vecEffectList.end(); itEffectList++)
	{
		effectList->Add(gcnew String((*itEffectList).c_str()));
	}
}

void CMainApplication::NPCListtreeView_AfterSelect(System::Object^  sender, System::Windows::Forms::TreeViewEventArgs^  e)
{

	//최상위노드이면
	if( e->Node == nullptr )		return;

	//------------------------------------------------------------------------
	// 데이터 초기화

	// 탤런트 히트
	InitTalentHitInfo();

	// 탤런트 이펙트, 이벤트
	SetTalent(NULL);

	//------------------------------------------------------------------------
	// 데이터 셋팅
	GlobalObjects::g_pMainForm->m_ParentName = "NPC";//parentName;
	Object^ tag = e->Node->Tag;
	if (tag == nullptr) return;
	if( e->Node->Parent == nullptr ||
		e->Node->Text == "탈렌트")
	{
		Object^ idtag = e->Node->Tag;
		if (idtag == nullptr) return;
		int NPCID = (int)idtag;

		if( !m_NPC.IsValidNPCID(NPCID) )
		{
			m_TalentAnimation.InitTalentAnimation();
			m_NormalAnimation.InitNormalAnimation();

			m_NPC.SetNPCID(NPCID);
			if(m_NPC.SetNPC(NPCID))
			{
				// 현재 선택한 BaseMesh의 재질 리스트를 읽어 들인다.
				m_Mtrl.SetCurrentMaterialList(NPCID);
				m_Mtrl.SetNPCInfoSubMtrl(NPCID);
				m_Mtrl.SetMeshTreeView();
				m_Mtrl.SetMtrlList();
			}
		}
	}
	else if(e->Node->Parent->Text == "탈렌트")
	{
		Object^ idtag = e->Node->Parent->Parent->Tag;
		if (idtag == nullptr) return;
		int NPCID2 = (int)idtag;

		if( !m_NPC.IsValidNPCID(NPCID2) )
		{
			m_NPC.SetNPCID(NPCID2);
			m_NPC.SetNPC(NPCID2);
		}

		Object^ skilltag = e->Node->Tag;
		if (skilltag ==nullptr) return;

		GlobalObjects::g_pMainForm->m_TalentTabPage->m_TalentHitTabController->ClearSel();

		SelectTalent((int)skilltag, 0);

		CSTalentInfo* pTalentInfo = TTalentHelper::GetTalentInfo((int)skilltag, 0);
		if (pTalentInfo == NULL)
			return;
		
		UpdateShowNPCHitCapsuleGroup(m_bShowNPCHitCapsuleGroup, pTalentInfo);
	}
}

void CMainApplication::DrawHitArea()
{
	GlobalObjects::g_pMainForm->m_TalentTabPage->m_TalentHitTabController->Render();
}

void CMainApplication::DrawPartsHitCapsule()
{
	// [1/7/2008 isnara] 부분 판정 렌더링
	if((m_bShowPartsHitCapsule &&														// 계속 렌더링을 하겠습니까?
		GlobalObjects::g_pMainForm->PartsHitCapsulesView->SelectedNode != nullptr) ||
		(m_bShowNPCHitCapsuleGroup &&
		GlobalObjects::g_pMainForm->m_NPCListTabPage->NPCListtreeView->SelectedNode != nullptr))
	{
		m_PartsHitInfoViewControl.RenderGroup(GlobalObjects::g_SelInfo.m_nMeshHitGroupID, GlobalObjects::g_SelInfo.m_nMeshHitCapsuleID);

		// 히트캡슐과 연결되어있는 히트이펙트캡슐 보여주기
		if(m_bShowPartsHitCapsuleLinkedHitEffect)
		{
			CSHitCapsule* pHitCapsule = m_PartsHitInfoViewControl.GetPartsHitCapsuleInfo(GlobalObjects::g_SelInfo.m_nMeshHitGroupID, GlobalObjects::g_SelInfo.m_nMeshHitCapsuleID);
			if(pHitCapsule)
			{
				m_MeshHitEffectInfoViewControl.RenderGroup(pHitCapsule->m_nHitEffectCapsuleID, true);
			}
		}
	}

	if(m_bShowMeshColCapsule &&															// 계속 렌더링을 하겠습니까?
		GlobalObjects::g_pMainForm->CollisionCapsuleAtt->SelectedObject != nullptr)		// 메쉬 충돌 속성창이 활성화 되어 있습니까?
	{
		m_MeshColInfoViewControl.RenderGroup(GlobalObjects::g_SelInfo.m_nMeshColID);
	}


	if(m_bShowMeshHitEffectCapsule &&
		GlobalObjects::g_pMainForm->HitEffect_propertyGrid->SelectedObject != nullptr)
	{
		m_MeshHitEffectInfoViewControl.RenderGroup(GlobalObjects::g_SelInfo.m_nMeshHitEffectCapsuleID);
	}
}

//void CMainApplication::OnOffBreakableParts(bool bChecked)
//{
//	if (GetActor() == NULL) return;
//
//	for (int nBPartID=1; nBPartID<=MAX_BREAKABLE_PARTS; nBPartID++)
//	{
//		OnOffBreakableParts(nBPartID, bChecked);
//	}
//}

void CMainApplication::OnOffBreakableParts(int nIndex, bool bChecked)
{
	if (GetActor() == NULL ||
		nIndex <= 0) return;

	char chNodeName[257] = {0,};
	sprintf(chNodeName, "%s%02d", "b_parts_", nIndex);
	RActorNode* pBreakablePartsNode = GetActor()->GetActorNode(chNodeName);
	if (pBreakablePartsNode)
	{
		if (pBreakablePartsNode->GetVisible() )
		{
			pBreakablePartsNode->SetVisible(false);
		}
		else
		{
			pBreakablePartsNode->SetVisible(true);
		}
	}
}

int CMainApplication::BreakablePartsCount()
{
	if (GetActor() == NULL) return 0;
	
	int nCount = 0;

	for (int nBPartID=1; nBPartID<=MAX_BREAKABLE_PARTS; nBPartID++)
	{
		char chNodeName[257] = {0,};
		sprintf(chNodeName, "%s%02d", "b_parts_", nBPartID);
		RActorNode* pBreakablePartsNode = GetActor()->GetActorNode(chNodeName);
		if (pBreakablePartsNode)
			nCount++;
	}

	return nCount;
}

//void CMainApplication::CheckBreakableParts()
//{
//	InitBreakablePartsMenu();
//
//	int nCount = BreakablePartsCount();
//	
//	if(nCount > MAX_BREAKABLE_PARTS)
//	{
//		// 브레이커블 파츠가 오버했다.
//		System::Windows::Forms::MessageBox::Show(L"브레이커블 파츠 최대 개수(4개)를 넘었습니다.", "에러!");
//
//		nCount = MAX_BREAKABLE_PARTS;
//	}
//
//	for(int nBPartID = 1; nBPartID <= nCount; ++nBPartID)
//	{
//		System::Windows::Forms::ToolStripMenuItem^ pPartsMenuItem = GetBreakablePartsMenuItem(nBPartID);
//
//		if(pPartsMenuItem != nullptr)
//		{
//			pPartsMenuItem->Visible = true;
//			pPartsMenuItem->Checked = true;
//		}
//	}
//}

//void CMainApplication::InitBreakablePartsMenu()
//{
//	for (int nBPartID=1; nBPartID<=MAX_BREAKABLE_PARTS; nBPartID++)
//	{
//		System::Windows::Forms::ToolStripMenuItem^ pPartsMenuItem = GetBreakablePartsMenuItem(nBPartID);
//		if(pPartsMenuItem != nullptr)
//			pPartsMenuItem->Visible = false;
//	}
//}

//System::Windows::Forms::ToolStripMenuItem^ CMainApplication::GetBreakablePartsMenuItem( int nIndex )
//{
//	switch(nIndex)
//	{
//	case 1:
//		return GlobalObjects::g_pMainForm->b_part_01_ToolStripMenuItem;
//	case 2:
//		return GlobalObjects::g_pMainForm->b_part_02_ToolStripMenuItem;
//	case 3:
//		return GlobalObjects::g_pMainForm->b_part_03_ToolStripMenuItem;
//	case 4:
//		return GlobalObjects::g_pMainForm->b_part_04_ToolStripMenuItem;
//	}
//
//	return nullptr;
//}

void CMainApplication::ModeltreeView_AfterSelect( System::Windows::Forms::TreeViewEventArgs^ e )
{
	// TODO: NPC탭을 왔다갔다 하면 먹통일때가 있다...
	//최상위노드이면 리턴
	if( e->Node->Parent == nullptr ) return;
	if( e->Node->Tag == nullptr ) return;

	Object^ tag = e->Node->Tag;
	String^ modelname(tag->ToString());

	String^ nodeName(e->Node->Text);
	String^ parentName(e->Node->Parent->Text);

	m_bTalentAnimation = false;
	m_TalentAnimation.InitTalentAnimation();
	m_NormalAnimation.InitNormalAnimation();

	//String^ FileName = String::Format("../../Data/Model/{0}/{1}/{1}.elu", parentName, nodeName);
	ModelLoad(nodeName, parentName, modelname);

	GlobalObjects::g_pMainForm->m_NPCListTabPage->NPCListtreeView->SelectedNode = nullptr;

	//tabControl2->SelectedIndex = 3;
}

void CMainApplication::UpdateProjectile( float fElapsed )
{
	vector<TProjectile*>::iterator itProjectile = m_vecProjectile.begin();
	while(itProjectile != m_vecProjectile.end())
	{
		(*itProjectile)->Update(fElapsed);

		if((*itProjectile)->IsCol())
		{
			SAFE_DELETE(*itProjectile);
			itProjectile = m_vecProjectile.erase(itProjectile);
			continue;
		}

		itProjectile++;
	}
}

void CMainApplication::UpdateMagicArea( float fElapsed )
{
	m_MagicArea.OnUpdate(fElapsed);
}

void CMainApplication::NPCListtreeView_NodeMouseClick( System::Object^ sender, System::Windows::Forms::TreeNodeMouseClickEventArgs^ e )
{
	//최상위노드이면
	if( e->Node == nullptr ||
		e->Node->Parent == nullptr)
		return;

	// 탤런트를 선택했을때에만 반응
	if(e->Node->Parent->Text == "탈렌트")
	{
		Object^ idtag = e->Node->Parent->Parent->Tag;
		if (idtag == nullptr) return;
		int NPCID2 = (int)idtag;

		if( !m_NPC.IsValidNPCID(NPCID2) )
		{
			m_TalentAnimation.InitTalentAnimation();
			m_NormalAnimation.InitNormalAnimation();

			m_NPC.SetNPCID(NPCID2);
			m_NPC.SetNPC(NPCID2);
		}

		Object^ skilltag = e->Node->Tag;
		if (skilltag ==nullptr) return;

		GlobalObjects::g_pMainForm->m_TalentTabPage->m_TalentHitTabController->ClearSel();

		SelectTalent((int)skilltag, 0);
	}
}

void CMainApplication::DelProjectile()
{
	vector<TProjectile*>::iterator itProjectile = m_vecProjectile.begin();
	while(itProjectile != m_vecProjectile.end())
	{
		SAFE_DELETE(*itProjectile);

		itProjectile++;
	}

	m_vecProjectile.clear();
}

void CMainApplication::OnMouseClick( int stat, RVector2& pos, int delta )
{
	if(m_TMenu.CheckDrawBone(SBS_SIMPLE) &&
		m_bCameraMove == false)
	{
		OnMouseClickBoneNode(stat, pos, delta);
	}

	m_bCameraMove = false;
}

void CMainApplication::OnMouseClickBoneNode( int stat, RVector2& pos, int delta )
{
	if(stat != 1)
		return;

	RActorNode* pSelectNode = m_TabModelInfo.BoneSelect();

	SelectActorNode(pSelectNode);
}

void CMainApplication::MainView_KeyCheck( System::Windows::Forms::PreviewKeyDownEventArgs^ e )
{
	// 나중에 내용이 늘어나면 클래스로 따로 분류
	// Ctrl
	if(e->Control)
	{
		switch (e->KeyCode)
		{
		case Keys::C:
			// 복사
			// 현재 본 이름만 복사를한다.
			Clipboard::SetText(gcnew String(m_TabModelInfo.GetSelectNodeName().c_str()));
			break;
		}
	}
}

void CMainApplication::AnimationEventPlay( RAnimationEvent* pAnimationEvent )
{
	m_eventListener.OnAnimationEvent(m_pActor, pAnimationEvent);	
}

void CMainApplication::DrawExistDummyLoc()
{
	System::Drawing::Graphics^ formGraphics		= GlobalObjects::g_pMainForm->dummy_loc_panel->CreateGraphics();	
	formGraphics->Clear(System::Drawing::Color::SlateGray);
	
	CHECK_DUMMY_LOC_RESULT result;
	if(m_bTalentAnimation)
		result = m_TalentAnimation.GetDummyLocResult();
	else
		result = m_NormalAnimation.GetDummyLocResult();

	// "dummy_loc"가 없다.
	if(result == CDLR_NO_DUMMY_NODE ||
		result == CDLR_NO_ANIMATION)
	{
		GlobalObjects::g_pMainForm->dummy_loc_panel->Hide();
		return;
	}

	GlobalObjects::g_pMainForm->dummy_loc_panel->Show();
	Size panelSize = GlobalObjects::g_pMainForm->dummy_loc_panel->Size;

	System::Drawing::Font^ drawFont				= gcnew System::Drawing::Font("굴림", 8);
	System::Drawing::StringFormat^ drawFormat	= gcnew System::Drawing::StringFormat;
	drawFormat->Alignment						= System::Drawing::StringAlignment::Center;
	drawFormat->LineAlignment					= System::Drawing::StringAlignment::Center;

	if(result == CDLR_NOT_USE)
	{
		// 에러
		System::Drawing::Pen^ myPen					= gcnew System::Drawing::Pen(System::Drawing::Color::Black);
		System::Drawing::SolidBrush^ fontBrush		= gcnew System::Drawing::SolidBrush(System::Drawing::Color::White);			
		System::Drawing::SolidBrush^ myBrush		= gcnew System::Drawing::SolidBrush(System::Drawing::Color::Blue);
		RectangleF drawRect = RectangleF(0, 0, (float)panelSize.Width, (float)panelSize.Height);
		formGraphics->FillRectangle(myBrush, drawRect);

		formGraphics->DrawString(gcnew String("dummy_loc 안움직임"), drawFont, fontBrush, drawRect, drawFormat);
		formGraphics->DrawRectangle(myPen, 0, 0, panelSize.Width, panelSize.Height);
	}
	else
	{
		// 성공
		System::Drawing::Pen^ myPen					= gcnew System::Drawing::Pen(System::Drawing::Color::Black);
		System::Drawing::SolidBrush^ fontBrush		= gcnew System::Drawing::SolidBrush(System::Drawing::Color::Black);			
		System::Drawing::SolidBrush^ myBrush		= gcnew System::Drawing::SolidBrush(System::Drawing::Color::White);
		RectangleF drawRect = RectangleF(1, 1, (float)panelSize.Width-1, (float)panelSize.Height-1);
		formGraphics->FillRectangle(myBrush, drawRect);
		formGraphics->DrawRectangle(myPen, 0, 0, panelSize.Width, panelSize.Height);

		formGraphics->DrawString(gcnew String("dummy_loc 움직임"), drawFont, fontBrush, drawRect, drawFormat);
	}
}

void CMainApplication::ShowHideSamplePlayerModel( bool bShow )
{
	if(bShow == false)
	{
		GlobalObjects::g_pMainForm->showPlayerModelToolStripMenuItem->Checked = false;
	}

	if(m_pSampleActor)
	{
		// 있다면.. 사라진다.
		m_SampleActorItem.AllUnEquipItem();
		m_pSampleActor->RemoveFromParent();
		m_pSampleActor = NULL;

		return;
	}

	if(m_pActor == NULL)
		return;

	string strMeshName = m_pActor->m_pMesh->GetName();
	if(strMeshName.find("NPC") == string::npos &&
		strMeshName.find("Monster") == string::npos)
	{
		GlobalObjects::g_pMainForm->showPlayerModelToolStripMenuItem->Checked = false;
		return;
	}

	// 없다면...
	m_pSampleActor = m_pActorMgr->GetFemalePlayerModel();

	m_SampleActorItem.SetEquipItemData(&m_ItemMgr, &m_TabModel, &m_ClothMgr);
	m_SampleActorItem.LoadEquipItems(m_pSampleActor);

	vec3 vStandPos(m_pActor->GetAABB().maxx + 20, 0, 0);
	m_pSampleActor->SetPosition(vStandPos);
	m_pSampleActor->UpdateTransform();

	if(m_pSampleActor)
		m_pSceneManager->AddSceneNode(m_pSampleActor);
}

void CMainApplication::OnPlayAnimationByShourtCut()
{
	if( GetActor() == NULL) 
		return;

	bool bStop = false;
	if(m_pActor->IsPlayDone() ||
		m_pActor->GetAnimationState() == RPS_STOP)
	{
		bStop = true;
		m_bPlayPauseButtonStateByShourtCut = false;
	}

	if(m_bPlayPauseButtonStateByShourtCut)
	{
		m_pActor->PauseAnimation();
		m_bPlayPauseButtonStateByShourtCut = false;

		if(m_NormalAnimation.GetUseAnimationLinkListTest())
			GlobalObjects::g_pMainForm->m_AnimationTabPage->SetPauseButtonDown_AnimationLinkTest();
		else
			GlobalObjects::g_pMainForm->m_FrameSliderController->pressPauseButton();
	}
	else
	{
		if(bStop)
			OnPlayAnimation();

		UpdateModelScale();
		m_pActor->PlayAnimation();
		m_bPlayPauseButtonStateByShourtCut = true;

		if(m_NormalAnimation.GetUseAnimationLinkListTest())
			GlobalObjects::g_pMainForm->m_AnimationTabPage->SetPlayButtonDown_AnimationLinkTest();
		else
			GlobalObjects::g_pMainForm->m_FrameSliderController->pressPlayButton();
	}
}

bool CMainApplication::FileModifyCheck( CheckModifyData eModifyName )
{
	if(eModifyName == CHECK_MODIFY_FILENAME_EFFECT)
		return m_pEffectInfoMgr->CheckEffectInfoModify();
	else if(eModifyName == CHECK_MODIFY_FILENAME_SOUND)
		return m_pSoundMgr.CheckSoundInfoModify();
	else if(eModifyName == CHECK_MODIFY_FILENAME_TALENT)
	{
		if(m_TTalentInfoMgr.CheckTalentInfoModify())
		{
			SAFE_DELETE(m_pTalentEffectMgr);

			m_pTalentEffectMgr = new TTalentEffectMgr(&m_TTalentInfoMgr);

			m_TalentAnimation.Reload();
			m_pTalentInfo = NULL;
			return true;
		}
	}
	else if(eModifyName == CHECK_MODIFY_FILENAME_BUFF_EFFECT_INFO)
	{
		return m_BuffInfo.CheckTalentInfoModify();
	}
	else if(eModifyName == CHECK_MODIFY_FILENAME_MOTION_INFO && m_pMotion)
	{
		SetCurrentDirectory("../../");

		bool bResult = m_pMotion->CheckMotionInfoModify();

		SetCurrentDir();

		return bResult;
	}
	else if(eModifyName == CHECK_MODIFY_FILENAME_NPC_INFO)
	{
		// 현 플레이어 모델 이외 모두 삭제
		m_pActor = NULL;
		m_pMotion->SetActor(NULL);
		m_pMotion->CurMotionClear();
		m_pMotion->InitMotion();
		m_EquipItems.SetActor(NULL);
		m_EquipItems.AllUnEquipItem();
		m_NormalAnimation.SetActor(NULL);
		m_NormalAnimation.InitNormalAnimation();
		m_TalentAnimation.SetActor(NULL);
		m_TalentAnimation.InitTalentAnimation();
		m_AnimationBledTable.SetActor(NULL);

		m_pActorMgr->ActorListDestroy();

		SetMeshInfo(gcnew String(""));
		SetMeshHitInfo(gcnew String(""));
		SetMeshColInfo(gcnew String(""));
		SetMeshHitEffectInfo(gcnew String(""));
		MeshInfoRefresh(string(""));

		// NPC 리스트 삭제
		GlobalObjects::g_pMainForm->m_NPCListTabPage->NPCListtreeView->Nodes->Clear();
		GlobalObjects::g_pMainForm->m_AnimationTabPage->AnitreeView->Nodes->Clear();
		m_TabAnimation.InitAnitreeView();
		m_TabAnimation.InitEventList();
		m_NPC.SetNPCID(0);

		// 리로드
		bool bResult = TNPCInfoMgr::GetInstance().CheckMotionInfoModify();

		GlobalObjects::g_pMainForm->m_NPCListTabPage->ReLoadNPCList();

		return bResult;
	}

	return false;
}

void CMainApplication::AmbientColorPreview()
{
	if(GlobalObjects::g_pMainForm->m_AmbientColorPickerDialog->Visible)
	{
		DWORD dwColor = GlobalObjects::g_pMainForm->m_AmbientColorPickerDialog->GetColor();
		RVector4 ambient = RVector4(dwColor);
		g_pMainApp->SetLightAmbient(ambient);
	}
}

void CMainApplication::ShowTestHitEffect( bool bShow )
{
	m_TalentAnimation.SetShowHitEffect(bShow);
}

void CMainApplication::SetMeshHitEffectInfo( System::String^ ModelName )
{
	m_MeshHitEffectInfoViewControl.ChangeMeshHitEffectInfoViewer(ModelName);
}

int CMainApplication::CreateMeshHitEffectCapsuleInfo()
{
	int nCapsuleID = TMeshInfoMgr::GetInstance().GetMeshHitEffectInfo()->CraateHitEffectCapsule(m_strModelFullPathName);

	if(nCapsuleID == -1)
	{
		System::Windows::Forms::MessageBox::Show(String::Format("{0}에 대한 히트 정보가 없습니다.", gcnew String(TMeshInfoMgr::GetInstance().GetMeshHitInfo()->GetModelName(m_strModelFullPathName).c_str())), "판정 이펙트 생성 에러");
		return -1;
	}

	m_MeshHitEffectInfoViewControl.CreateMeshHitEffectInfoViewer(nCapsuleID);
	m_MeshHitEffectInfoViewControl.NotifyChanging();
	return nCapsuleID;
}

void CMainApplication::DelMeshHitEffectCapsuleInfo( int nCapsuleID )
{
	TMeshInfoMgr::GetInstance().GetMeshHitEffectInfo()->DelHitEffectCapsule(m_strModelFullPathName, nCapsuleID);
	m_MeshHitEffectInfoViewControl.Reload();
	m_MeshHitEffectInfoViewControl.NotifyChanging();
}

void CMainApplication::CheckShowMeshHitEffectCapsule()
{
	//m_bShowMeshHitEffectCapsule = GlobalObjects::g_pMainForm->toolStripMenuItem2->Checked;
}

void CMainApplication::ChangeMeshHitEffectCapsuleInfo( int nCapsuleID )
{
	m_MeshHitEffectInfoViewControl.ChangeMeshHitEffectCapsuleInfo(nCapsuleID);
}

void CMainApplication::OnMouseMoveHitEffectEdit( int stat, RVector2 &pos, int delta )
{
	if(IsKeyDown(VK_CONTROL))
	{
		if(GlobalObjects::g_pMainForm->Top_toolStripButton->Checked)
		{
			if( stat == 1)
			{
				if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
					MoveHitEffectSegmentBottom(m_nMousePrevX - (int)pos.x, 0, 0);
				if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
					MoveHitEffectSegmentBottom(0, m_nMousePrevY - (int)pos.y, 0);
				if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
					MoveHitEffectSegmentBottom(0, 0, m_nMousePrevY - (int)pos.y);
				if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
					MoveHitEffectSegmentBottom(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
			}
		}
		else if(GlobalObjects::g_pMainForm->Bottom_toolStripButton->Checked)
		{
			if (stat == 1)
			{
				if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
					MoveHitEffectSegmentTop(m_nMousePrevX - (int)pos.x, 0, 0);
				if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
					MoveHitEffectSegmentTop(0, m_nMousePrevY - (int)pos.y, 0);
				if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
					MoveHitEffectSegmentTop(0, 0, m_nMousePrevY - (int)pos.y);
				if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
					MoveHitEffectSegmentTop(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
			}
		}
		else if(GlobalObjects::g_pMainForm->TopBottom_toolStripButton->Checked)
		{
			if (stat == 1)
			{
				if( GlobalObjects::g_pMainForm->X_toolStripButton->Checked)
				{
					MoveHitEffectSegmentBottom(m_nMousePrevX - (int)pos.x, 0, 0);
					MoveHitEffectSegmentTop(m_nMousePrevX - (int)pos.x, 0, 0);
				}
				if( GlobalObjects::g_pMainForm->Y_toolStripButton->Checked)
				{
					MoveHitEffectSegmentBottom(0, m_nMousePrevY - (int)pos.y, 0);
					MoveHitEffectSegmentTop(0, m_nMousePrevY - (int)pos.y, 0);
				}
				if( GlobalObjects::g_pMainForm->Z_toolStripButton->Checked)
				{
					MoveHitEffectSegmentBottom(0, 0, m_nMousePrevY - (int)pos.y);
					MoveHitEffectSegmentTop(0, 0, m_nMousePrevY - (int)pos.y);
				}
				if( GlobalObjects::g_pMainForm->XY_toolStripButton->Checked)
				{
					MoveHitEffectSegmentBottom(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
					MoveHitEffectSegmentTop(m_nMousePrevX - (int)pos.x, m_nMousePrevY - (int)pos.y, 0);
				}
			}
		}
		else
			OnCameraMouseMove(stat, pos, delta);
	}
	else
		OnCameraMouseMove(stat, pos, delta);
}

void CMainApplication::MoveHitEffectSegmentBottom( int dx, int dy, int dz )
{
	MCapsule* pCapsule = m_MeshHitEffectInfoViewControl.GetCurrentSelectCapsule();
	if (pCapsule == NULL)
	{
		return;
	}

	RVector v = RVector(0.f, 1.f, 0.f);// -m_pCamera->GetDirection();//m_pActor->GetDirection();

	v.z = 0;
	v.Normalize();

	RVector side;

	side = v.CrossProduct(RVector::AXISZ);

	RVector mov;

	mov = dy * 3.0f * v;
	mov += dx * 3.0f * -side;
	mov += dz * 3.0f * RVector::AXISZ;

	pCapsule->bottom += mov;
	pCapsule->CalcHeight();

	GlobalObjects::g_SaveNotifier.SetSaveMeshInfo(true);
}

void CMainApplication::MoveHitEffectSegmentTop( int dx, int dy, int dz )
{
	MCapsule* pCapsule = m_MeshHitEffectInfoViewControl.GetCurrentSelectCapsule();
	if (pCapsule == NULL)
	{
		return;
	}

	RVector v = RVector(0.f, 1.f, 0.f);
	v.z = 0;
	v.Normalize();

	RVector side;
	side = v.CrossProduct(RVector::AXISZ);

	RVector mov;
	mov = dy * 3.0f * v;
	mov += dx * 3.0f * -side;
	mov += dz * 3.0f * RVector::AXISZ;

	pCapsule->top += mov;
	pCapsule->CalcHeight();

	GlobalObjects::g_SaveNotifier.SetSaveMeshInfo(true);
}

void CMainApplication::DrawHitEffectPosDir()
{
	GlobalObjects::g_pMainForm->m_TalentEffectTabController->RenderHitEffectPosDir();
}

void CMainApplication::SetMeshInfo( System::String^ ModelName )
{
	GlobalObjects::g_pMainForm->m_MeshInfoTabPage->SetMeshInfo(ModelName);
}

void CMainApplication::GetAnimationList( System::Collections::ArrayList^ arrayAnimationList )
{
	vector<string> vecAnimationList;
	GetAnimationList(vecAnimationList);

	for(vector<string>::iterator it = vecAnimationList.begin(); it != vecAnimationList.end(); ++it)
	{
		String^ Name = gcnew String(it->c_str());
		arrayAnimationList->Add(Name);
	}
}

void CMainApplication::GetAnimationList( vector<string>& vecAnimationList )
{
	if( g_pMainApp->GetActor() == NULL || g_pMainApp->GetActor()->m_pMesh == NULL)
		return;

	RMeshAnimationMgr* pAMgr = &g_pMainApp->GetActor()->m_pMesh->m_AniMgr;

	vector<string> strErrorMessage;

	for(RAnimationHashList_Iter it = pAMgr->m_NodeHashList.begin(); it != pAMgr->m_NodeHashList.end();it++)
	{
		RAnimation* pAnimation = *it;
		if(pAnimation)
		{
			string strAniName = pAnimation->GetAliasName();

			//파일이 존재하는냐?
			RAnimation* pAnimation = g_pMainApp->m_TabAnimation.GetAnimation(strAniName.c_str());
			if( pAnimation == NULL
				|| pAnimation->GetID() < 0
				|| pAnimation->GetResource() == NULL
				|| !ExistFile(pAnimation->GetResource()->GetName().c_str()))
			{
				continue;
			}

			vecAnimationList.push_back(strAniName);
		}
	}
}

void CMainApplication::SelectWeaponEffect()
{
	if(m_pEffectManager == NULL)
		return;

	if(CheckWeaponEffect() == false)
		return;

	EffectSelectForm dialogEffectSelectForm;

	if(m_nWeaponEffectBuffID > 0)
	{
		dialogEffectSelectForm.SetEffectBuffID(m_nWeaponEffectBuffID);
		dialogEffectSelectForm.SetEffectEquipType(gcnew String(m_strEnchantBuffEquipType.c_str()));
	}
	else
	{
		for(int j = 0; j < WEAPON_EFFECT_DUMMY_MAX_COUNT_TOOL; ++j)
		{
			dialogEffectSelectForm.SetEffectName(gcnew String(m_strWeaponEffectName[j].c_str()), j);
		}
	}

	dialogEffectSelectForm.ShowDialog();
	System::Windows::Forms::DialogResult res = dialogEffectSelectForm.DialogResult;
	if (res == System::Windows::Forms::DialogResult::OK)
	{
		DeleteWeaponEffect();

		m_nWeaponEffectBuffID = dialogEffectSelectForm.GetSelectEnchantBuffEffectID();
		m_strEnchantBuffEquipType = MStringToCharPointer(dialogEffectSelectForm.GetSelectEnchantEquipType());
		m_nEnchantBuffEquipType = EnchantEquipType2PlayerPartsSlotType(m_strEnchantBuffEquipType);
		if(m_nWeaponEffectBuffID == 0)
		{
			for(int i = 0; i < WEAPON_EFFECT_DUMMY_MAX_COUNT_TOOL; ++i)
			{
				String^ strSelectEffectName = dialogEffectSelectForm.GetSelectEffectName(i);
				m_strWeaponEffectName[i] = MStringToCharPointer(strSelectEffectName);
				m_pEffectManager->SetWeaponEffect(GetActor(), m_strWeaponEffectName[i], i);
			}
		}
		else
		{
			XBuffInfo* pBuffInfo = m_BuffInfo.Get(m_nWeaponEffectBuffID);
			if(pBuffInfo == NULL)
				return;

			WEAPON_TYPE eWeaponType = EnchantEquipType2WeaponType(m_strEnchantBuffEquipType);
			vector<XEnchantBuffEffectInfo *> vecBuffEffectInfo;
			pBuffInfo->m_EnchantBuffEffect.GetEffectInfoList(EBET_DUMMY_EFFECT, eWeaponType, eWeaponType, m_nEnchantBuffEquipType, vecBuffEffectInfo);

			for(vector<XEnchantBuffEffectInfo *>::iterator itBuff = vecBuffEffectInfo.begin(); itBuff != vecBuffEffectInfo.end(); ++itBuff)
			{
				TEffectCharacterToolWeaponInvoker effectInvoker;

				if((*itBuff)->m_strBoneName == "")
				{
					for(int j = 0; j < WEAPON_EFFECT_DUMMY_MAX_COUNT_TOOL; ++j)
					{
						char chEffectName[256] = {0,};
						sprintf_s(chEffectName, "%s%02d", WEAPON_EFFECT_DUMMY_NAME_TOOL, j);

						effectInvoker.Invoke(GetActor(), (*itBuff)->m_strEffectName, string(chEffectName));
					}
				}
				else
					effectInvoker.Invoke(GetActor(), (*itBuff)->m_strEffectName, (*itBuff)->m_strBoneName);
			}
		}
	}
}

void CMainApplication::DeleteWeaponEffect()
{
	if(m_pEffectManager == NULL)
		return;

	for(int i = 0; i < WEAPON_EFFECT_DUMMY_MAX_COUNT_TOOL; ++i)
	{
		m_strWeaponEffectName[i].clear();
	}

	m_pEffectManager->StopWepaonEffect();

}

bool CMainApplication::CheckWeaponEffect()
{
	// 무기 이펙트를 보여줄수 있는지 체크
	GetActor();

	if(GetActor() == NULL ||
		(m_pActorMgr->GetFemalePlayerModel() != GetActor() && m_pActorMgr->GetMalePlayerModel() != GetActor()))
	{
		System::Windows::Forms::MessageBox::Show(L"먼저 플레이어 모델을 열어야 합니다.", "에러!");
		return false;
	}

	return true;
}

int CMainApplication::EnchantEquipType2PlayerPartsSlotType( string strType )
{
	if(strType == "1hs" ||
		strType == "1hb" ||
		strType == "2hd" ||
		strType == "sta" ||
		strType == "arc" ||
		strType == "2hb" ||
		strType == "dwd" ||
		strType == "dwp")
		return PLAYER_PARTS_SLOT_RWEAPON;
	else if(strType == "head")
		return PLAYER_PARTS_SLOT_HEAD;
	else if(strType == "body")
		return PLAYER_PARTS_SLOT_BODY;
	else if(strType == "hands")
		return PLAYER_PARTS_SLOT_HANDS;
	else if(strType == "leg")
		return PLAYER_PARTS_SLOT_LEG;
	else if(strType == "feet")
		return PLAYER_PARTS_SLOT_FEET;

	return -1;
}

WEAPON_TYPE CMainApplication::EnchantEquipType2WeaponType( string strType )
{
	if(strType == "1hs")
		return WEAPON_1H_SLASH;
	else if(strType == "1hb")
		return WEAPON_1H_BLUNT;
	else if(strType == "1hp")
		return WEAPON_1H_PIERCE;
	else if(strType == "2hd")
		return WEAPON_TWO_HANDED;
	else if(strType == "sta")
		return WEAPON_STAFF;
	else if(strType == "arc")
		return WEAPON_ARCHERY;
	else if(strType == "2hb")
		return WEAPON_2H_BLUNT;
	else if(strType == "dwd")
		return WEAPON_DUAL_WIELD;
	else if(strType == "dwp")
		return WEAPON_DUAL_PIERCE;

	return WEAPON_NONE;
}

void CMainApplication::OvelayAnimationEditTool()
{
	if(GetActor() == NULL || GetActor()->m_pMesh == NULL)
	{
		// 모델이 필요하다.
		if (System::Windows::Forms::MessageBox::Show("모델을 선택해 주세요. ", "에러",MessageBoxButtons::OK) == System::Windows::Forms::DialogResult::OK)
			return;
	}

	OverlayAnimationTable overlayAnimation_dialog;
	overlayAnimation_dialog.ShowDialog();
	System::Windows::Forms::DialogResult res = overlayAnimation_dialog.DialogResult;
	if (res == System::Windows::Forms::DialogResult::OK)
	{

	}
}

void CMainApplication::AnimationBlendEditTool()
{
	if( GetActor() == NULL || GetActor()->m_pMesh == NULL)
	{
		// 모델이 필요하다.
		if (System::Windows::Forms::MessageBox::Show("모델을 선택해 주세요. ", "에러",MessageBoxButtons::OK) == System::Windows::Forms::DialogResult::OK)
			return;
	}

	AnimationBlendTable animationBlend_dialog;
	animationBlend_dialog.ShowDialog();
	System::Windows::Forms::DialogResult res = animationBlend_dialog.DialogResult;
	if (res == System::Windows::Forms::DialogResult::OK)
	{

	}
}

void CMainApplication::ReActionAnimationEditTool()
{
	if( GetActor() == NULL || GetActor()->m_pMesh == NULL)
	{
		// 모델이 필요하다.
		if (System::Windows::Forms::MessageBox::Show("모델을 선택해 주세요. ", "에러",MessageBoxButtons::OK) == System::Windows::Forms::DialogResult::OK)
			return;
	}

	GlobalObjects::g_pMainForm->ShowReactionAnimationTable();
}

void CMainApplication::ReActionAnimationTest()
{
	if(GetActor() == NULL)
		return;

	// 무기별 증가율
	float fWeightRate = 1.0f;
	float fSpeedRate = 1.0f;

	WEAPON_TYPE nAttackWeaponType = m_pActor->GetCurrentWeaponType();

	if(nAttackWeaponType != WEAPON_NONE)
	{
		fWeightRate = TCONST::HIT_FAKE_BEATEN_WEAPON_TYPE_BLEND_WEIGHT[nAttackWeaponType - 1];
		fSpeedRate = TCONST::HIT_FAKE_BEATEN_WEAPON_TYPE_BLEND_SPEED[nAttackWeaponType - 1];
	}

	TNPCInfo* pNpcInfo = m_NPC.GetNPCInfo();
	if (pNpcInfo)
	{
		RAnimation* pCurAnimation = m_pActor->GetCurAnimation();
		if(pCurAnimation)
			m_pActor->StartFakeBeaten(pCurAnimation->GetAliasName(), fWeightRate, fSpeedRate);
	}
	else
	{
		RAnimation* pCurAnimation = m_pActor->GetCurAnimation();
		if(pCurAnimation)
		{
			CHAR_STANCE nStance = CS_NORMAL;
			if(nAttackWeaponType > 0)
				nStance = CS_BATTLE;

			string strAniPrefix = TNaming::PlayerAnimationNameHeader(nAttackWeaponType, nStance);
			if(nStance == CS_NORMAL)
			{
				string strAnimationName = pCurAnimation->GetAliasName();
				strAniPrefix = GetRemoveUnderBarlast(strAnimationName);

				//검수
				if(EnchantEquipType2WeaponType(strAniPrefix) == WEAPON_NONE)
					strAniPrefix = "";
			}

			m_pActor->StartFakeBeaten(pCurAnimation->GetAliasName(), fWeightRate, fSpeedRate, strAniPrefix.c_str());
		}
	}
}

void CMainApplication::ReActionAnimationNomalTest()
{
	tstring strBeatenAniName = TCONST::HIT_FAKE_BEATEN_DEFAULT_ANI_NAME;
	GetActor()->InitFakeBeatenAnimation(strBeatenAniName, TCONST::HIT_FAKE_BEATEN_ANIMATION_BLEND_WEIGHT, TCONST::HIT_FAKE_BEATEN_ANIMATION_BLEND_SPEED);

	ReActionAnimationTest();
}

void CMainApplication::ReActionAnimationBossTest()
{
	// 페이크 비튼 설정
	tstring strBeatenAniName = TCONST::HIT_FAKE_BEATEN_DEFAULT_ANI_NAME;
	GetActor()->InitFakeBeatenAnimation(strBeatenAniName, TCONST::HIT_HUGE_FAKE_BEATEN_ANIMATION_BLEND_WEIGHT, TCONST::HIT_HUGE_FAKE_BEATEN_ANIMATION_BLEND_SPEED);

	ReActionAnimationTest();
}

void CMainApplication::UpdateModelScale()
{
	if(GetActor() == NULL)
		return;

	if(m_SelectedTab == TAB_MTRLSTATE_NPC)
	{
		m_NPC.SetNPCScale();
		return;
	}

	GetActor()->SetScale(1.0f);
}

void CMainApplication::SetNPCAnimationInfo( String^ strNpcName /*= nullptr*/ )
{
	NPCAnimationInfoSetting npcInfoForm;

	npcInfoForm.SetSelectNpcName(strNpcName);
	npcInfoForm.Init(m_pActorMgr->GetFemalePlayerModel(), m_pActorMgr->GetMalePlayerModel());
	npcInfoForm.ShowDialog();
	System::Windows::Forms::DialogResult res = npcInfoForm.DialogResult;
	if (res == System::Windows::Forms::DialogResult::OK)
	{

	}
}

bool CMainApplication::SkinColorTest()
{
	return m_TabModelInfo.ReadySkinColorTest();
}

bool CMainApplication::HairColorTest()
{
	return m_TabModelInfo.ReadyHairColorTest();
}

bool CMainApplication::ItemColorTest()
{
	return m_TabModelInfo.ReadyItemColorTest();
}

void CMainApplication::RenderAxisText( RMatrix mat, float fSize )
{
	rs3::RFont::BeginFont();

	g_pMainApp->GetDevice()->SetTransform(RST_VIEW, g_pMainApp->GetCamera()->GetViewMatrix());

	rs3::RMatrix matInverse;
	g_pMainApp->GetDevice()->GetTransform( rs3::RST_VIEW ).GetInverse( &matInverse );
	g_pMainApp->GetCamera()->GetViewMatrix().GetInverse( &matInverse );

	rs3::RMatrix matTextReflectAndScale
		(
		0.5f,    0.f,    0.f,   0.f,
		0.f,     -0.5f,  0.f,   0.f,
		0.f,      0.f,    0.5f, 0.f,
		0.f,      0.f,    0.f,   1.f
		);

	vec3 vTempPos = vec3(fSize + 7, 0, 0);
	mat.TransformVect(vTempPos);
	matInverse.SetTranslation(vTempPos);
	RMatrix matT = matTextReflectAndScale * matInverse;
	g_pMainApp->GetFont()->DrawTextIn3DSpace(matT, L"X", -1, AXIS_X_COLOR);

	vTempPos = vec3(0, fSize + 7, 0);
	mat.TransformVect(vTempPos);
	matInverse.SetTranslation(vTempPos);
	matT = matTextReflectAndScale * matInverse;
	g_pMainApp->GetFont()->DrawTextIn3DSpace(matT, L"Y", -1, AXIS_Y_COLOR);

	vTempPos = vec3(0, 0, fSize + 15);
	mat.TransformVect(vTempPos);
	matInverse.SetTranslation(vTempPos);
	matT = matTextReflectAndScale * matInverse;
	g_pMainApp->GetFont()->DrawTextIn3DSpace(matT, L"Z", -1, AXIS_Z_COLOR);

	rs3::RFont::EndFont();
}

String^ CMainApplication::GetSearchCustomEffectInfoName()
{
	SearchCustomEffectInfoName searchEffectInfoName;
	searchEffectInfoName.Init();
	searchEffectInfoName.ShowDialog();
	return searchEffectInfoName.m_strSelectCustoEffectInfoName;
}

int CMainApplication::CreateMeshColBoxInfo()
{
	int nBoxID = TMeshInfoMgr::GetInstance().GetMeshColInfo()->CreateBox(m_strModelFullPathName, m_pActor);

	if(nBoxID == -1)
	{
		System::Windows::Forms::MessageBox::Show(String::Format("{0}에 대한 메쉬 정보가 없습니다.", gcnew String(TMeshInfoMgr::GetInstance().GetMeshHitInfo()->GetModelName(m_strModelFullPathName).c_str())), "충돌 박스 생성 에러");
		return -1;
	}

	m_MeshColInfoViewControl.CreateMeshColBoxInfoViewer(nBoxID);
	m_MeshColInfoViewControl.NotifyChanging();
	return nBoxID;
}

void CMainApplication::DelMeshColBoxInfo( int nBoxID )
{
	TMeshInfoMgr::GetInstance().GetMeshColInfo()->DelBox(m_strModelFullPathName, nBoxID);
	m_MeshColInfoViewControl.Reload();
	m_MeshColInfoViewControl.NotifyChanging();
}

void CMainApplication::OnMouseDown( int stat, RVector2 pos, int delta )
{
	m_MeshColInfoViewControl.CheckMouseDown(stat, pos, pos);
}

void CMainApplication::UpdateShowNPCHitCapsuleGroup( bool bShow, CSTalentInfo* pTalentInfo /*= NULL*/ )
{
	GlobalObjects::g_SelInfo.m_nMeshHitGroupID = -1;
	GlobalObjects::g_SelInfo.m_nMeshHitCapsuleID = -1;

	if(bShow)
	{
		CSTalentInfo* pTempTalentInfo = NULL;
		if(pTalentInfo)
			pTempTalentInfo = pTalentInfo;
		else if(m_pTalentInfo)
			pTempTalentInfo = m_pTalentInfo;
		else
			return;

		GlobalObjects::g_SelInfo.m_nMeshHitGroupID = pTempTalentInfo->m_nHitCapsuleGroupIndex;
		GlobalObjects::g_SelInfo.m_nMeshHitCapsuleID = -1;
	}
}

void CMainApplication::StartTalentPosInfoExporter()
{
	float fTalentInfoCount = m_TTalentInfoMgr.size();
	float fCount = 0;
	for(map<int , CSTalentInfo* >::iterator it = m_TTalentInfoMgr.begin(); it != m_TTalentInfoMgr.end(); ++it)
	{
		TalentPosInfoRefresh(it->second);

		++fCount;

		float fRatio = (fCount / fTalentInfoCount) * 100.0f;

		if(fRatio > 100)
			fRatio = 100;
		SetProgressBarValue((int)fRatio);
	}
}
