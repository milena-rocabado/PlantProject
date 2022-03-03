#ifndef ANALIZADORVIDEO_H
#define ANALIZADORVIDEO_H

#include <Analizador.h>

class AnalizadorSegmentacionFijo : public Analizador
{
public:
    static constexpr int NUM_FRAMES_DF = 20;
    static constexpr int ESPACIADO_DF = 5;
    static constexpr double CONTRASTE_DF = 1.75;
    static constexpr int BLUR_DF = 0;

    AnalizadorSegmentacionFijo(const bool &adaptativo,
                    const double &contraste,
                    const int &num_frames,
                    const int &espaciado,
                    const int &blur):
        _adaptativo(adaptativo),
        _contraste(contraste),
        _num_frames(num_frames),
        _espaciado(espaciado),
        _blur(blur)
    {}

    AnalizadorSegmentacionFijo():
        _adaptativo(false),
        _contraste(CONTRASTE_DF),
        _num_frames(NUM_FRAMES_DF),
        _espaciado(ESPACIADO_DF),
        _blur(BLUR_DF)
    {}

    bool adaptativo();
    double contraste();
    int num_frames();
    int espaciado();
    int blur();
    void set_adaptativo(const bool &adaptativo);
    void set_contraste(const double &contraste);
    void set_num_frames(const int &num_frames);
    void set_espaciado(const int &espaciado);
    void set_blur(const int &blur);

    virtual bool set_video(const std::string& path) override;

    void analizar_video() override;
    void analizar_2_frames();
    void crear_imagen_media(std::string fp);

    ~ AnalizadorSegmentacionFijo() override;
private:
    bool _adaptativo;
    double _contraste;
    int _num_frames;
    int _espaciado;
    int _blur;
    int _i;

    bool _ejemplo;
    bool _apply_blur;
    bool _apply_contrast;
    bool _debug;

    cv::Mat _modelo;
    cv::Mat _mascara;

    cv::VideoWriter _mascara_of;

    void mostrar_frame(const cv::Mat &frame, const std::string &name) override;

    void crear_modelo();
    void cargar_modelo();
    void calcular_mascara();
    void iterar_contrastes();
    void iterar_blur();
    void iterar_umbrales();
    void procesar();
};

#endif // ANALIZADORVIDEO_H
