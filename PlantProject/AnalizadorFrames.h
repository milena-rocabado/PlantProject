#ifndef ANALIZADORFRAMES_H
#define ANALIZADORFRAMES_H

#include "Analizador.h"
#include <QMetaType>

enum Umbralizado {
    Medio,
    Fijo,
    AdaptativoMedia,
    AdaptativoGauss,
    Otsu
};
Q_DECLARE_METATYPE(Umbralizado)

class AnalizadorFrames : public Analizador
{
public:

    static constexpr double CONTRASTE_DF = 2.0;
    static constexpr Umbralizado UMBRALIZADO_DF = Umbralizado::Fijo;

    AnalizadorFrames():
        _umbralizado(UMBRALIZADO_DF),
        _contraste(CONTRASTE_DF),
        _mostrar(false)
    {}

    void analizar_2_frames();
    void analizar_video() override;

    Umbralizado umbralizado();
    double contraste();
    bool mostrar();

    void set_umbralizado(const Umbralizado &umbralizado);
    void set_contraste(const double &contraste);
    void set_mostrar(const bool &mostrar);

    static std::string umbralizado_to_string(Umbralizado u);

private:
    Umbralizado _umbralizado;
    double _contraste;
    bool _mostrar;

    void analizar_frame(std::string name);
    void mostrar_frame(const cv::Mat &frame, const std::string &name) override;

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
