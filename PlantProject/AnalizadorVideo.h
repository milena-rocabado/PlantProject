#ifndef ANALIZADORVIDEO_H
#define ANALIZADORVIDEO_H

#include <Analizador.h>

class AnalizadorVideo : public Analizador
{
public:
    static constexpr int NUM_FRAMES_DF = 20;
    static constexpr int ESPACIADO_DF = 5;
    static constexpr double CONTRASTE_DF = 2.0;

    AnalizadorVideo(const bool &adaptativo,
                    const double &contraste,
                    const int &num_frames,
                    const int &espaciado):
        _adaptativo(adaptativo),
        _contraste(contraste),
        _num_frames(num_frames),
        _espaciado(espaciado)
    {}

    AnalizadorVideo():
        _adaptativo(false),
        _contraste(CONTRASTE_DF),
        _num_frames(NUM_FRAMES_DF),
        _espaciado(ESPACIADO_DF)
    {}

    bool adaptativo();
    double contraste();
    int num_frames();
    int espaciado();
    void set_adaptativo(const bool &adaptativo);
    void set_contraste(const double &contraste);
    void set_num_frames(const int &num_frames);
    void set_espaciado(const int &espaciado);

    virtual bool set_video(const std::string& path) override;

    void analizar_video() override;
    void crear_imagen_media(std::string fp);

    ~ AnalizadorVideo() override;
private:
    bool _adaptativo;
    double _contraste;
    int _num_frames;
    int _espaciado;
    bool _mostrar;

    cv::Mat _modelo; // 32FC3
    cv::Mat _mascara;

    cv::VideoWriter _mascara_of;

    void mostrar_frame(const cv::Mat &frame, const std::string &name) override;

    void crear_modelo();
    void cargar_modelo();
    void calcular_mascara();
};

#endif // ANALIZADORVIDEO_H
