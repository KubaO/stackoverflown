#include "ASICamera2.h"
#include <QtGui>

struct Cam {
   ASI_CAMERA_INFO info;
   bool open = false;
   bool initialized = false;
   bool capturing = false;
   QImage frame;
   QElapsedTimer timer;
   qint64 exposure_ms = 50;
   void newFrame();
};

static Cam cam;

int ASIGetNumOfConnectedCameras() { return 1; }

ASI_ERROR_CODE ASIGetCameraProperty(ASI_CAMERA_INFO *pASICameraInfo, int iCameraIndex) {
   if (iCameraIndex != 0)
      return ASI_ERROR_INVALID_INDEX;
   *pASICameraInfo = cam.info;
   return ASI_SUCCESS;
}

ASI_ERROR_CODE ASIOpenCamera(int iCameraID) {
   if (iCameraID != cam.info.CameraID)
      return ASI_ERROR_INVALID_ID;
   cam.open = true;
   return ASI_SUCCESS;
}

ASI_ERROR_CODE ASICloseCamera(int iCameraID) {
   if (iCameraID != cam.info.CameraID)
      return ASI_ERROR_INVALID_ID;
   cam.open = false;
   cam.initialized = false;
   return ASI_SUCCESS;
}

ASI_ERROR_CODE ASIInitCamera(int iCameraID) {
   if (iCameraID != cam.info.CameraID)
      return ASI_ERROR_INVALID_ID;
   if (!cam.open)
      return ASI_ERROR_CAMERA_CLOSED;
   QThread::msleep(200);
   cam.initialized = true;
   return ASI_SUCCESS;
}

ASI_ERROR_CODE ASISetROIFormat(int iCameraID, int iWidth, int iHeight,  int iBin, ASI_IMG_TYPE Img_type) {
   if (iCameraID != cam.info.CameraID)
      return ASI_ERROR_INVALID_ID;
   if (!cam.open || !cam.initialized)
      return ASI_ERROR_CAMERA_CLOSED;
   if (cam.capturing)
      return ASI_ERROR_GENERAL_ERROR;
   if (iWidth % 8 || iHeight % 2 || (iWidth*iHeight) % 1024)
      return ASI_ERROR_INVALID_SIZE;
   if (iBin != 1)
      return ASI_ERROR_GENERAL_ERROR;
   if (Img_type != ASI_IMG_RGB24)
      return ASI_ERROR_INVALID_IMGTYPE;
   cam.frame = QImage{iWidth, iHeight, QImage::Format_RGB888};
   return ASI_SUCCESS;
}

ASI_ERROR_CODE ASIStartVideoCapture(int iCameraID) {
   if (iCameraID != cam.info.CameraID)
      return ASI_ERROR_INVALID_ID;
   if (!cam.open || !cam.initialized)
      return ASI_ERROR_CAMERA_CLOSED;
   if (!cam.capturing) {
      cam.capturing = true;
      cam.timer.start();
   }
   return ASI_SUCCESS;
}

ASI_ERROR_CODE ASIStopVideoCapture(int iCameraID) {
   if (iCameraID != cam.info.CameraID)
      return ASI_ERROR_INVALID_ID;
   if (!cam.open || !cam.initialized)
      return ASI_ERROR_CAMERA_CLOSED;
   cam.capturing = false;
   return ASI_SUCCESS;
}

void Cam::newFrame() {
   timer.start();
   frame.fill(Qt::blue);
   QPainter p{&frame};
   p.setFont({"Helvetica", 48});
   p.setPen(Qt::white);
   p.drawText(frame.rect(), Qt::AlignCenter,
              QStringLiteral("Hello,\nWorld!\n%1").arg(
                 QTime::currentTime().toString(QStringLiteral("hh:mm:ss.zzz"))));
}

ASI_ERROR_CODE ASIGetVideoData(int iCameraID, unsigned char* pBuffer, long lBuffSize, int iWaitms) {
   if (iCameraID != cam.info.CameraID)
      return ASI_ERROR_INVALID_ID;
   if (!cam.open || !cam.initialized)
      return ASI_ERROR_CAMERA_CLOSED;
   if (!cam.capturing)
      return ASI_ERROR_GENERAL_ERROR;

   auto left_ms = cam.exposure_ms - cam.timer.elapsed();
   if (left_ms > 0) {
      auto sleep_ms = (iWaitms == -1) ? left_ms : std::min(left_ms, (qint64)iWaitms);
      QThread::msleep(sleep_ms);
      if (left_ms > sleep_ms)
         return ASI_ERROR_TIMEOUT;
   }
   cam.newFrame();
   memcpy(pBuffer, cam.frame.constBits(), std::min(lBuffSize, (long)cam.frame.byteCount()));
   return ASI_SUCCESS;
}
