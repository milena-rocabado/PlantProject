#ifndef ANALIZADOR_H
#define ANALIZADOR_H

#include <string>
#include <opencv2/opencv.hpp>

bool set_video(std::string path);

std::string outfilename(const std::string& filename);

void show_video();

void analizar_2_frames();

void analizar_video();

void analizar_frame(cv::Mat img, std::string name);

void umbralizar_frame(cv::Mat img);

void umbral_medio(cv::Mat img);

void umbral_fijo(cv::Mat img);

#endif // ANALIZADOR_H
