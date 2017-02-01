#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<iostream>
#include<fstream>
#include"jsoncpp/dist/json/json.h"
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
void modifyBoundingRect(Rect* crop, Mat* inFrame);
void saveDetails(Json::Value root);
int main(){
  char modifing;
  Json::Value root;
  Json::Reader iFile;
  std::ifstream test("dataset/dataSet.json", std::ifstream::binary);
  bool parsedCorrectly = iFile.parse(test,root,false);
  if(!parsedCorrectly){
    std::cout<<"File did not parse correclty"<<std::endl;
  }
  Json::Value files;
  //move to for loop and do not append
  Json::Value beanBags;
  //std::cout<<root.size()<<std::endl;
  Mat frame;
  int offset=0;
  int x;
  std::cout<<"Press <ESC> to exit or any key to move to next image"<<std::endl;
  for(int i=offset;i<root.size();i++){
    Json::Value board;
    Json::Value boundingRect;
    std::cout<<"reference: "<<i<<std::endl;
    files.append(root[i].get("_reference", "" ));
    beanBags.append(root[i].get("beanBags",""));
    board = root[i]["board"];
    frame=imread("dataset/"+files[i-offset].asString());

    // Boarder for board and hole
    {
      Json::Value center;
      Json::Value hole;
      Json::Value size;
      Json::Value holeCenter;
      center = board.get("center","");
      hole = board.get("hole","");
      int radius = hole.get("radius","").asInt();
      holeCenter=hole.get("center","");
      size = board.get("size","");
      int width=size.get("width","").asInt();
      int height=size.get("height","").asInt();
      Rect rect =Rect(center[0].asInt()-width/2,center[1].asInt()-height/2,width,height);
      cv::rectangle(frame,rect,(0,255,0),2);
      circle(frame,Point(holeCenter[0].asInt(),holeCenter[1].asInt()),radius,Scalar(255,255,255));
    }

    //display beanbags
    for(int j=0;j<beanBags[i-offset].size();j++){
      //pull out annotation
      boundingRect=beanBags[i-offset][j].get("bounded_rectangle",""); 
      std::string location=beanBags[i-offset][j].get("location","").asString();
      Json::Value center;
      std::string color=beanBags[i-offset][j].get("color","").asString();
      center=boundingRect.get("center","");
      int width=boundingRect.get("width","").asInt();
      int height=boundingRect.get("height","").asInt();
      //create rectagnle around beanbag
      Rect rect =Rect(center[0].asInt()-width/2,center[1].asInt()-height/2,width,height);
      Scalar rectColor=Scalar(0,0,0);
      Scalar fontColor=Scalar(0,0,0);
      if(color.compare("red")==0){
        rectColor=Scalar(0,0,255);
        fontColor=Scalar(0,0,150);
      }else if(color.compare("blue")==0){
        rectColor=Scalar(255,0,0);
        fontColor=Scalar(150,0,0);
      }
      //draw rectangle of beanbag on frame
      rectangle(frame,rect,rectColor);
      putText(frame,location,Point(center[0].asInt(),center[1].asInt()),FONT_HERSHEY_SIMPLEX,1,fontColor,2);
      }
    imshow(files[i-offset].asString(),frame);
    x=waitKeyEx(0);
    if (x == 27) {
      destroyAllWindows();
      break;
    }
    destroyAllWindows();
  }
  return 0;
}
