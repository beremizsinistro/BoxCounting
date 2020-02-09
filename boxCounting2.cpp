#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <vector>
#include <dirent.h>
#include <cstdlib>

using namespace cv;

using namespace std;

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



int main (int agrc, char* argv[]){
    
    vector<string> images; //vetor com os nomes das imagens
    images.clear();
    le_diretorio("./imagens/modificadas", images); // le o diretorio com as imagens tratadas e coloca no vetor os nomes das imagens
    sort(images.begin(), images.end()); //coloca as imagens em ordem

    vector<vector<vector<bool>>> imagensHits;
    imagensHits.clear(); // vetor onde será colocado os resultados dos quadrados de cada imagem a cada ciclo
    vector<float> logL; // vetor onde será colocado o resultado do eixe X, log(1/L) do stack a cada ciclo
    logL.clear();
    int arestaInicial; // aresta inicial do cubo. Ela diminui a cada ciclo.

   
    Mat imagemParaCalculo = imread("./imagens/modificadas/"+images[0], IMREAD_GRAYSCALE);
    
    int alturaStack, menor, maior, larguraImagem, alturaImagem; 
    
    larguraImagem = imagemParaCalculo.size().width;
    alturaImagem = imagemParaCalculo.size().height;
    alturaStack = images.size();
    
    menor = min({larguraImagem, alturaImagem, alturaStack});
    maior = max({larguraImagem, alturaImagem, alturaStack});

    if(maior/4 > menor){
        arestaInicial = menor;
    }else
    {
        arestaInicial = maior/4;
    }
        
        
    for(int index = 0; index < images.size(); index++){ // Loop que varrerá o vetor de imagens e fará o cálculo de box counting nos slices
        Mat img; // variável responsável pela imagem.
        vector<bool> hits; // conterá zeros e uns. A cada ciclo a imagem será dividida em um uma malha de quadrados. Se dentro do quadrado não houver
        vector<vector<bool>> ciclos;
        ciclos.clear(); // vetor com os quadrados de cada ciclo
                    // poro algum o valor do quadrado será zero; caso haja ao menos um ponto de poro, será um.
        hits.clear(); //limpa o vetor caso haja algo dentro dele

        img = imread("./imagens/modificadas/"+images[index], IMREAD_GRAYSCALE); // Da load da imagem como escala de cinza

        int aresta;
        aresta = arestaInicial;
    
    

        int erroY = 0; 
        int erroX = 0;

        while(aresta > 1){
            for(int j=0; j < alturaImagem; j+= aresta){
                for(int i=0; i < larguraImagem; i+= aresta){
                    for(int l =0; l < (erroY = (j+aresta <= alturaImagem) ? aresta : alturaImagem - j); l++){
                        for(int k=0; k< (erroX = (i+aresta <= larguraImagem) ? aresta : larguraImagem - i); k++){
                            if (img.at<uchar>(l+j, k+i) == 0){
                                i+=aresta;
                                l=-1;
                                hits.push_back(1);
                                break;
                            }          
                        }
                    }
                    if(i>larguraImagem)break;
                    hits.push_back(0);
                }
            }
            int aux = ceil(float(maior)/float(aresta));
            logL.push_back(log10(1.0/aux));
            aresta = aresta /2;
            ciclos.push_back(hits);
            hits.clear();
        }
        
        imagensHits.push_back(ciclos);
        //cout << "Imagem número: " << index+1 <<" processada" << endl;
    }
    int erroZ =0;
    vector<bool> v3;
    vector<bool> aux;
    vector<long int> aux2;
    aux2.clear();
    
    vector<int> ciclosCubo; 
    ciclosCubo.clear();
    
    long int cubos = 0;
    vector<float> epoca;
    epoca.clear();
    int numCiclos = imagensHits[0].size();
    
    for(int j=0; j<numCiclos; j++){
        aux2.clear();
        for(int i =0; i< alturaStack; i+=arestaInicial){
            aux.clear();
            aux.assign(imagensHits[0][j].size(), 0);
            for(int k =0; k < (erroZ = (i + arestaInicial <=alturaStack) ? (arestaInicial):(alturaStack - i)); k++){
                v3.clear();
                transform(  aux.begin(), aux.end(), imagensHits[i+k][j].begin(), back_inserter( v3 ), logical_or<>() );
                aux=v3;
                
            }
            for(int l=0; l<aux.size(); l++){
                if(aux[l] == 1){
                    cubos++;
                }
            }
            aux2.push_back(cubos);
            cubos = 0;
        }
        for(int m = 0; m < aux2.size(); m++){
            cubos = cubos + aux2[m];
        }
        cout << cubos << endl;
    
        epoca.push_back(log10(cubos));
        arestaInicial = arestaInicial/2;
        cubos = 0;
    }
    for (int i = 0; i < numCiclos; i++){
        cout << "Eixo X: "<< logL[i] << " Eixo Y: " << epoca[i] << endl;
    }
    double b0 = 0;
    double b1 = 0;
    double alpha = 0.01;
    
    for (int i = 0; i < 10000000; i ++) {
        int id = i % 5;
        double p = b0 + b1 * logL[id];
        double err = p - epoca[id];
        b0 = b0 - alpha * err;
        b1 = b1 - alpha * err * logL[id];
    } 
    cout << "Dimensão Fractal: " << -1*b1 << endl;
    return 0;
}       
