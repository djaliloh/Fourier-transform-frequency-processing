////*********************************************************************////
////TP2 2eme Partie: Traitements fréquentiels au spectre d'une image     ////
////	               donnée en argument  			        ////          
////     							       ////
////     Auteur: OUSSEINI HAMZA Abdoul-Djalil                         ////  			
////     Promotion: 22    
////                 /*TRAITEMENTS.cpp*/             
////							            ////																		////
////                       					   ////
////************************************************************* ////

#include "Traitements.h"
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// Fonction pour redimensionner l image entree



Mat ResizeImage (Mat ImageEntree){

	    Mat padded;     //image redimensionnee  a renvoyer en sortie

	    // taille optimale de l image redimensionnee
	    int m = getOptimalDFTSize( ImageEntree.rows );
	    int n = getOptimalDFTSize( ImageEntree.cols );

	    //Ajout de valeur de bourrage dans l'image redimensionnee
	    copyMakeBorder(ImageEntree, padded, 0, m - ImageEntree.rows, 0, n - ImageEntree.cols, BORDER_CONSTANT, Scalar::all(0));
	    return padded;

}

Mat FourierTransform (Mat ImageRedimensionnee){


	    // Creation de deux Conteneurs avec valeurs flottantes pour les parties reelles et imaginaires de la transformee de fourier
	    Mat Conteneurs[] = {Mat_<float>(ImageRedimensionnee), Mat::zeros(ImageRedimensionnee.size(), CV_32F)};

	    //Concatenation des deux conteneurs
	    Mat ImageTransFourier;
	    merge(Conteneurs, 2, ImageTransFourier);

	    //Transformee de fourier
	    dft(ImageTransFourier, ImageTransFourier);

	    return ImageTransFourier;
}

Mat ComputeNorme (Mat ImageTransFourier){


	    vector<Mat> Conteneurs;
	    // separation des parties reelles et imaginaires
	    split(ImageTransFourier, Conteneurs);

	    // Calcul de norme de la transformee de fourier
	    magnitude(Conteneurs[0], Conteneurs[1], Conteneurs[0]);
	    Mat ImageNormeFourier = Conteneurs[0];

	    // Passage a l echelle logarithmique
	    ImageNormeFourier += Scalar::all(1);
	    log(ImageNormeFourier, ImageNormeFourier);


	    // recadrage du spectre
	    ImageNormeFourier = ImageNormeFourier(Rect(0, 0, ImageNormeFourier.cols & -2, ImageNormeFourier.rows & -2));
	    normalize(ImageNormeFourier, ImageNormeFourier, 0, 1, CV_MINMAX);
	    Mat Sortie;
	    ImageNormeFourier.convertTo(Sortie, CV_8UC1,255);

	    return Sortie;
}

Mat CadransChange (Mat ImageNormeFourier){

	    int cx = ImageNormeFourier.cols/2;
	    int cy = ImageNormeFourier.rows/2;

	    Mat cadran0(ImageNormeFourier, Rect(0, 0, cx, cy));   // Cadrant en haut à gauche
	    Mat cadran1(ImageNormeFourier, Rect(cx, 0, cx, cy));  // Cadrant en haut à droite
	    Mat cadran2(ImageNormeFourier, Rect(0, cy, cx, cy));  // Cadrant en bas à gauche
	    Mat cadran3(ImageNormeFourier, Rect(cx, cy, cx, cy)); // Cadrant en bas à droite

	    Mat tmp;                           // tampon pour l echange des cadrants

	    //Echange entre le Cadrant en haut à gauche et le Cadrant en bas à droite
	    cadran0.copyTo(tmp);
	    cadran3.copyTo(cadran0);
	    tmp.copyTo(cadran3);

	    //Echange entre le Cadrant en haut à droite et le Cadrant en bas à gauche
	    cadran1.copyTo(tmp);
	    cadran2.copyTo(cadran1);
	    tmp.copyTo(cadran2);

	    return ImageNormeFourier;

}

Mat TransFourierInverse (Mat ImageTransFourier, int nblignes, int nbcolones){
	Mat ImageRestauree;
	Mat ImageFourierInverse;
	vector<Mat> Conteneurs;

	//Transformee inverse de fourier
	dft( ImageTransFourier,ImageFourierInverse,DFT_INVERSE + DFT_SCALE);

	split(ImageFourierInverse,Conteneurs);

	ImageRestauree = Conteneurs[0];

	//Recadrage de l image
	ImageRestauree = ImageRestauree(Rect(0, 0, ImageRestauree.cols & -2, ImageRestauree.rows & -2));

	//Conversion en image à niveaux de gris
	ImageRestauree.convertTo(ImageRestauree, CV_8UC1);
	Mat Sortie (ImageRestauree, Rect(0,0, nbcolones, nblignes));

	return Sortie;
}

Mat FiltrePasseBas (Mat ImageTransFourier, float freqcoupure ){
	    Mat ImageFiltrePasseBas;
	    vector<Mat> Conteneurs;
	    float cx = ImageTransFourier.cols / (float) 2;
		float cy = ImageTransFourier.rows / (float) 2;
		// Determination du rayon du cercle
		int rayon = (int) (min(float(cx), float(cy)))*freqcoupure;

		split(ImageTransFourier,Conteneurs);

		// Mise à jour des valeurs à l'extérieur du cercle
		for(int k = 0; k < (int) Conteneurs.size(); k++){
			CadransChange(Conteneurs[k]);
			for(int i = 0; i < Conteneurs[k].rows; i++)
				for(int j = 0; j < Conteneurs[k].cols; j++)
				{
					if(pow((j - cx),2) + pow((i - cy),2) > pow(rayon,2))
						Conteneurs[k].at<float>(i, j) = 0;
				}
		}
		merge(Conteneurs,ImageFiltrePasseBas);


		return ImageFiltrePasseBas;

}

Mat FiltrePasseHaut (Mat ImageTransFourier, float freqcoupure ){
	    Mat ImageFiltrePasseHaut;
	    vector<Mat> Conteneurs;
	   float cx = ImageTransFourier.cols / (float) 2;
		float cy = ImageTransFourier.rows / (float) 2;
		// Determination du rayon du cercle
		int rayon = (int) (min(float(cx), float(cy)))*freqcoupure;

		split(ImageTransFourier,Conteneurs);
		// Mise à jour des valeurs à l'interieur du cercle
		for(int k = 0; k < (int) Conteneurs.size(); k++){
			CadransChange(Conteneurs[k]);
			for(int i = 0; i < Conteneurs[k].rows; i++)
				for(int j = 0; j < Conteneurs[k].cols; j++)
				{
					if(pow((j - cx),2) + pow((i - cy),2) < pow(rayon,2))
						Conteneurs[k].at<float>(i, j) = 0;
				}
		}
		merge(Conteneurs,ImageFiltrePasseHaut);

		return ImageFiltrePasseHaut;

}

