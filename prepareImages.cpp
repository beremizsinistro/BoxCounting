#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iostream>
#include <dirent.h>

using namespace std;

struct AreaCmp {
  AreaCmp(const vector<float>& _areas) : areas(&_areas) {}
  bool operator()(int a, int b) const { return (*areas)[a] > (*areas)[b]; }
  const vector<float>* areas;
};

void le_diretorio(string diretorio_imagens, vector<string>& lista_imagens)
{
    DIR *dir;
    struct dirent *lsdir;
    string nome_arquivo;

    dir = opendir(diretorio_imagens.c_str());

    while ( ( lsdir = readdir(dir) ) != NULL )
    {
        nome_arquivo = lsdir->d_name;
        string auxiliar;

        if(nome_arquivo.length()>4)
        {
            auxiliar = nome_arquivo.substr(nome_arquivo.length()-4).c_str();


            if (auxiliar==".tif")//".tif")
            {
                lista_imagens.push_back(nome_arquivo);
            }
        }


    }

    closedir(dir);
}

int main(int argc, char* argv[]) {

  cv::Mat img, img_edge;
  vector<string> images;
  le_diretorio("./imagens",images);
  cout << " quantidade de imagens: " << images.size() << endl;
  sort(images.begin(), images.end());
  
  
  for(int index = 0; index < images.size(); index++){

    img = cv::imread("./imagens/"+images[index],cv::IMREAD_GRAYSCALE);
    cv::resize(img, img, cv::Size(), 0.35, 0.35);
    cv::threshold(img, img_edge, 130, 255, cv::THRESH_BINARY);
    
    vector< vector< cv::Point > > contours;
    vector< cv::Vec4i > hierarchy;

    cv::findContours(
      img_edge,
      contours,
      hierarchy,
      cv::RETR_LIST,
      cv::CHAIN_APPROX_SIMPLE
    );
    
    vector<int> sortIdx(contours.size());
    vector<float> areas(contours.size());
    for( int n = 0; n < (int)contours.size(); n++ ) {
      sortIdx[n] = n;
      areas[n] = contourArea(contours[n], false);
    }

    // sort contours so that the largest contours go first
    //
    std::sort( sortIdx.begin(), sortIdx.end(), AreaCmp(areas ));
    cout << "sortIdx "<<sortIdx[0] << endl;
    cout << " area 0 : " << areas[sortIdx[0]] << endl;
    
    for( int n = 0; n < 1 ; n++ ) {
      int idx = sortIdx[n];
      
      cv::drawContours(
        img, contours, idx,
        cv::Scalar(0,0,0), CV_FILLED, 8, hierarchy,
        0 // Try different values of max_level, and see what happens
      );
      
    }
    
    cv::Mat res;
    cv::bitwise_or(img, img_edge, res);
    cv::imwrite("./imagens/modificadas/binaria"+to_string(index)+".tif", res);
    
    
  }
  return 0;
}
