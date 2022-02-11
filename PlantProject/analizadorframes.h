#ifndef ANALIZADORFRAMES_H
#define ANALIZADORFRAMES_H

#include "analizador.h"
#include "enums.h"

class AnalizadorFrames : Analizador
{
public:
    AnalizadorFrames(): _umbralizado(Umbralizado::Fijo), _contraste(2.0), _mostrar(false) {}

    void analizar_2_frames();
    void analizar_video();

    Umbralizado umbralizado();
    double contraste();
    bool mostrar();

    void set_umbralizado(Umbralizado umbralizado);
    void set_contraste(double contraste);
    void set_mostrar(bool mostrar);

    ~AnalizadorFrames();
private:
    Umbralizado _umbralizado;
    double _contraste;
    bool _mostrar;
    cv::Mat _frame;

    void analizar_frame(std::string name);
    void mostrar_frame(cv::Mat frame, std::string name);

    // PREPROCESADO
    void preprocesar();

    // UMBRALIZADO
    void umbralizar();
    void umbral_fijo();
    void umbral_medio();
    void umbral_adaptativo_media();
    void umbral_adaptativo_gauss();
    void umbral_otsu();

    // MORFOLOGÍA
    void invertir();
    void abrir();
};

#endif // ANALIZADORFRAMES_H
