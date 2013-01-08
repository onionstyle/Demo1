#pragma once
#include "DirectHelper.h"
//rbga通道索引值
#define MT_ALPHA 3
#define MT_RED   2
#define MT_GREEN  1
#define MT_BLUE 0
using namespace Platform;
namespace Demo1
{
    public ref class Gradient sealed
    {
	public:
		/// <summary>
		/// 初始化类，在所有其它函数之前调用
		/// </summary>
		void Initialize(const Array<byte>^ pImageData,int nWidth,int nHeight);

		/// <summary>
		/// 释放类资源
		/// </summary>
		void DiscardDeviceResources();
		/// <summary>
		/// 圆形虚化
		/// </summary>	
		Array<byte>^ RadiusGradient(float centX,float centY,float insideRadius,float outsideRadius);

		/// <summary>
		/// 线性虚化
		/// </summary>	
		Array<byte>^ LinearGradient(float centX,float centY,float rate,float posX2,float posY2);

		Array<byte>^ RadiusApply(const Array<byte>^ pImageData,int nWidth,int nHeight,float centX,float centY,float insideRadius,float outsideRadius);

		Array<byte>^ LinearApply(const Array<byte>^ pImageData,int nWidth,int nHeight,float centX,float centY,float rate,float posX2,float posY2);

	public:
		Gradient(void);
		virtual ~Gradient(void);
	private:
	
		HRESULT CreateDeviceResouces(const Array<byte>^ pImageData);
		
		//初始化蒙版图层图
		void  InitializeMask(const Array<byte>^ pData);

		bool GrayData(const Array<unsigned char>^ bitData ,int w, int h);
	private:
		ID2D1Factory1*	m_pD2d1Factory;
		IWICImagingFactory2*	m_pWicFractory;
		ID2D1RenderTarget* m_pRenderTarget;
		//被绘制的IWIC
		IWICBitmap* m_pMainBimtap;
		//初始传入图片画刷
		ID2D1BitmapBrush* m_pD2d1BitmapBrush;
		//蒙版画刷
		ID2D1BitmapBrush* m_pD2d1MaskBrush;
		ID2D1RectangleGeometry *m_pRectGeo;    
		int m_nWidth;
		int m_nHeight;
		const Array<byte>^ m_pImageData;


		ID2D1RenderTarget* t_pRenderTarget;
		IWICBitmap* t_pMainBimtap;
		ID2D1BitmapBrush* t_pD2d1BitmapBrush;
		ID2D1BitmapBrush* t_pD2d1MaskBrush;
		ID2D1RectangleGeometry *t_pRectGeo;    
    };
}