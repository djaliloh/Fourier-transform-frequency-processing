////*********************************************************************////
//// TP2 1ere Partie: Transformé de fourier d'une image et restauration ////
////			d'une image à partir de sa transformé fourier	////          
////     							       ////
////     Auteur: OUSSEINI HAMZA Abdoul-Djalil                         ////  			
////     Promotion: 22                              	             ////             
////							            ////																		////
////                       					   ////
////************************************************************* //// 																									////
													 										
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

	    return ImageNormeFourier;
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

	    normalize(ImageNormeFourier, ImageNormeFourier, 0, 1, CV_MINMAX);

	    Mat Sortie;

	    ImageNormeFourier.convertTo(Sortie, CV_8UC1,255);

	    return Sortie;

}

Mat TansFourierInverse (Mat ImageTransFourier, int nblignes, int nbcolones){
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

int main (int argc, char** argv){


	if(argc!= 2 ){
		cout << "Veuillez entrer le systaxe suivante:";
		cout << " ./FourierTrans nom_image"<<endl;
	}
	else
	{
		cout << "Voici le spectre de l image entree et l'image issue de la transformee inverse du spectre"<<endl;

		const char* NomImage = argv[1];
		// Ouverture de l'image en nuance de gris
		Mat ImageEntree = imread(NomImage, CV_LOAD_IMAGE_GRAYSCALE);

		if(!ImageEntree.data){
			cout << "Veuillez fournir une image valide"<<endl;
			return -1;
		}
		else{

			 // Calcul de la transformé de fourier de l'image

			 Mat TransFourierImage = FourierTransform(ResizeImage(ImageEntree));

			 Mat SpectreImage = CadransChange(ComputeNorme(TransFourierImage));

			 Mat ImageRestauree = TansFourierInverse(TransFourierImage,ImageEntree.rows, ImageEntree.cols);

			 imshow("Image Entree", ImageEntree);

			 imshow("Spectre de Fourier de l image entree", SpectreImage);

			 imshow("Transformee inverse du spectre", ImageRestauree);

			 // Enregistrement de l'image originale
			 if(!imwrite("Image_Entree.png", ImageEntree))
			     cout<<"Erreur lors de l'enregistrement"<<endl;

			// Enregistrement de l'image du spectre
			 if(!imwrite("Spectre.png", SpectreImage))
			     cout<<"Erreur lors de l'enregistrement"<<endl;

			// Enregistrement de l'image de la transformée de fourier
			 if(!imwrite("Image_Transformee-Inverse.png", ImageRestauree))
			     cout<<"Erreur lors de l'enregistrement"<<endl;

		}
	}
	waitKey();
	return 0;
}




