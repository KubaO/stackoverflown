#include <QImage>
#include <QTemporaryFile>
#include <QDebug>
#include <vector>
#include <opencv2/opencv.hpp>

std::vector<cv::Mat> PD_Classifier_VEC;

void PD()
{
  cv::Mat Point_MAT = cv::Mat(PD_Classifier_VEC[0].size(),CV_8UC1,cv::Scalar::all(0));

  //Some Calculation //

  std::stringstream stream;
  stream<<Point_MAT<<"\n";
  int Nonz = cv::countNonZero(Point_MAT);
  cv::imshow("Point_MAT",Point_MAT);

  PD_Classifier_VEC.erase(PD_Classifier_VEC.begin());
}

int main()
{
  QTemporaryFile file;
  file.setFileTemplate(file.fileTemplate() + ".jpg");

  const int N = 100;
  for(int j = 0 ; j < N ; j++)
  {
    file.open();
    QImage img(800, 600, QImage::Format_RGB32);
    img.save(&file);
    file.close();
    QString address = file.fileName();
    cv::Mat image = cv::imread(address.toStdString(),0);
    PD_Classifier_VEC.push_back(image);
  }
  while (!PD_Classifier_VEC.empty()) PD();
  cv::waitKey();
  return 0;
}
