 #include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDebug>
#include "GlobalSegmentator.h"
#include "StaticModelSegmentator.h"
#include "BgSubtractorSegmentator.h"
#include "AdaptiveModelSegmentator.h"

const StaticModelSegmentator::Configuration StaticModelSegmentator::CONF1;
const StaticModelSegmentator::Configuration StaticModelSegmentator::CONF2;

using namespace std;

enum FijoConf {
    Conf1,
    Conf2,
    Personalizada
};

class MainWindow::PrivateData {
public:
    map<FijoConf, StaticModelSegmentator::Configuration> fijoConfs =
    {
        { Conf1, StaticModelSegmentator::CONF1 },
        { Conf2, StaticModelSegmentator::CONF2 },
        { Personalizada, StaticModelSegmentator::Configuration() }
    };
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    p(new PrivateData)
{
    ui->setupUi(this);
    initialize();
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
    init_tabGlobal();
    init_tabSegmF();
    init_tabSegmA();
    init_tabBgS();
    init_tabBgS();

    // quitar ?
    ui->chkDebug->setCheckState(Qt::CheckState::Checked);
    ui->lneVideoPath->setText("C:/Users/milena/git/PlantProject/Media/climbing_bean_project3_leaf_folding.AVI");
}
//-----------------------------------------------------------------
bool MainWindow::set_file()
{
    return a->set_video(ui->lneVideoPath->text().toStdString());
}
//-----------------------------------------------------------------
void MainWindow::init_tabGlobal() {
    ui->cboGlobalUmbr->clear();
    for (int i = GlobalSegmentator::ThreshType::Medio; i <= GlobalSegmentator::ThreshType::Otsu; i++) {
        GlobalSegmentator::ThreshType umb = static_cast<GlobalSegmentator::ThreshType>(i);
        ui->cboGlobalUmbr->addItem(QString::fromStdString
                                   (GlobalSegmentator::umbralizado_to_str(umb)));
    }
    ui->spnGlobalContraste->setValue(GlobalSegmentator::ALPHA_DF);
    ui->spnGlobalContraste->setMinimum(1.0);
    ui->cboGlobalUmbr->setCurrentIndex(GlobalSegmentator::THRESHTYPE_DF);
}
//-----------------------------------------------------------------
void MainWindow::init_tabSegmF() {
    p->fijoConfs[Personalizada].name = "Personalizada";
    for (auto pair: p->fijoConfs) {
        ui->cboFijoConf->addItem(pair.second.nombre);
    }
    ui->cboFijoConf->setCurrentIndex(0);

    ui->cboFijoModelo->addItem(StaticModelSegmentator::CONF1.name);
    ui->cboFijoModelo->addItem(StaticModelSegmentator::CONF2.name);
    ui->cboFijoModelo->setCurrentIndex(0);

    ui->spnFijoContraste->setValue(StaticModelSegmentator::CONF1.alpha);
    ui->spnFijoContraste->setMinimum(1.0);
    ui->spnFijoContraste->setEnabled(false);

    ui->spnFijoUmbral->setValue(StaticModelSegmentator::CONF1.thresh);
    ui->spnFijoUmbral->setEnabled(false);
}
//-----------------------------------------------------------------
void MainWindow::init_tabSegmA() {
    ui->spnAdapContraste->setValue(AdaptiveModelSegmentator::ALPHA_DF);
    ui->spnAdapContraste->setMinimum(1.0);
}
//-----------------------------------------------------------------
void MainWindow::init_tabBgS() {
    ui->cboBgSAlg->clear();
    for (int i = BgSubtractorSegemntator::Algorithm::KNN; i <= BgSubtractorSegemntator::Algorithm::MOG2; i++) {
        BgSubtractorSegemntator::Algorithm alg = static_cast<BgSubtractorSegemntator::Algorithm>(i);
        ui->cboBgSAlg->addItem(QString::fromStdString
                               (BgSubtractorSegemntator::algorithm_to_string(alg)));
    }
}
//-----------------------------------------------------------------
//------------------------------SLOTS------------------------------
//-----------------------------------------------------------------
void MainWindow::on_pbEjecutar_clicked() {
    qDebug() << "Signal received: " << sender();

    if (ui->tabWidget->currentWidget() == ui->tabUmbr) {
        crear_analizadorGlobal();
    }
    else if (ui->tabWidget->currentWidget() == ui->tabSegm) {
        crear_analizadorSegmF();
    }
    else if (ui->tabWidget->currentWidget() == ui->tabSegmA) {
        crear_analizadorSegmA();
    }
    else {
        crear_analizadorBgS();
    }

    if (set_file()) {
        if (ui->chkDebug->checkState() == Qt::CheckState::Checked)
            a->process_debug();
        else if (ui->chkTime->checkState() == Qt::CheckState::Checked)
            a->process_timed();
        else
            a->process_video();
    }
}
//-----------------------------------------------------------------
void MainWindow::on_pbExaminar_clicked() {
    QString fp = QFileDialog::getOpenFileName(this, "Seleccionar video");
    ui->lneVideoPath->setText(fp);
}
//-----------------------------------------------------------------
void MainWindow::on_cboFijoConf_currentIndexChanged(int index) {
    ui->spnFijoContraste->setEnabled(index == Personalizada);
    ui->spnFijoUmbral->setEnabled(index == Personalizada);
    ui->cboFijoModelo->setEnabled(index == Personalizada);

    if (index == Personalizada) return;

    StaticModelSegmentator::Configuration conf = p->fijoConfs[static_cast<FijoConf>(index)];

    ui->spnFijoContraste->setValue(conf.alpha);
    ui->spnFijoUmbral->setValue(conf.thresh);
}
//-----------------------------------------------------------------
void MainWindow::on_chkDebug_stateChanged(int arg1) {
    ui->chkTime->setEnabled(arg1 != Qt::CheckState::Checked);
}
//-----------------------------------------------------------------
//----------------------------CREACION-----------------------------
//-----------------------------------------------------------------
void MainWindow::crear_analizadorGlobal() {
    qDebug() << "AnalizadorFrames";
    a = new GlobalSegmentator;
    GlobalSegmentator *analizador = dynamic_cast<GlobalSegmentator *>(a);

    GlobalSegmentator::ThreshType umb = static_cast<GlobalSegmentator::ThreshType>(ui->cboGlobalUmbr->currentIndex());
    double contraste = spanish->toDouble(ui->spnGlobalContraste->text());

    analizador->set_threshType(umb);
    qDebug() << "\tUmbralizado: "
             << QString::fromStdString(GlobalSegmentator::umbralizado_to_str(umb));
    analizador->set_alpha(contraste);
    qDebug() << "\tContraste: " << contraste;
}
//-----------------------------------------------------------------
void MainWindow::crear_analizadorSegmF() {
    qDebug() << "AnalizadorSegmFijo";
    int index = ui->cboFijoConf->currentIndex();

    if (index == Personalizada) {
        double contraste = spanish->toDouble(ui->spnFijoContraste->text());
        qDebug() << "\tContraste: " << contraste;
        double umbral = spanish->toDouble(ui->spnFijoUmbral->text());
        qDebug() << "\tUmbral: " << umbral;
        int index = ui->cboFijoModelo->currentIndex();

        a = new StaticModelSegmentator(contraste, umbral, static_cast<StaticModelSegmentator::Model>(index));
    } else {
        StaticModelSegmentator::Configuration conf = p->fijoConfs[static_cast<FijoConf>(index)];
        a = new StaticModelSegmentator(conf);
        qDebug() << "\tConfiguracion: " << QString::fromStdString(StaticModelSegmentator::conf_to_str(conf));
    }
}
//-----------------------------------------------------------------
void MainWindow::crear_analizadorSegmA() {
    qDebug() << "AnalizadorSegmAdapatativo";
    a = new AdaptiveModelSegmentator();
    AdaptiveModelSegmentator *analizador = dynamic_cast<AdaptiveModelSegmentator *>(a);
    double contraste = spanish->toDouble(ui->spnAdapContraste->text());

    analizador->set_alpha(contraste);
    qDebug() << "\tContraste: " << contraste;
}
//-----------------------------------------------------------------
void MainWindow::crear_analizadorBgS() {
    qDebug() << "AnalizadorBgSubtractor";
    a = new BgSubtractorSegemntator();
    BgSubtractorSegemntator *analizador = dynamic_cast<BgSubtractorSegemntator *>(a);

    BgSubtractorSegemntator::Algorithm alg = static_cast<BgSubtractorSegemntator::Algorithm>(ui->cboBgSAlg->currentIndex());
    qDebug() << "\tAlgoritmo de segmentación: "
             << QString::fromStdString(BgSubtractorSegemntator::algorithm_to_string(alg));
    analizador->set_alg(alg);
}
