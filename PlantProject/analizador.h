#ifndef ANALIZADOR_H
#define ANALIZADOR_H

#include <string>
#include <opencv2/opencv.hpp>

bool set_video(std::string path);

// --------------------------------------------------------
// --------------- OPERACIONES SOBRE VIDEO ----------------
// --------------------------------------------------------

void show_video();

void analizar_2_frames();

void analizar_video();

// --------------------------------------------------------
// --------------- OPERACIONES SOBRE FRAMES ---------------
// --------------------------------------------------------

void analizar_frame(std::string name);

void mostrar_frame(cv::Mat frame, std::string name);

void imprimir_hist(cv::Mat img);

// PREPROCESADO

void preprocesar();

// UMBRALIZADO

void umbralizar();

void umbral_medio();

void umbral_fijo();

// MORFOLOGÍA

void invertir();

void abrir();

#endif // ANALIZADOR_H
