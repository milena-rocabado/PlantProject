#ifndef ANALIZADOR_H
#define ANALIZADOR_H

#include <string>
#include <opencv2/opencv.hpp>

class Analizador {
public:
    bool set_video(std::string path);
    void show_video();

    virtual void analizar_video();
    virtual ~Analizador();

protected:
    static cv::VideoCapture _video;
    static cv::VideoWriter _salida;

private:
    std::string outfilename(const std::string& filename);
};

#endif // ANALIZADOR_H
