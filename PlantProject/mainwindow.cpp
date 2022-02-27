#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDebug>
#include "AnalizadorVideo.h"
#include "AnalizadorFrames.h"
#include "AnalizadorBgSubtractor.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initialize();
    //signalsAndSlots();
}
//-----------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//-----------------------------------------------------------------
void MainWindow::initialize()
{
    spanish = new QLocale(QLocale::Spanish);

    ui->cboUmbr->clear();
    for (int i = Umbralizado::Medio; i <= Umbralizado::Otsu; i++) {
        Umbralizado umb = static_cast<Umbralizado>(i);
        QVariant v = QVariant::fromValue(umb);
        QString str = QString::fromStdString
                (AnalizadorFrames::umbralizado_to_string(umb));
        ui->cboUmbr->addItem(str, v);
    }
    ui->spnUmbContraste->setValue(AnalizadorFrames::CONTRASTE_DF);
    ui->cboUmbr->setCurrentIndex(0);

    ui->spnSegmContraste->setValue(AnalizadorVideo::CONTRASTE_DF);
    ui->spnNum->setValue(AnalizadorVideo::NUM_FRAMES_DF);
    ui->spnSpace->setValue(AnalizadorVideo::ESPACIADO_DF);

    ui->spnNum->setEnabled(false);
    ui->spnSpace->setEnabled(false);

    ui->cboAlg->clear();
    for (int i = AlgoritmoSegm::KNN; i <= AlgoritmoSegm::MOG2; i++) {
        AlgoritmoSegm alg = static_cast<AlgoritmoSegm>(i);
        QVariant v = QVariant::fromValue(alg);
        QString str = QString::fromStdString
                (AnalizadorBgSubtractor::algoritmo_to_string(alg));
        ui->cboAlg->addItem(str, v);
    }

    // quitar ?
    ui->lneVideoPath->setText("C:/Users/milena/git/PlantProject/Media/climbing_bean_project3_leaf_folding.AVI");
}
//-----------------------------------------------------------------
void MainWindow::signalsAndSlots()
{
    connect(ui->pbEjecutar, &QAbstractButton::clicked,
            this, &MainWindow::on_pbEjecutar_clicked);
    connect(ui->chkAdaptativo, &QCheckBox::stateChanged,
            this, &MainWindow::on_chkAdaptativo_stateChanged);
}
//-----------------------------------------------------------------
void MainWindow::on_pbEjecutar_clicked()
{
    qDebug() << "Signal received: " << sender();
    ui->pbEjecutar->setEnabled(false);

    set_working_directory();
    if (ui->tabWidget->currentWidget() == ui->tabUmbr)
    {
        crear_analizador_frames();
    }
    else if (ui->tabWidget->currentWidget() == ui->tabSegm)
    {
        crear_analizador_video();
    }
    else
    {
        crear_analizador_bgS();
    }

    if (set_file()) {
        a->analizar_video();
    }
}
//-----------------------------------------------------------------
void MainWindow::on_pbExaminar_clicked()
{
    QString fp = QFileDialog::getOpenFileName(this, "Seleccionar video");
    ui->lneVideoPath->setText(fp);
}
//-----------------------------------------------------------------
void MainWindow::on_chkAdaptativo_stateChanged(int state)
{
    ui->spnNum->setEnabled(state == Qt::CheckState::Checked);
    ui->spnSpace->setEnabled(state == Qt::CheckState::Checked);
}
//-----------------------------------------------------------------
void MainWindow::set_working_directory()
{
    string fp = ui->lneVideoPath->text().toStdString();
    size_t last_slash = fp.find_last_of("/");
    if (last_slash != string::npos)
        Analizador::set_wd(fp.substr(0, last_slash + 1));
}
//-----------------------------------------------------------------
bool MainWindow::set_file()
{
    string fp = ui->lneVideoPath->text().toStdString();
    size_t last_slash = fp.find_last_of("/");
    bool success = false;
    if (last_slash != string::npos)
        success = a->set_video(fp.substr(last_slash + 1));

    return success;
}
//-----------------------------------------------------------------
void MainWindow::crear_analizador_frames()
{
    qDebug() << "AnalizadorFrames";
    a = new AnalizadorFrames;
    AnalizadorFrames *analizador = dynamic_cast<AnalizadorFrames *>(a);

    Umbralizado umb = static_cast<Umbralizado>(ui->cboUmbr->currentIndex());
    double contraste = spanish->toDouble(ui->spnUmbContraste->text());
    bool mostrar = ui->chkEjem->isChecked();

    analizador->set_umbralizado(umb);
    qDebug() << "\tUmbralizado: " << umb;
    analizador->set_contraste(contraste);
    qDebug() << "\tContraste: " << contraste;
    analizador->set_mostrar(mostrar);
    qDebug() << "\tMostrar: " << mostrar;
}
//-----------------------------------------------------------------
void MainWindow::crear_analizador_video()
{
    qDebug() << "AnalizadorVideo";
    a = new AnalizadorVideo();
    AnalizadorVideo *analizador = dynamic_cast<AnalizadorVideo *>(a);

    bool adp = ui->chkAdaptativo->checkState() == Qt::CheckState::Checked;
    double contraste = spanish->toDouble(ui->spnSegmContraste->text());

    analizador->set_adaptativo(adp);
    qDebug() << "\tModelo adaptativo: " << adp;
    analizador->set_contraste(contraste);
    qDebug() << "\tContraste: " << contraste;

    if (adp) {
        int num = ui->spnNum->text().toInt();
        int sp = ui->spnSpace->text().toInt();

        analizador->set_num_frames(num);
        qDebug() << "\tFrames: " << num;
        analizador->set_espaciado(sp);
        qDebug() << "\tEspaciado: " << sp;
    }
}
//-----------------------------------------------------------------
void MainWindow::crear_analizador_bgS()
{
    qDebug() << "AnalizadorBgSubtractor";
    a = new AnalizadorBgSubtractor();
    AnalizadorBgSubtractor *analizador = dynamic_cast<AnalizadorBgSubtractor *>(a);

    AlgoritmoSegm alg = static_cast<AlgoritmoSegm>(ui->cboAlg->currentIndex());
    qDebug() << "\tAlgoritmo de segmentación: "
             << QString::fromStdString(AnalizadorBgSubtractor::algoritmo_to_string(alg));
    analizador->set_algoritmo_segm(alg);
}
