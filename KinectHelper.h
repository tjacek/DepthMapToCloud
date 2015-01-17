
#pragma warning(push)
#pragma warning(disable : 6294 6031)
#include <opencv2/core/core.hpp>
#pragma warning(pop)
#include "windows.h"
#include "NuiApi.h"
#include "ImageRenderer.h"


class KinectHelper{
  public:
	static const int        cDepthWidth  = 640;
    static const int        cDepthHeight = 480;
    static const int        cBytesPerPixel = 4;

	INuiSensor*             m_pNuiSensor;
	HANDLE                  m_pDepthStreamHandle;
	BYTE*                   m_depthRGBX;

	KinectHelper();
	~KinectHelper();
	void ProcessDepth( ImageRenderer* m_pDrawDepth);

    void getMat();
	BYTE * getBytes(const NUI_DEPTH_IMAGE_PIXEL * pBufferRun,BOOL nearMode);
};