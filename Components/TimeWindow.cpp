//
//    TimeWindow.cpp: Time Window for time view operations
//    Copyright (C) 2020 Gonzalo José Carracedo Carballal
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as
//    published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but
//    WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this program.  If not, see
//    <http://www.gnu.org/licenses/>
//

#include <TimeWindow.h>
#include <QFileDialog>
#include <QMessageBox>
#include <Suscan/Library.h>
#include <sigutils/sampling.h>
#include <fstream>
#include <iomanip>
#include <SuWidgetsHelpers.h>
#include "ui_TimeWindow.h"
#include <climits>
#include <CarrierDetector.h>
#include <CarrierXlator.h>

using namespace SigDigger;

bool
TimeWindow::exportToMatlab(QString const &path, int start, int end)
{
  std::ofstream of(path.toStdString().c_str(), std::ofstream::binary);
  const SUCOMPLEX *data = this->getDisplayData();
  int length = static_cast<int>(this->getDisplayDataLength());

  if (!of.is_open())
    return false;

  of << "%\n";
  of << "% Time domain capture file generated by SigDigger\n";
  of << "%\n\n";

  of << "sampleRate = " << this->fs << ";\n";
  of << "deltaT = " << 1 / this->fs << ";\n";
  of << "X = [ ";

  of << std::setprecision(std::numeric_limits<float>::digits10);

  if (start < 0)
    start = 0;
  if (end >= length)
    end = length - 1;

  for (int i = start; i <= end; ++i)
    of << SU_C_REAL(data[i]) << " + " << SU_C_IMAG(data[i]) << "i, ";

  of << "];\n";

  return true;
}

bool
TimeWindow::exportToWav(QString const &path, int start, int end)
{
  SF_INFO sfinfo;
  SNDFILE *sfp = nullptr;
  const SUCOMPLEX *data = this->getDisplayData();
  int length = static_cast<int>(this->getDisplayDataLength());
  bool ok = false;

  sfinfo.channels = 2;
  sfinfo.samplerate = static_cast<int>(this->ui->realWaveform->getSampleRate());
  sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

  if ((sfp = sf_open(path.toStdString().c_str(), SFM_WRITE, &sfinfo)) == nullptr)
    goto done;

  if (start < 0)
    start = 0;
  if (end > length)
    end = length;

  length = end - start;

  ok = sf_write_float(
        sfp,
        reinterpret_cast<const SUFLOAT *>(data + start),
        length << 1) == (length << 1);

done:
  if (sfp != nullptr)
    sf_close(sfp);

  return ok;
}

void
TimeWindow::connectAll(void)
{
  connect(
        this->ui->realWaveform,
        SIGNAL(horizontalRangeChanged(qint64, qint64)),
        this,
        SLOT(onHZoom(qint64, qint64)));

  connect(
        this->ui->realWaveform,
        SIGNAL(horizontalSelectionChanged(qreal, qreal)),
        this,
        SLOT(onHSelection(qreal, qreal)));

  connect(
        this->ui->imagWaveform,
        SIGNAL(horizontalRangeChanged(qint64, qint64)),
        this,
        SLOT(onHZoom(qint64, qint64)));

  connect(
        this->ui->imagWaveform,
        SIGNAL(horizontalSelectionChanged(qreal, qreal)),
        this,
        SLOT(onHSelection(qreal, qreal)));

  connect(
        this->ui->realWaveform,
        SIGNAL(hoverTime(qreal)),
        this,
        SLOT(onHoverTime(qreal)));

  connect(
        this->ui->imagWaveform,
        SIGNAL(hoverTime(qreal)),
        this,
        SLOT(onHoverTime(qreal)));

  connect(
        this->ui->actionSave,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onSaveAll(void)));

  connect(
        this->ui->actionSave_selection,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onSaveSelection(void)));

  connect(
        this->ui->actionFit_to_gain,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onFit(void)));

#if 0
  connect(
        this->ui->actionHorizontal_selection,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onToggleHorizontalSelection(void)));

  connect(
        this->ui->actionVertical_selection,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onToggleVerticalSelection(void)));
#endif

  connect(
        this->ui->actionZoom_selection,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onZoomToSelection(void)));

  connect(
        this->ui->actionResetZoom,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onZoomReset(void)));

  connect(
        this->ui->actionShowWaveform,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onShowWaveform(void)));

  connect(
        this->ui->actionShowEnvelope,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onShowEnvelope(void)));

  connect(
        this->ui->actionShowPhase,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onShowPhase(void)));

  connect(
        this->ui->actionPhaseDerivative,
        SIGNAL(triggered(bool)),
        this,
        SLOT(onPhaseDerivative(void)));

  connect(
        this->ui->periodicSelectionCheck,
        SIGNAL(stateChanged(int)),
        this,
        SLOT(onTogglePeriodicSelection(void)));

  connect(
        this->ui->periodicDivisionsSpin,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(onPeriodicDivisionsChanged(void)));

  connect(
        this->ui->paletteCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onPaletteChanged(int)));

  connect(
        this->ui->offsetSlider,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(onChangePaletteOffset(int)));

  connect(
        &this->taskController,
        SIGNAL(cancelling(void)),
        this,
        SLOT(onTaskCancelling(void)));

  connect(
        &this->taskController,
        SIGNAL(progress(qreal, QString)),
        this,
        SLOT(onTaskProgress(qreal, QString)));

  connect(
        &this->taskController,
        SIGNAL(done(void)),
        this,
        SLOT(onTaskDone(void)));

  connect(
        &this->taskController,
        SIGNAL(cancelled(void)),
        this,
        SLOT(onTaskCancelled(void)));

  connect(
        &this->taskController,
        SIGNAL(error(QString)),
        this,
        SLOT(onTaskError(QString)));

  connect(
        this->ui->guessCarrierButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onGuessCarrier(void)));

  connect(
        this->ui->syncButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onSyncCarrier(void)));

  connect(
        this->ui->resetButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onResetCarrier(void)));


}

int
TimeWindow::getPeriodicDivision(void) const
{
  return this->ui->periodicDivisionsSpin->value();
}

const SUCOMPLEX *
TimeWindow::getDisplayData(void) const
{
  return this->displayData->data();
}

size_t
TimeWindow::getDisplayDataLength(void) const
{
  return this->displayData->size();
}

void
TimeWindow::kahanMeanAndRms(
    SUCOMPLEX *mean,
    SUFLOAT *rms,
    const SUCOMPLEX *data,
    int length)
{
  SUCOMPLEX meanSum = 0;
  SUCOMPLEX meanC   = 0;
  SUCOMPLEX meanY, meanT;

  SUFLOAT   rmsSum = 0;
  SUFLOAT   rmsC   = 0;
  SUFLOAT   rmsY, rmsT;

  for (int i = 0; i < length; ++i) {
    meanY = data[i] - meanC;
    rmsY  = SU_C_REAL(data[i] * SU_C_CONJ(data[i])) - rmsC;

    meanT = meanSum + meanY;
    rmsT  = rmsSum  + rmsY;

    meanC = (meanT - meanSum) - meanY;
    rmsC  = (rmsT  - rmsSum)  - rmsY;

    meanSum = meanT;
    rmsSum  = rmsT;
  }

  *mean = meanSum / SU_ASFLOAT(length);
  *rms  = SU_SQRT(rmsSum / length);
}

void
TimeWindow::calcLimits(
    SUCOMPLEX *oMin,
    SUCOMPLEX *oMax,
    const SUCOMPLEX *data,
    int length)
{
  SUFLOAT minReal = +std::numeric_limits<SUFLOAT>::infinity();
  SUFLOAT maxReal = -std::numeric_limits<SUFLOAT>::infinity();
  SUFLOAT minImag = minReal;
  SUFLOAT maxImag = maxReal;

  for (int i = 0; i < length; ++i) {
    if (SU_C_REAL(data[i]) < minReal)
      minReal = SU_C_REAL(data[i]);
    if (SU_C_IMAG(data[i]) < minImag)
      minImag = SU_C_IMAG(data[i]);

    if (SU_C_REAL(data[i]) > maxReal)
      maxReal = SU_C_REAL(data[i]);
    if (SU_C_IMAG(data[i]) > maxImag)
      maxImag = SU_C_IMAG(data[i]);
  }

  *oMin = minReal + I * minImag;
  *oMax = maxReal + I * maxImag;
}

void
TimeWindow::setPalette(std::string const &name)
{
  int ndx = 0;

  for (auto p : this->palettes) {
    if (p.getName() == name) {
      this->ui->paletteCombo->setCurrentIndex(ndx);
      this->onPaletteChanged(ndx);
      break;
    }
    ++ndx;
  }
}

void
TimeWindow::setPaletteOffset(unsigned int offset)
{
  if (offset > 255)
    offset = 255;
  this->ui->offsetSlider->setValue(static_cast<int>(offset));
  this->onChangePaletteOffset(static_cast<int>(offset));
}

void
TimeWindow::setColorConfig(ColorConfig const &cfg)
{
  this->ui->constellation->setBackgroundColor(cfg.constellationBackground);
  this->ui->constellation->setForegroundColor(cfg.constellationForeground);
  this->ui->constellation->setAxesColor(cfg.constellationAxes);

  this->ui->realWaveform->setBackgroundColor(cfg.spectrumBackground);
  this->ui->realWaveform->setForegroundColor(cfg.spectrumForeground);
  this->ui->realWaveform->setAxesColor(cfg.spectrumAxes);
  this->ui->realWaveform->setTextColor(cfg.spectrumText);
  this->ui->realWaveform->setSelectionColor(cfg.selection);

  this->ui->imagWaveform->setBackgroundColor(cfg.spectrumBackground);
  this->ui->imagWaveform->setForegroundColor(cfg.spectrumForeground);
  this->ui->imagWaveform->setAxesColor(cfg.spectrumAxes);
  this->ui->imagWaveform->setTextColor(cfg.spectrumText);
  this->ui->imagWaveform->setSelectionColor(cfg.selection);
}

std::string
TimeWindow::getPalette(void) const
{
  if (this->ui->paletteCombo->currentIndex() < 0
      || this->ui->paletteCombo->currentIndex() >=
      static_cast<int>(this->palettes.size()))
    return "Suscan";

  return this->palettes[this->ui->paletteCombo->currentIndex()].getName();
}

unsigned int
TimeWindow::getPaletteOffset(void) const
{
  return this->ui->offsetSlider->value();
}


void
TimeWindow::recalcLimits(void)
{
  const SUCOMPLEX *data = this->getDisplayData();
  int length = static_cast<int>(this->getDisplayDataLength());

  if (length == 0) {
    this->min = this->max = this->mean = this->rms = 0;
  } else {
    kahanMeanAndRms(&this->mean, &this->rms, data, length);
    calcLimits(&this->min, &this->max, data, length);
  }
}

void
TimeWindow::refreshUi(void)
{
  bool haveSelection = this->ui->realWaveform->getHorizontalSelectionPresent();
  this->ui->periodicDivisionsSpin->setEnabled(
        this->ui->periodicSelectionCheck->isChecked());
  this->ui->selStartLabel->setEnabled(haveSelection);
  this->ui->selEndLabel->setEnabled(haveSelection);
  this->ui->selLengthLabel->setEnabled(haveSelection);
  this->ui->periodLabel->setEnabled(haveSelection);
  this->ui->baudLabel->setEnabled(haveSelection);
  this->ui->actionSave_selection->setEnabled(haveSelection);
  this->ui->sampleRateLabel->setText(
        QString::number(static_cast<int>(
          this->ui->realWaveform->getSampleRate())));
}

void
TimeWindow::refreshMeasures(void)
{
  qreal selStart = 0;
  qreal selEnd   = 0;
  qreal deltaT = 1. / this->ui->realWaveform->getSampleRate();
  SUCOMPLEX min, max, mean;
  SUFLOAT rms;
  const SUCOMPLEX *data = this->getDisplayData();
  int length = static_cast<int>(this->getDisplayDataLength());

  if (this->ui->realWaveform->getHorizontalSelectionPresent()) {
    selStart = this->ui->realWaveform->getHorizontalSelectionStart();
    selEnd   = this->ui->realWaveform->getHorizontalSelectionEnd();

    if (selStart < 0)
      selStart = 0;
    if (selEnd > length)
      selEnd = length;
  }

  if (selEnd - selStart > 0) {
    qreal period =
        (selEnd - selStart) /
        (this->ui->periodicSelectionCheck->isChecked()
           ? this->getPeriodicDivision()
           : 1)
        * deltaT;
    qreal baud = 1 / period;

    kahanMeanAndRms(
          &mean,
          &rms,
          data + static_cast<qint64>(selStart),
          static_cast<int>(selEnd - selStart));
    calcLimits(
          &min,
          &max,
          data + static_cast<qint64>(selStart),
          static_cast<int>(selEnd - selStart));

    this->ui->periodLabel->setText(
          SuWidgetsHelpers::formatQuantity(period, "s"));
    this->ui->baudLabel->setText(SuWidgetsHelpers::formatReal(baud));
    this->ui->selStartLabel->setText(
          SuWidgetsHelpers::formatQuantity(
            this->ui->realWaveform->samp2t(selStart),
            "s")
          + " (" + SuWidgetsHelpers::formatReal(selStart) + ")");
    this->ui->selEndLabel->setText(
          SuWidgetsHelpers::formatQuantity(
            this->ui->realWaveform->samp2t(selEnd),
            "s")
          + " (" + SuWidgetsHelpers::formatReal(selEnd) + ")");
    this->ui->selLengthLabel->setText(
          SuWidgetsHelpers::formatQuantity(
            (selEnd - selStart) * deltaT,
            "s")
          + " (" + SuWidgetsHelpers::formatReal(selEnd - selStart) + ")");
  } else {
    min = this->min;
    max = this->max;
    mean = this->mean;
    rms = this->rms;
    this->ui->periodLabel->setText("N/A");
    this->ui->baudLabel->setText("N/A");
    this->ui->selStartLabel->setText("N/A");
    this->ui->selEndLabel->setText("N/A");
    this->ui->selLengthLabel->setText("N/A");
  }

  this->ui->lengthLabel->setText(QString::number(length) + " samples");

  this->ui->durationLabel->setText(
        SuWidgetsHelpers::formatQuantity(length * deltaT, "s"));

  this->ui->minILabel->setText(
        SuWidgetsHelpers::formatScientific(SU_C_REAL(min)));

  this->ui->maxILabel->setText(
        SuWidgetsHelpers::formatScientific(SU_C_REAL(min)));

  this->ui->meanILabel->setText(
        SuWidgetsHelpers::formatScientific(SU_C_REAL(mean)));

  this->ui->minQLabel->setText(
        SuWidgetsHelpers::formatScientific(SU_C_IMAG(min)));

  this->ui->maxQLabel->setText(
        SuWidgetsHelpers::formatScientific(SU_C_IMAG(min)));

  this->ui->meanQLabel->setText(
        SuWidgetsHelpers::formatScientific(SU_C_IMAG(mean)));

  this->ui->rmsLabel->setText(
        SuWidgetsHelpers::formatReal(rms));
}

void
TimeWindow::deserializePalettes(void)
{
  Suscan::Singleton *sus = Suscan::Singleton::get_instance();
  int ndx = 0;

  // Fill palette vector
  for (auto i = sus->getFirstPalette();
       i != sus->getLastPalette();
       i++)
    this->palettes.push_back(Palette(*i));

  this->ui->paletteCombo->clear();

  // Populate combo
  for (auto p : this->palettes) {
    this->ui->paletteCombo->insertItem(
          ndx,
          QIcon(QPixmap::fromImage(p.getThumbnail())),
          QString::fromStdString(p.getName()),
          QVariant::fromValue(ndx));
    ++ndx;
  }

  this->onPaletteChanged(0);
}

void
TimeWindow::setCenterFreq(SUFREQ center)
{
  this->centerFreq = center;
  this->ui->centerFreqLabel->setText(
        SuWidgetsHelpers::formatIntegerPart(center) + " Hz");

  this->ui->refFreqSpin->setValue(center);
}

void
TimeWindow::setDisplayData(
    std::vector<SUCOMPLEX> const *displayData,
    bool keepView)
{
  this->displayData = displayData;

  this->ui->realWaveform->setData(displayData, keepView);
  this->ui->imagWaveform->setData(displayData, keepView);

  this->recalcLimits();

  this->refreshUi();
  this->refreshMeasures();
}

void
TimeWindow::setData(std::vector<SUCOMPLEX> const &data, qreal fs)
{
  this->fs = fs;

  this->ui->syncFreqSpin->setMinimum(-this->fs / 2);
  this->ui->syncFreqSpin->setMaximum(this->fs / 2);
  this->ui->realWaveform->setSampleRate(fs);
  this->ui->imagWaveform->setSampleRate(fs);

  this->data = &data;
  this->setDisplayData(&data);
}

TimeWindow::TimeWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::TimeWindow)
{
  ui->setupUi(this);

  this->ui->realWaveform->setRealComponent(true);
  this->ui->imagWaveform->setRealComponent(false);

  this->ui->syncFreqSpin->setExtraDecimals(6);

  this->recalcLimits();

  this->refreshUi();
  this->refreshMeasures();
  this->deserializePalettes();
  this->connectAll();
}

TimeWindow::~TimeWindow()
{
  delete ui;
}

//////////////////////////////////// Slots /////////////////////////////////////
void
TimeWindow::onHZoom(qint64 min, qint64 max)
{
  QObject* obj = sender();

  if (!this->adjusting) {
    Waveform *wf = nullptr;
    this->adjusting = true;

    if (obj == this->ui->realWaveform)
      wf = this->ui->imagWaveform;
    else
      wf = this->ui->realWaveform;

    wf->zoomHorizontal(min, max);
    wf->invalidate();
    this->adjusting = false;
  }
}

void
TimeWindow::onVZoom(qreal, qreal)
{
  // QObject* obj = sender();
}

void
TimeWindow::onHSelection(qreal min, qreal max)
{
  QObject *obj = sender();

  if (!this->adjusting) {
    Waveform *curr = static_cast<Waveform *>(obj);
    Waveform *wf;
    this->adjusting = true;

    if (obj == this->ui->realWaveform)
      wf = this->ui->imagWaveform;
    else
      wf = this->ui->realWaveform;

    if (curr->getHorizontalSelectionPresent())
      wf->selectHorizontal(min, max);
    else
      wf->selectHorizontal(0, 0);

    this->refreshUi();
    this->refreshMeasures();
    wf->invalidate();

    this->adjusting = false;
  }
}

void
TimeWindow::onVSelection(qreal, qreal)
{
  // QObject* obj = sender();
}

void
TimeWindow::onHoverTime(qreal time)
{
  const SUCOMPLEX *data = this->getDisplayData();
  int length = static_cast<int>(this->getDisplayDataLength());
  qreal samp = this->ui->realWaveform->t2samp(time);
  qint64 iSamp = static_cast<qint64>(std::floor(samp));
  qint64 selStart = 0, selEnd = 0, selLen = 0;
  qreal max = std::max<qreal>(
        std::max<qreal>(
          std::fabs(this->ui->realWaveform->getMax()),
          std::fabs(this->ui->realWaveform->getMin())),
        std::max<qreal>(
          std::fabs(this->ui->imagWaveform->getMax()),
          std::fabs(this->ui->imagWaveform->getMin())));

  qreal ampl = 1;
  if (max > 0)
    ampl = 1. / max;

  if (iSamp < 0)
    samp = iSamp = 0;
  if (iSamp > length)
    samp = iSamp = length - 1;

  SUFLOAT t = static_cast<SUFLOAT>(samp - iSamp);
  SUCOMPLEX val = (1 - t) * data[iSamp] + t * data[iSamp + 1];

  this->ui->constellation->setGain(ampl);

  if (this->ui->realWaveform->getHorizontalSelectionPresent()) {
     selStart = static_cast<qint64>(
          this->ui->realWaveform->getHorizontalSelectionStart());
    selEnd = static_cast<qint64>(
          this->ui->realWaveform->getHorizontalSelectionEnd());

    if (selStart < 0)
      selStart = 0;
    if (selEnd > length)
      selEnd = length;

    if (selEnd - selStart > TIME_WINDOW_MAX_SELECTION)
      selStart = selEnd - TIME_WINDOW_MAX_SELECTION;

    selLen = selEnd - selStart;

    if (selLen > 0) {
      this->ui->constellation->setHistorySize(
            static_cast<unsigned int>(selLen));
      this->ui->constellation->feed(
            data + selStart,
            static_cast<unsigned int>(selLen));
    }
  } else {
    if (iSamp == length - 1) {
      this->ui->constellation->setHistorySize(1);
      this->ui->constellation->feed(data + iSamp, 1);
    } else if (iSamp >= 0 && iSamp < length - 1) {
      this->ui->constellation->setHistorySize(1);
      this->ui->constellation->feed(&val, 1);
    } else {
      this->ui->constellation->setHistorySize(0);
    }
  }

  this->ui->positionLabel->setText(
        SuWidgetsHelpers::formatQuantity(time, "s")
        + " (" + SuWidgetsHelpers::formatReal(samp) + ")");
  this->ui->iLabel->setText(SuWidgetsHelpers::formatScientific(SU_C_REAL(val)));
  this->ui->qLabel->setText(SuWidgetsHelpers::formatScientific(SU_C_IMAG(val)));
  this->ui->magPhaseLabel->setText(
        SuWidgetsHelpers::formatReal(SU_C_ABS(val))
        + "("
        + SuWidgetsHelpers::formatReal(SU_C_ARG(val) / M_PI * 180)
        + "º)");

  // Frequency calculations
  qint64 dopplerLen = this->ui->realWaveform->getHorizontalSelectionPresent()
      ? selLen
      : static_cast<qint64>(
          std::ceil(this->ui->realWaveform->getSamplesPerPixel()));
  qint64 dopplerStart = this->ui->realWaveform->getHorizontalSelectionPresent()
      ? selStart
      : iSamp;
  qint64 delta = 1;
  qreal omegaAccum = 0;
  unsigned int count = 0;

  if (dopplerLen > TIME_WINDOW_MAX_DOPPLER_ITERS) {
    delta = dopplerLen / TIME_WINDOW_MAX_DOPPLER_ITERS;
    dopplerLen = TIME_WINDOW_MAX_DOPPLER_ITERS;
  }

  for (auto i = dopplerStart; i < dopplerLen + dopplerStart; i += delta) {
    if (i >= 1 && i < length) {
      omegaAccum += SU_C_ARG(data[i] * SU_C_CONJ(data[i - 1]));
      ++count;
    }
  }

  if (count > 0) {
    SUFLOAT normFreq;
    normFreq = SU_ANG2NORM_FREQ(omegaAccum / count);
    SUFREQ freq = SU_NORM2ABS_FREQ(this->fs, normFreq);
    SUFREQ ifFreq = this->ui->refFreqSpin->value() - this->centerFreq;
    SUFREQ doppler = -3e8 / this->centerFreq * (freq - ifFreq);
    this->ui->freqShiftLabel->setText(
          SuWidgetsHelpers::formatIntegerPart(freq) + " Hz");
    this->ui->dopplerShiftLabel->setText(
          SuWidgetsHelpers::formatQuantity(doppler, "m/s"));
  } else {
    this->ui->freqShiftLabel->setText("N/A");
    this->ui->dopplerShiftLabel->setText("N/A");
  }
}

void
TimeWindow::onTogglePeriodicSelection(void)
{
  this->ui->realWaveform->setPeriodicSelection(
        this->ui->periodicSelectionCheck->isChecked());
  this->ui->imagWaveform->setPeriodicSelection(
        this->ui->periodicSelectionCheck->isChecked());

  this->ui->realWaveform->invalidate();
  this->ui->imagWaveform->invalidate();

  this->refreshUi();
}

void
TimeWindow::onPeriodicDivisionsChanged(void)
{
  this->ui->realWaveform->setDivsPerSelection(
        this->getPeriodicDivision());
  this->ui->imagWaveform->setDivsPerSelection(
        this->getPeriodicDivision());

  this->ui->realWaveform->invalidate();
  this->ui->imagWaveform->invalidate();

  this->refreshMeasures();
}

void
TimeWindow::saveSamples(int start, int end)
{
  bool done = false;

  do {
    QFileDialog dialog(this);
    QStringList filters;

    dialog.setFileMode(QFileDialog::FileMode::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setWindowTitle(QString("Save capture"));

    filters << "MATLAB/Octave file (*.m)"
            << "Audio file (*.wav)";

    dialog.setNameFilters(filters);

    if (dialog.exec()) {
      QString path = dialog.selectedFiles().first();
      QString filter = dialog.selectedNameFilter();
      bool result;

      if (strstr(filter.toStdString().c_str(), ".m") != nullptr)  {
        path = SuWidgetsHelpers::ensureExtension(path, "m");
        result = this->exportToMatlab(path, start, end);
      } else {
        path = SuWidgetsHelpers::ensureExtension(path, "wav");
        result = this->exportToWav(path, start, end);
      }

      if (!result) {
        QMessageBox::warning(
              this,
              "Cannot open file",
              "Cannote save file in the specified location. Please choose "
              "a different location and try again.",
              QMessageBox::Ok);
      } else {
        done = true;
      }
    } else {
      done = true;
    }
  } while (!done);
}


void
TimeWindow::onSaveAll(void)
{
  this->saveSamples(
        0,
        static_cast<int>(this->getDisplayDataLength()));
}

void
TimeWindow::onSaveSelection(void)
{
  this->saveSamples(
        static_cast<int>(this->ui->realWaveform->getHorizontalSelectionStart()),
        static_cast<int>(this->ui->realWaveform->getHorizontalSelectionEnd()));
}

void
TimeWindow::onFit(void)
{
  this->ui->realWaveform->fitToEnvelope();
  this->ui->imagWaveform->fitToEnvelope();
  this->ui->realWaveform->invalidate();
  this->ui->imagWaveform->invalidate();
}

void
TimeWindow::onToggleAutoFit(void)
{

}

#if 0
void
TimeWindow::onToggleHorizontalSelection(void)
{
  if (!this->adjusting) {
    this->adjusting = true;
    this->ui->actionVertical_selection->setChecked(
          !this->ui->actionHorizontal_selection->isChecked());
    this->adjusting = false;
  }
}

void
TimeWindow::onToggleVerticalSelection(void)
{
  if (!this->adjusting) {
    this->adjusting = true;
    this->ui->actionHorizontal_selection->setChecked(
          !this->ui->actionVertical_selection->isChecked());
    this->adjusting = false;
  }
}
#endif
void
TimeWindow::onZoomToSelection(void)
{
  if (this->ui->realWaveform->getHorizontalSelectionPresent()) {
    this->ui->realWaveform->zoomHorizontal(
          static_cast<qint64>(
            this->ui->realWaveform->getHorizontalSelectionStart()),
          static_cast<qint64>(
            this->ui->realWaveform->getHorizontalSelectionEnd()));
    this->ui->imagWaveform->zoomHorizontal(
          static_cast<qint64>(
            this->ui->realWaveform->getHorizontalSelectionStart()),
          static_cast<qint64>(
            this->ui->realWaveform->getHorizontalSelectionEnd()));
    this->ui->realWaveform->invalidate();
    this->ui->imagWaveform->invalidate();
  }
}

void
TimeWindow::onZoomReset(void)
{
  // Should propagate to imaginary
  this->ui->realWaveform->zoomHorizontalReset();
  this->ui->realWaveform->zoomVerticalReset();

  this->ui->realWaveform->invalidate();
  this->ui->imagWaveform->invalidate();
}

void
TimeWindow::onShowWaveform(void)
{
  this->ui->realWaveform->setShowWaveform(
        this->ui->actionShowWaveform->isChecked());

  this->ui->imagWaveform->setShowWaveform(
        this->ui->actionShowWaveform->isChecked());
}

void
TimeWindow::onShowEnvelope(void)
{
  this->ui->realWaveform->setShowEnvelope(
        this->ui->actionShowEnvelope->isChecked());

  this->ui->imagWaveform->setShowEnvelope(
        this->ui->actionShowEnvelope->isChecked());

  this->ui->actionShowPhase->setEnabled(
        this->ui->actionShowEnvelope->isChecked());

  this->ui->actionPhaseDerivative->setEnabled(
        this->ui->actionShowEnvelope->isChecked());
}

void
TimeWindow::onShowPhase(void)
{
  this->ui->realWaveform->setShowPhase(this->ui->actionShowPhase->isChecked());
  this->ui->imagWaveform->setShowPhase(this->ui->actionShowPhase->isChecked());

  this->ui->actionPhaseDerivative->setEnabled(
        this->ui->actionShowPhase->isChecked());
}

void
TimeWindow::onPhaseDerivative(void)
{
  this->ui->realWaveform->setShowPhaseDiff(
        this->ui->actionPhaseDerivative->isChecked());

  this->ui->imagWaveform->setShowPhaseDiff(
        this->ui->actionPhaseDerivative->isChecked());
}

void
TimeWindow::onPaletteChanged(int index)
{
  this->ui->realWaveform->setPalette(
        this->palettes[static_cast<unsigned>(index)].getGradient());
  this->ui->imagWaveform->setPalette(
        this->palettes[static_cast<unsigned>(index)].getGradient());

  emit configChanged();
}

void
TimeWindow::onChangePaletteOffset(int val)
{
  this->ui->realWaveform->setPhaseDiffOrigin(static_cast<unsigned>(val));
  this->ui->imagWaveform->setPhaseDiffOrigin(static_cast<unsigned>(val));

  emit configChanged();
}

void
TimeWindow::onTaskCancelling(void)
{
  this->ui->taskProgressBar->setEnabled(false);
  this->ui->taskStateLabel->setText("Cancelling...");
}

void
TimeWindow::onTaskProgress(qreal progress, QString status)
{
  this->ui->taskStateLabel->setText(status);
  this->ui->taskProgressBar->setValue(static_cast<int>(progress * 100));
}

void
TimeWindow::onTaskDone(void)
{
  this->ui->taskStateLabel->setText("Done.");
  this->ui->taskProgressBar->setValue(0);

  if (this->taskController.getName() == "guessCarrier") {
    const CarrierDetector *cd =
        static_cast<const CarrierDetector *>(this->taskController.getTask());

    this->ui->syncFreqSpin->setValue(
          SU_NORM2ABS_FREQ(this->fs, SU_ANG2NORM_FREQ(cd->getPeak())));
  } else if (this->taskController.getName() == "xlateCarrier") {
    this->setDisplayData(&this->processedData, true);
  }
}

void
TimeWindow::onTaskCancelled(void)
{
  this->ui->taskProgressBar->setEnabled(true);
  this->ui->taskStateLabel->setText("Idle");
  this->ui->taskProgressBar->setValue(0);
}

void
TimeWindow::onTaskError(QString error)
{
  this->ui->taskStateLabel->setText("Idle");
  this->ui->taskProgressBar->setValue(0);
  QMessageBox::warning(this, "Background task failed", "Task failed: " + error);
}

void
TimeWindow::onGuessCarrier(void)
{
  if (this->ui->realWaveform->getHorizontalSelectionPresent()) {
    const SUCOMPLEX *data = this->getDisplayData();
    qint64 selStart = static_cast<qint64>(
          this->ui->realWaveform->getHorizontalSelectionStart());
    qint64 selEnd = static_cast<qint64>(
          this->ui->realWaveform->getHorizontalSelectionEnd());

    CarrierDetector *cd = new CarrierDetector(
          data + selStart,
          selEnd - selStart,
          this->ui->acdResolutionSpin->value(),
          this->ui->dcNotchSlider->value() / 100.);

    this->taskController.process("guessCarrier", cd);
  }
}

void
TimeWindow::onSyncCarrier(void)
{
  SUFLOAT relFreq = SU_ABS2NORM_FREQ(
        this->fs,
        this->ui->syncFreqSpin->value());

  this->processedData.resize(this->getDisplayDataLength());

  CarrierXlator *cx = new CarrierXlator(
        this->getDisplayData(),
        this->processedData.data(),
        this->getDisplayDataLength(),
        relFreq);

  this->taskController.process("xlateCarrier", cx);
}

void
TimeWindow::onResetCarrier(void)
{
  this->setDisplayData(this->data, true);
}
