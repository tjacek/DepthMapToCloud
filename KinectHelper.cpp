#include "KinectHelper.h"


KinectHelper::KinectHelper():
	m_pDepthStreamHandle(INVALID_HANDLE_VALUE),
	m_pNuiSensor(NULL)
{
	m_depthRGBX = new BYTE[cDepthWidth*cDepthHeight*cBytesPerPixel];
	depthImage.create(this->cDepthWidth,this->cDepthHeight,CV_16U);
}

KinectHelper::~KinectHelper(){
	if (m_pNuiSensor)
    {
        m_pNuiSensor->NuiShutdown();
    }
	delete[] m_depthRGBX;
}


void KinectHelper::ProcessDepth( ImageRenderer*          m_pDrawDepth)
{
    HRESULT hr;
    NUI_IMAGE_FRAME imageFrame;

    // Attempt to get the depth frame
    hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);
    if (FAILED(hr))
    {
        return;
    }

    BOOL nearMode;
    INuiFrameTexture* pTexture;

    // Get the depth image pixel texture
    hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
        m_pDepthStreamHandle, &imageFrame, &nearMode, &pTexture);
    if (FAILED(hr))
    {
        goto ReleaseFrame;
    }

    NUI_LOCKED_RECT LockedRect;

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    pTexture->LockRect(0, &LockedRect, NULL, 0);

    // Make sure we've received valid data
    if (LockedRect.Pitch != 0)
    {
        const NUI_DEPTH_IMAGE_PIXEL * pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits);
		//BYTE * rgbrun = getBytes(pBufferRun,nearMode);

        //Draw the data with Direct2D
        //m_pDrawDepth->Draw(m_depthRGBX, cDepthWidth * cDepthHeight * cBytesPerPixel);

       getMat(pBufferRun);
    }

    // We're done with the texture so unlock it
    pTexture->UnlockRect(0);

    pTexture->Release();

ReleaseFrame:
    // Release the frame
    m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);
}

void KinectHelper::getMat(const NUI_DEPTH_IMAGE_PIXEL * pBufferRun){
	const NUI_DEPTH_IMAGE_PIXEL * pBufferEnd = pBufferRun + (cDepthWidth * cDepthHeight);
	int i=0;
	while ( pBufferRun < pBufferEnd )
    {
		int x=i % this->cDepthWidth;
		int y=(i-x) / this->cDepthWidth;
        // discard the portion of the depth that contains only the player index
        USHORT depth = pBufferRun->depth;
		ushort* p = depthImage.ptr<ushort>(y);
		p[x]=depth;
			//this->depthImage.data[i]=depth;		 
		pBufferRun+=4;
		i++;
	}
}

BYTE * KinectHelper::getBytes(const NUI_DEPTH_IMAGE_PIXEL * pBufferRun,BOOL nearMode){
  BYTE * rgbrun = m_depthRGBX;
  int minDepth = (nearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
  int maxDepth = (nearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

  const NUI_DEPTH_IMAGE_PIXEL * pBufferEnd = pBufferRun + (cDepthWidth * cDepthHeight);
    while ( pBufferRun < pBufferEnd )
        {
            // discard the portion of the depth that contains only the player index
            USHORT depth = pBufferRun->depth;

            // To convert to a byte, we're discarding the most-significant
            // rather than least-significant bits.
            // We're preserving detail, although the intensity will "wrap."
            // Values outside the reliable depth range are mapped to 0 (black).

            // Note: Using conditionals in this loop could degrade performance.
            // Consider using a lookup table instead when writing production code.
            BYTE intensity = static_cast<BYTE>(depth >= minDepth && depth <= maxDepth ? depth % 256 : 0);

            // Write out blue byte
            *(rgbrun++) = intensity;

            // Write out green byte
            *(rgbrun++) = intensity;

            // Write out red byte
            *(rgbrun++) = intensity;

            // We're outputting BGR, the last byte in the 32 bits is unused so skip it
            // If we were outputting BGRA, we would write alpha here.
            ++rgbrun;

            // Increment our index into the Kinect's depth buffer
            ++pBufferRun;
        }
  return rgbrun;
}