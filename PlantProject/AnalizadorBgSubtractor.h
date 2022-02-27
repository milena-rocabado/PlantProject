#ifndef ANALIZADORBGSUBTRACTOR_H
#define ANALIZADORBGSUBTRACTOR_H

#include <Analizador.h>
#include <opencv2/video/background_segm.hpp>
#include <QMetaType>

enum AlgoritmoSegm {
    KNN,
    MOG2
};
Q_DECLARE_METATYPE(AlgoritmoSegm)

class AnalizadorBgSubtractor : public Analizador
{
public:
    AnalizadorBgSubtractor():
        _algoritmo(KNN)
    {}
    ~AnalizadorBgSubtractor();

    AlgoritmoSegm algoritmo();
    void set_algoritmo_segm(AlgoritmoSegm algoritmo);

    bool set_video(const std::string &path);
    void analizar_video();

    static std::string algoritmo_to_string(AlgoritmoSegm a);

private:
    AlgoritmoSegm _algoritmo;

    cv::Ptr<cv::BackgroundSubtractor> _bgSub;
    cv::Mat _mask;
    cv::Mat _fg;
    cv::Mat _model;

    cv::VideoWriter _mask_out;
    cv::VideoWriter _model_out;

    void crear_bgSubtractor();
};

#endif // ANALIZADORBGSUBTRACTOR_H
