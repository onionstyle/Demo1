// Class1.cpp
#include "pch.h"
#include "Gradient.h"

using namespace Demo1;
Gradient::Gradient(void):
	m_pWicFractory(nullptr),
	m_pD2d1Factory(nullptr),
	m_pMainBimtap(nullptr),
	m_pRenderTarget(nullptr),
	m_pD2d1BitmapBrush(nullptr)
{


}

Gradient::~Gradient(void)
{

}

void Gradient::DiscardDeviceResources()
{
	DX::SafeRelease(&m_pD2d1Factory);
	DX::SafeRelease(&m_pWicFractory);
	DX::SafeRelease(&m_pMainBimtap);
	DX::SafeRelease(&m_pRenderTarget);
	DX::SafeRelease(&m_pD2d1BitmapBrush);
}

HRESULT Gradient::CreateDeviceResouces(const Array<byte>^ pImageData)
{
	HRESULT hr = S_OK;

	m_pWicFractory->CreateBitmap(m_nWidth,m_nHeight,GUID_WICPixelFormat32bppPBGRA,WICBitmapCacheOnDemand,&m_pMainBimtap);
	if (SUCCEEDED(hr))
	{
	hr = m_pD2d1Factory->CreateWicBitmapRenderTarget(
		m_pMainBimtap,
		D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_SOFTWARE),
		&m_pRenderTarget
		);
	}
	//创建图片画刷
	ID2D1Bitmap* pTemp = nullptr;
	if (SUCCEEDED(hr))
	{
		hr =m_pRenderTarget->CreateBitmap(D2D1::SizeU(m_nWidth,m_nHeight),pImageData->Data,m_nWidth*4,
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_IGNORE)),&pTemp);
	}
	hr =m_pRenderTarget->CreateBitmapBrush(pTemp,&m_pD2d1BitmapBrush);
	//灰度处理
	GrayData(pImageData,m_nWidth,m_nHeight);

	InitializeMask(pImageData);

	//创建蒙版矩形
	if (SUCCEEDED(hr))
	{
		hr = m_pD2d1Factory->CreateRectangleGeometry(
			D2D1::RectF(0,0,m_nWidth,m_nHeight),
			&m_pRectGeo
			);
	}

	return hr;
}

void Gradient::Initialize(const Array<byte>^ pImageData,int nWidth,int nHeight)
{
	m_nWidth=nWidth;
	m_nHeight=nHeight;
	m_pImageData = ref new Array<byte>(pImageData->Length);
	memcpy(m_pImageData->Data,pImageData->Data,pImageData->Length);

	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1),&options, (void**)&m_pD2d1Factory);

	CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&m_pWicFractory));

	CreateDeviceResouces(pImageData);
}

Array<byte>^ Gradient::RadiusGradient(float centX,float centY,float insideRadius,float outsideRadius)
{
	 HRESULT hr;
	 ID2D1GradientStopCollection *pGradientStops = NULL;
	 //内圆半径占外园半径的比例
	 float rate =insideRadius/outsideRadius;
	 const D2D1_GRADIENT_STOP gradientStops[] =
	 {
		 {   0.f,  D2D1::ColorF(D2D1::ColorF::Black, 0.0f)  },
		 {   rate,  D2D1::ColorF(D2D1::ColorF::Black, 0.5f)  },
		 {   1.0f,  D2D1::ColorF(D2D1::ColorF::Black, 1.0f)  },
	 };
	 m_pRenderTarget->BeginDraw();
 	 m_pRenderTarget->Clear( D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
	 hr = m_pRenderTarget->CreateGradientStopCollection(
		 gradientStops,
		 3,
		 D2D1_GAMMA_1_0,
		 D2D1_EXTEND_MODE_CLAMP,
		 &pGradientStops);
	 //渐变画笔
	ID2D1RadialGradientBrush* pRadiusBrush=nullptr;
	hr = m_pRenderTarget->CreateRadialGradientBrush(
		D2D1::RadialGradientBrushProperties(
		D2D1::Point2F(centX,centY),
		D2D1::Point2F(0, 0),
		outsideRadius,
		outsideRadius),
		pGradientStops,
		&pRadiusBrush);
	D2D1_RECT_F rcBrushRect = D2D1::RectF(0, 0, m_nWidth,m_nHeight);
	// 填充渐变
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0,0));
	m_pRenderTarget->FillRectangle(rcBrushRect, m_pD2d1BitmapBrush);
	m_pRenderTarget->FillGeometry(
		m_pRectGeo, 
		m_pD2d1MaskBrush, 
		pRadiusBrush
		);
	m_pRenderTarget->EndDraw();
	DX::SafeRelease(&pRadiusBrush);


	IWICBitmapLock* pLock = nullptr;
	byte* Com_1 = nullptr;
	WICRect rcLock = { 0, 0, m_nWidth, m_nHeight };
	m_pMainBimtap->Lock(&rcLock,WICBitmapLockRead,&pLock);

	UINT cbBufferSize = 0;
	pLock->GetDataPointer(&cbBufferSize,&Com_1);

	Array<byte>^ ary = ref new Array<byte>(cbBufferSize);
	memcpy(ary->Data,Com_1,cbBufferSize);
	
	pLock->Release();

	return ary;
}

Array<byte>^ Gradient::LinearGradient(float centX,float centY,float rate,float posX,float posY)
{

	HRESULT hr;
	ID2D1GradientStopCollection *pGradientStops = NULL;
	const D2D1_GRADIENT_STOP gradientStops[] =
	{
		{   0.f,  D2D1::ColorF(D2D1::ColorF::Black, 0.0f)  },
		{   rate,  D2D1::ColorF(D2D1::ColorF::Black, 0.5f)  },
		{   1.0f,  D2D1::ColorF(D2D1::ColorF::Black, 1.0f)  },
	};
	m_pRenderTarget->BeginDraw();
	m_pRenderTarget->Clear( D2D1::ColorF(D2D1::ColorF::White, 0.0f));
	hr = m_pRenderTarget->CreateGradientStopCollection(
		gradientStops,
		3,
		D2D1_GAMMA_1_0,
		D2D1_EXTEND_MODE_CLAMP,
		&pGradientStops);

	//渐变画笔
	ID2D1LinearGradientBrush *brush1;
	ID2D1LinearGradientBrush *brush2;
	hr = m_pRenderTarget->CreateLinearGradientBrush(
		D2D1::LinearGradientBrushProperties(
		D2D1::Point2F(centX,centY),
		D2D1::Point2F(posX,posY)),
		pGradientStops,
		&brush1);
	hr = m_pRenderTarget->CreateLinearGradientBrush(
		D2D1::LinearGradientBrushProperties(
		D2D1::Point2F(centX,centY),
		D2D1::Point2F(centX+centX-posX,centY+centY-posY)),
		pGradientStops,
		&brush2);

	D2D1_RECT_F rcBrushRect = D2D1::RectF(0, 0, m_nWidth,m_nHeight);
	// 填充渐变
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0,0));
	m_pRenderTarget->FillRectangle(rcBrushRect, m_pD2d1BitmapBrush);
	m_pRenderTarget->FillGeometry(
		m_pRectGeo, 
		m_pD2d1MaskBrush, 
		brush1
		);	
	m_pRenderTarget->FillGeometry(
		m_pRectGeo, 
		m_pD2d1MaskBrush, 
		brush2
		);
	m_pRenderTarget->EndDraw();
	
	DX::SafeRelease(&brush1);
	DX::SafeRelease(&brush2);


	IWICBitmapLock* pLock = nullptr;
	byte* Com_1 = nullptr;
	WICRect rcLock = { 0, 0, m_nWidth, m_nHeight };
	m_pMainBimtap->Lock(&rcLock,WICBitmapLockRead,&pLock);

	UINT cbBufferSize = 0;
	pLock->GetDataPointer(&cbBufferSize,&Com_1);

	Array<byte>^ ary = ref new Array<byte>(cbBufferSize);
	memcpy(ary->Data,Com_1,cbBufferSize);

	pLock->Release();

	return ary;
}

void  Gradient::InitializeMask(const Array<byte>^ pData)
{
	HRESULT hr = S_OK;
	//创建图片画刷
	ID2D1Bitmap* pTemp = nullptr;
	hr =m_pRenderTarget->CreateBitmap(D2D1::SizeU(m_nWidth,m_nHeight),pData->Data,m_nWidth*4,
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_IGNORE)),&pTemp);
	hr =m_pRenderTarget->CreateBitmapBrush(pTemp,&m_pD2d1MaskBrush);
}

bool Gradient::GrayData(const Array<unsigned char>^ bitData ,int w, int h)
{
	unsigned char* pBitData = bitData->Data;
	if(pBitData == NULL)
	{
		return false;
	}
	int sum = w*h;
#pragma omp parallel for 
	for (int i = 0; i < sum; i++)
	{
		unsigned char *tmpData = pBitData + (i<<2);
		tmpData[0] = tmpData[1] = tmpData[2] = (tmpData[0] * 19595 + tmpData[1] * 38469 + tmpData[2] * 7472) >> 16;
	}
	return true;
}


Array<byte>^ Gradient::RadiusApply(const Array<byte>^ pImageData,int nWidth,int nHeight,float centX,float centY,float insideRadius,float outsideRadius)
{
	//缓存缩略图设备地址
	t_pRenderTarget= m_pRenderTarget;
	t_pMainBimtap=m_pMainBimtap;
	t_pD2d1BitmapBrush=m_pD2d1BitmapBrush;
	t_pD2d1MaskBrush=m_pD2d1MaskBrush;
	t_pRectGeo=m_pRectGeo;    

	m_pRenderTarget=nullptr ;
	m_pMainBimtap=nullptr;
	m_pD2d1BitmapBrush=nullptr;
	m_pD2d1MaskBrush=nullptr;
	m_pRectGeo=nullptr;  

	int t_nWidth=m_nWidth;
	int t_nHeight=m_nHeight;
	m_nWidth=nWidth;
	m_nHeight=nHeight;
	//创建主图设备
	CreateDeviceResouces(pImageData);
	Array<byte>^  pData= RadiusGradient(centX,centY,insideRadius,outsideRadius);

	m_nWidth=t_nWidth;
	m_nHeight=t_nHeight;

	DX::SafeRelease(&m_pRenderTarget);
	DX::SafeRelease(&m_pMainBimtap);
	DX::SafeRelease(&m_pD2d1BitmapBrush);
	DX::SafeRelease(&m_pD2d1MaskBrush);
	DX::SafeRelease(&m_pRectGeo);
	//恢复缩略图设备地址
	m_pRenderTarget=t_pRenderTarget ;
	m_pMainBimtap=t_pMainBimtap;
	m_pD2d1BitmapBrush=t_pD2d1BitmapBrush;
	m_pD2d1MaskBrush=t_pD2d1MaskBrush;
	m_pRectGeo=t_pRectGeo;  
	
	return pData;
}

Array<byte>^ Gradient::LinearApply(const Array<byte>^ pImageData,int nWidth,int nHeight,float centX,float centY,float rate,float posX2,float posY2)
{
	//缓存缩略图设备地址
	t_pRenderTarget= m_pRenderTarget;
	t_pMainBimtap=m_pMainBimtap;
	t_pD2d1BitmapBrush=m_pD2d1BitmapBrush;
	t_pD2d1MaskBrush=m_pD2d1MaskBrush;
	t_pRectGeo=m_pRectGeo;    

	m_pRenderTarget=nullptr ;
	m_pMainBimtap=nullptr;
	m_pD2d1BitmapBrush=nullptr;
	m_pD2d1MaskBrush=nullptr;
	m_pRectGeo=nullptr;  
	int t_nWidth=m_nWidth;
	int t_nHeight=m_nHeight;
	m_nWidth=nWidth;
	m_nHeight=nHeight;
	//创建主图设备
	CreateDeviceResouces(pImageData);
	Array<byte>^  pData= LinearGradient(centX,centY,rate,posX2,posY2);
	
	m_nWidth=t_nWidth;
	m_nHeight=t_nHeight;
	DX::SafeRelease(&m_pRenderTarget);
	DX::SafeRelease(&m_pMainBimtap);
	DX::SafeRelease(&m_pD2d1BitmapBrush);
	DX::SafeRelease(&m_pD2d1MaskBrush);
	DX::SafeRelease(&m_pRectGeo);

	//恢复缩略图设备地址
	m_pRenderTarget=t_pRenderTarget ;
	m_pMainBimtap=t_pMainBimtap;
	m_pD2d1BitmapBrush=t_pD2d1BitmapBrush;
	m_pD2d1MaskBrush=t_pD2d1MaskBrush;
	m_pRectGeo=t_pRectGeo;  

	return pData;
}

