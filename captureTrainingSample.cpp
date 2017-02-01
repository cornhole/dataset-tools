#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/features2d/features2d.hpp>
#include<iostream>
#include<fstream>
#include<iomanip>
#include<boost/lexical_cast.hpp>
#include<unistd.h>
#include"jsoncpp/dist/json/json.h"
#include<boost/filesystem.hpp>

using namespace cv;

struct beanBag{
  std::string color;
  std::string location;
  int xCenter;
  int yCenter;
  int width;
  int height;
};

void loopImg(VideoCapture* camera);
void saveDetails(std::string fileName,beanBag* bb,int numBeanBags);
void modifyBoundingRect(Rect* crop,Mat* inFrame);
int findBoundingBoxes(Mat* frame, Mat* preProcessedFrame,Rect* boundingBoxes,std::vector<std::vector<Point> >* contours);

int main(int argc, char *argv[]){
  // variable declaration
  const int maxBeanBags = 8;
  std::string color;
  std::string quitFlag;
  std::string location;
  std::string addBeanBag;
  int fileNum;
  int numberBlue=0;
  int numberRed=0;
  int totalBeanBags =0;
  bool isOpen=0;
  std::vector<std::vector<Point> > contours;
  Mat newImage,input,hsvImage,roi;
  Rect* boundingBoxes=new Rect[25];
  Rect* boundingBoxesRed=new Rect[25];
  Rect crop;
  beanBag* bb;
  std::string path="./dataset/";

  // initialize camera 
  VideoCapture camera = VideoCapture();
  int cameraNumber = 0;
  isOpen = camera.open(cameraNumber);
  if (!isOpen) {
    std::cout<<"Camera "<< cameraNumber <<" is not available, make sure the camera is connected correctly\n";
    isOpen = camera.open(++cameraNumber);
    if (!isOpen) {
      std::cout<<"Camera "<< cameraNumber <<" is not available, make sure the camera is connected correctly. \nNo camera found exiting program.\n";
      exit(0);
    }
  }
  std::cout<<"Connected to camera "<< cameraNumber <<"\n\n";

  // set frame width & height
  camera.set(CV_CAP_PROP_FRAME_WIDTH,1920);
  camera.set(CV_CAP_PROP_FRAME_HEIGHT,1080);

  // Ask for next data point number
  std::cout<<"Enter the next file #\n";
  std::cin>>fileNum; 
  
  while(true){
    totalBeanBags = 0;
    numberBlue = 0;
    numberRed = 0;

    // new filename
    std::ostringstream filename;
    filename<<std::setfill('0')<<std::setw(4)<<fileNum++<<".jpg";

    // make sure newImage isn't empty
    loopImg(&camera);
    camera>>newImage;
    
    newImage.adjustROI(0,0,-100,-200);
    cvtColor(newImage,hsvImage,COLOR_RGB2HSV);

    // find potential boundingBoxes for blue beanbags
    // Comment to find boundingBoxes by hand only
    inRange(hsvImage,Scalar(0,105,35),Scalar(97,255,255),roi);
    findContours(roi,contours,CV_RETR_LIST,CV_CHAIN_APPROX_NONE);
    numberBlue=findBoundingBoxes(&newImage,&roi,boundingBoxes,&contours);

    // find potential boundingBoxes for red beanbags
    // Comment to find boundingBoxes by hand only
    inRange(hsvImage,Scalar(98,105,35),Scalar(149,255,255),roi);
    findContours(roi,contours,CV_RETR_LIST,CV_CHAIN_APPROX_NONE);
    numberRed=findBoundingBoxes(&newImage,&roi,boundingBoxesRed,&contours);

    // add red bounding boxes to blue bounding boxes
    // Comment to find boundingBoxes by hand only
    for(int j=0;j<numberRed;j++){
      boundingBoxes[numberBlue+j]=boundingBoxesRed[j];
    }

    totalBeanBags = numberBlue+numberRed;
    bb = new beanBag[maxBeanBags];
    if (totalBeanBags > maxBeanBags) {
      totalBeanBags = maxBeanBags;
      //totalBeanBags=7;
    }
    std::cout<<"Total number of bounding boxes found "<< totalBeanBags<<"\n\n";

    for(int i=0;i<totalBeanBags;i++){
      crop=Rect(boundingBoxes[i].x,boundingBoxes[i].y,boundingBoxes[i].width,boundingBoxes[i].height);  
      // modify crop area for boundingBox
      modifyBoundingRect(&crop,&newImage); 
      std::cout<<"What color is the beanBag (red/blue)\n";
      std::cin >> bb[i].color; 
      std::cout<<"Where is the location of the beanBag (on/off/in)\n";
      std::cin >>bb[i].location;
      bb[i].xCenter=crop.x+crop.width/2;
      bb[i].yCenter=crop.y+crop.height/2;
      bb[i].width=crop.width;
      bb[i].height=crop.height;
    }
    while(totalBeanBags < maxBeanBags){
      std::cout<<"Add another bounding rectangle for missed beanBags? (y/n)\n";
      std::cin >> addBeanBag;
      if(addBeanBag.compare("y")==0){
        crop=Rect(100,100,50,50);
        modifyBoundingRect(&crop,&newImage);
        std::cout<<"What color is the beanBag (red/blue)\n";
        std::cin >> bb[totalBeanBags].color; 
        std::cout<<"Where is the location of the beanBag (on/off/in)\n";
        std::cin >>bb[totalBeanBags].location;
        bb[totalBeanBags].xCenter=crop.x+crop.width/2;
        bb[totalBeanBags].yCenter=crop.y+crop.height/2;
        bb[totalBeanBags].width=crop.width;
        bb[totalBeanBags++].height=crop.height;
      }else{break;}
    }

    std::cout<<"end\n";
    destroyAllWindows();
    imwrite(path+filename.str(),newImage);
    saveDetails(filename.str(),bb,totalBeanBags);

    std::cout<<"Capture another data point (y/n)?\n";
    std::cin>>quitFlag; 
    if(quitFlag.compare("n")==0){ break;}
  }
  camera.release();
}

void loopImg(VideoCapture* camera) {
    Mat img;
    camera ->read(img);
    camera ->read(img);
    camera ->read(img);
    camera ->read(img);
    camera ->read(img);
    camera ->read(img);
}

void saveDetails(std::string fileName,beanBag* bb,int numBeanBags){
  std::ofstream myfile;
  Json::Value root;
  Json::Reader iFile;
  std::ifstream test("dataset/dataSet.json", std::ifstream::binary);
  bool parsedCorrectly = iFile.parse(test,root,false);
  if(!parsedCorrectly){
    std::cout<<"File did not parse correclty\n";
  }
  Json::Value files;
  Json::Value beanBags;
  Json::Value mycenter; 
  Json::Value beans;
  Json::Value rect;
  files["_reference"]=fileName;
  // parse the json file, add another data point to the file... {_reference: ~~~ beanBags:~~}
  for(int i=0;i<numBeanBags;i++){
    mycenter.clear();
    std::string name="beanBag_"+boost::lexical_cast<std::string>(i);
    mycenter.append(bb[i].xCenter);
    mycenter.append(bb[i].yCenter);
    //mycenter.append(center[1]);
    rect["bounded_rectangle"]["center"]=mycenter;
    rect["color"]=bb[i].color;
    rect["bounded_rectangle"]["width"]=bb[i].width;
    rect["bounded_rectangle"]["height"]=bb[i].height;
    rect["location"]=bb[i].location;
    beans.append(rect);
  }
  files["beanBags"]=beans;
  root.append(files);
  myfile.open("./dataset/dataSet.json",std::ios::trunc);
  myfile<<root.toStyledString();
  myfile.close();
}

void modifyBoundingRect(Rect* crop, Mat* inFrame){
  Mat temp;
  inFrame->copyTo(temp);
  rectangle(temp,*crop,Scalar(0,0,0),1);
  imshow("CropIMG",temp);
  std::cout<<"Focus on CropIMG. Move bounding rectangle with arrows and resize with shift + arrows. Hit (q) when complete\n";
  int x=0;
  while(true){
    x=waitKeyEx(0);
    if (x != 255 && x !=-1) {
      std::cout<<x<<"\n";
    }
    if(x==113){break;}
    else if(x==65363){
      crop->x+=5;
    }else if(x==65362){
      crop->y-=5;
    }else if(x==65361){
      crop->x-=5;
    }else if(x==65364){
      crop->y+=5;
    }else if(x==130899){
      crop->width+=5;
    }else if(x==130898){
      crop->height-=5;
    }else if(x==130897){
      crop->width-=5;
    }else if(x==130900){
      crop->height+=5;
    }
    std::cout<<crop->x<<" "<<crop->y<<" "<<crop->width<<" "<<crop->height<<"\n";
    inFrame->copyTo(temp);
    rectangle(temp,*crop,Scalar(0,0,0),1);
    imshow("CropIMG",temp);
  }
}
int findBoundingBoxes(Mat* frame, Mat* preProcessedFrame,Rect* boundingBoxes ,std::vector<std::vector<Point> >* contours){
  int minArea=500;
  int maxArea=10000;
  int numObjects=contours->size();
  std::vector<Moments>allMoments(numObjects);
  RotatedRect bag;
  Rect boundedBox;
  int counter=0;
  if(numObjects>0){
    for(int index=0;index<numObjects;index++){
      allMoments[index]=moments(contours[0][index]);
      if(allMoments[index].m00>minArea&&allMoments[index].m00<maxArea){
        float area=allMoments[index].m00;
        bag=minAreaRect(contours[0][index]);
        boundedBox=bag.boundingRect();
        if(boundedBox.x>0&&boundedBox.x+boundedBox.width<frame->cols&&boundedBox.y>0&&boundedBox.y+boundedBox.height<frame->rows){
          boundingBoxes[counter++]=boundedBox; 
        }
      }
    }
  }
  return counter;
}
//void discoverContours(Mat* ioFrame,std::vector<std::vector<Point> >* contours, int algorithm){
//  std::vector<Vec4i> hierarchy;
//  //findContours(*ioFrame,*contours,hierarchy,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);
//  findContours(*ioFrame,*contours,CV_RETR_LIST,CV_CHAIN_APPROX_NONE);
//  //std::cout<<contours[0][0]<<stdstd::endl;
//}
