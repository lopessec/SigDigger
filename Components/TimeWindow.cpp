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
#include <SigDiggerHelpers.h>
#include <climits>
#include <CarrierDetector.h>
#include <CarrierXlator.h>
#include <HistogramFeeder.h>
#include <DopplerCalculator.h>

#include "ui_TimeWindow.h"

using namespace SigDigger;

void
TimeWindow::connectFineTuneSelWidgets(void)
{
  connect(
        this->ui->selStartDecDeltaTButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onFineTuneSelectionClicked(void)));

  connect(
        this->ui->selStartDecSampleButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onFineTuneSelectionClicked(void)));

  connect(
        this->ui->selStartIncDeltaTButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onFineTuneSelectionClicked(void)));

  connect(
        this->ui->selStartIncSampleButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onFineTuneSelectionClicked(void)));

  connect(
        this->ui->selEndDecDeltaTButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onFineTuneSelectionClicked(void)));

  connect(
        this->ui->selEndDecSampleButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onFineTuneSelectionClicked(void)));

  connect(
        this->ui->selEndIncDeltaTButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onFineTuneSelectionClicked(void)));

  connect(
        this->ui->selEndIncSampleButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onFineTuneSelectionClicked(void)));
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
        this->ui->contrastSlider,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(onChangePaletteContrast(int)));

  connect(
        this->ui->taskAbortButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onAbort(void)));

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


  connect(
        this->ui->dcNotchSlider,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(onCarrierSlidersChanged(void)));

  connect(
        this->ui->averagerSlider,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(onCarrierSlidersChanged(void)));

  connect(
        this->ui->showHistogramButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onTriggerHistogram(void)));

  connect(
        this->histogramDialog,
        SIGNAL(blanked(void)),
        this,
        SLOT(onHistogramBlanked(void)));

  connect(
        this->ui->startSamplinButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onTriggerSampler(void)));

  connect(
        this->samplerDialog,
        SIGNAL(resample(void)),
        this,
        SLOT(onResample(void)));

  connect(
        this->samplerDialog,
        SIGNAL(stopTask(void)),
        this,
        SLOT(onAbort(void)));

  connect(
        this->histogramDialog,
        SIGNAL(stopTask(void)),
        this,
        SLOT(onAbort(void)));

  connect(
        this->ui->clckSourceBtnGrp,
        SIGNAL(buttonClicked(int)),
        this,
        SLOT(onClkSourceButtonClicked()));

  connect(
        this->ui->dopplerButton,
        SIGNAL(clicked(void)),
        this,
        SLOT(onCalculateDoppler(void)));

  connectFineTuneSelWidgets();
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
TimeWindow::showEvent(QShowEvent *)
{
  if (this->firstShow) {
    this->ui->dockWidget->setMinimumWidth(
          this->ui->measurementsGrid->sizeHint().width()
            + TIME_WINDOW_EXTRA_WIDTH);
    this->firstShow = false;
  }
}


void
TimeWindow::setPalette(std::string const &name)
{
  int index = SigDiggerHelpers::instance()->getPaletteIndex(name);

  if (index >= 0) {
    this->ui->paletteCombo->setCurrentIndex(index);
    this->onPaletteChanged(index);
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
TimeWindow::setPaletteContrast(int contrast)
{
  this->ui->contrastSlider->setValue(contrast);
  this->onChangePaletteContrast(contrast);
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

  this->histogramDialog->setColorConfig(cfg);
  this->samplerDialog->setColorConfig(cfg);
  this->dopplerDialog->setColorConfig(cfg);
}

std::string
TimeWindow::getPalette(void) const
{
  const Palette *palette = SigDiggerHelpers::instance()->getPalette(
        this->ui->paletteCombo->currentIndex());

  if (palette == nullptr)
    return "Suscan";

  return palette->getName();
}

unsigned int
TimeWindow::getPaletteOffset(void) const
{
  return static_cast<unsigned>(this->ui->offsetSlider->value());
}

int
TimeWindow::getPaletteContrast(void) const
{
  return this->ui->contrastSlider->value();
}

void
TimeWindow::fineTuneSelSetEnabled(bool enabled)
{
  this->ui->selStartButtonsWidget->setEnabled(enabled);
  this->ui->selEndButtonsWidget->setEnabled(enabled);
  this->ui->lockButton->setEnabled(enabled);
}

void
TimeWindow::fineTuneSelNotifySelection(bool sel)
{
  this->fineTuneSelSetEnabled(sel);
}

void
TimeWindow::carrierSyncSetEnabled(bool enabled)
{
  this->ui->carrierSyncPage->setEnabled(enabled);
}

void
TimeWindow::carrierSyncNotifySelection(bool selection)
{
  this->ui->guessCarrierButton->setEnabled(selection);
}

void
TimeWindow::samplingSetEnabled(bool enabled)
{
  this->ui->samplingPage->setEnabled(enabled);
}


void
TimeWindow::populateSamplingProperties(SamplingProperties &prop)
{
  bool haveSelection = this->ui->realWaveform->getHorizontalSelectionPresent();
  bool intSelection =
      haveSelection && this->ui->intSelectionButton->isChecked();
  qreal seconds;

  prop.fs = this->fs;
  prop.loopGain = 0;

  prop.sync = this->ui->clkGardnerButton->isChecked()
      ? SamplingClockSync::GARDNER
      : SamplingClockSync::MANUAL;

  if (this->ui->decAmplitudeButton->isChecked())
    prop.space = SamplingSpace::AMPLITUDE;
  else if (this->ui->decPhaseButton->isChecked())
    prop.space = SamplingSpace::PHASE;
  else if (this->ui->decFrequencyButton->isChecked())
    prop.space = SamplingSpace::FREQUENCY;

  if (intSelection) {
    size_t start = static_cast<size_t>(
          this->ui->realWaveform->getHorizontalSelectionStart());
    prop.data = this->getDisplayData() + start;
    prop.length =
        static_cast<size_t>(
          this->ui->realWaveform->getHorizontalSelectionEnd()
          - this->ui->realWaveform->getHorizontalSelectionStart());
    prop.symbolSync = start;
  } else {
    prop.data = this->getDisplayData();
    prop.length = this->getDisplayDataLength();
    prop.symbolSync = 0;
  }

  seconds = prop.length / this->fs;

  if (haveSelection && this->ui->clkSelectionButton->isChecked()) {
    if (intSelection) {
      // Interval is selection. Select all subdivisions
      prop.symbolCount = this->ui->periodicDivisionsSpin->value();
      prop.rate        = prop.symbolCount / seconds;
    } else {
      qreal selLength =
            this->ui->realWaveform->getHorizontalSelectionEnd()
            - this->ui->realWaveform->getHorizontalSelectionStart();

      // Compute deltaT based on selection and then the number of symbols
      // in the defined interval.
      qreal deltaT     = selLength / this->ui->periodicDivisionsSpin->value();
      prop.rate        = 1 / deltaT;
      prop.symbolCount = prop.length / deltaT;
    }
  } else if (this->ui->clkManualButton->isChecked()) {
    prop.rate        = this->ui->baudSpin->value();
    prop.symbolCount = seconds * prop.rate;
  } else if (this->ui->clkPartitionButton->isChecked()) {
    prop.symbolCount = this->ui->numSymSpin->value();
    prop.rate        = prop.symbolCount / seconds;
  } else {
    prop.rate        = this->ui->baudSpin->value();
    prop.loopGain       =
        static_cast<qreal>(
          SU_MAG_RAW(
            static_cast<SUFLOAT>(this->ui->clkGardnerLoopGain->value())));
  }
}

void
TimeWindow::samplingNotifySelection(bool selection, bool periodic)
{
  this->ui->intSelectionButton->setEnabled(selection);
  this->ui->clkSelectionButton->setEnabled(selection);

  if (!selection) {
    if (this->ui->intSelectionButton->isChecked())
      this->ui->intFullButton->setChecked(true);

    if (this->ui->clkSelectionButton->isChecked())
      this->ui->clkManualButton->setChecked(true);
  } else {
    this->ui->intSelectionButton->setChecked(true);

    if (periodic)
      this->ui->clkSelectionButton->setChecked(true);
  }
}

void
TimeWindow::notifyTaskRunning(bool running)
{
  this->ui->taskAbortButton->setEnabled(running);
  this->carrierSyncSetEnabled(!running);
  this->samplingSetEnabled(!running);
}

void
TimeWindow::recalcLimits(void)
{
  const SUCOMPLEX *data = this->getDisplayData();
  int length = static_cast<int>(this->getDisplayDataLength());

  if (length == 0) {
    this->min = this->max = this->mean = this->rms = 0;
  } else {
    SigDiggerHelpers::kahanMeanAndRms(&this->mean, &this->rms, data, length);
    SigDiggerHelpers::calcLimits(&this->min, &this->max, data, length);
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
  this->ui->dopplerButton->setEnabled(haveSelection);

  if (haveSelection != this->hadSelectionBefore) {
    this->carrierSyncNotifySelection(haveSelection);
    this->fineTuneSelNotifySelection(haveSelection);
    this->samplingNotifySelection(
          haveSelection,
          this->ui->periodicSelectionCheck->isChecked());
  }

  this->ui->sampleRateLabel->setText(
        QString::number(static_cast<int>(
          this->ui->realWaveform->getSampleRate())) +
        QStringLiteral(" sp/s"));

  this->ui->clkRateFrame->setEnabled(
        this->ui->clkManualButton->isChecked()
        || this->ui->clkGardnerButton->isChecked());
  this->ui->clkPartitionFrame->setEnabled(
        this->ui->clkPartitionButton->isChecked());
  this->ui->clkGardnerFrame->setEnabled(
        this->ui->clkGardnerButton->isChecked());

  if (this->ui->clkSelectionButton->isChecked()
      || this->ui->clkPartitionButton->isChecked()) {
    SamplingProperties sp;
    this->populateSamplingProperties(sp);
    this->ui->baudSpin->setValue((sp.symbolCount * this->fs) / sp.length);
  }

  this->hadSelectionBefore = haveSelection;
}

void
TimeWindow::startSampling(void)
{
  WaveSampler *ws = this->samplerDialog->makeSampler();

  connect(
        ws,
        SIGNAL(data(SigDigger::WaveSampleSet)),
        this,
        SLOT(onSampleSet(SigDigger::WaveSampleSet)));

  this->samplerDialog->show();
  this->notifyTaskRunning(true);
  this->taskController.process(QStringLiteral("triggerSampler"), ws);
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

    SigDiggerHelpers::kahanMeanAndRms(
          &mean,
          &rms,
          data + static_cast<qint64>(selStart),
          static_cast<int>(selEnd - selStart));
    SigDiggerHelpers::calcLimits(
          &min,
          &max,
          data + static_cast<qint64>(selStart),
          static_cast<int>(selEnd - selStart));

    this->ui->periodLabel->setText(
          SuWidgetsHelpers::formatQuantityFromDelta(
            period,
            deltaT,
            "s"));
    this->ui->baudLabel->setText(
      SuWidgetsHelpers::formatQuantity(
        baud,
        4,
        "Hz"));
    this->ui->selStartLabel->setText(
          SuWidgetsHelpers::formatQuantityFromDelta(
            this->ui->realWaveform->samp2t(selStart),
            deltaT,
            "s",
            true)
          + " (" + SuWidgetsHelpers::formatReal(selStart) + ")");
    this->ui->selEndLabel->setText(
          SuWidgetsHelpers::formatQuantityFromDelta(
            this->ui->realWaveform->samp2t(selEnd),
            deltaT,
            "s",
            true)
          + " (" + SuWidgetsHelpers::formatReal(selEnd) + ")");
    this->ui->selLengthLabel->setText(
          SuWidgetsHelpers::formatQuantityFromDelta(
            (selEnd - selStart) * deltaT,
            deltaT,
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
        SuWidgetsHelpers::formatQuantityFromDelta(
          length * deltaT,
          deltaT,
          "s"));

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

  if (displayData->size() == 0) {
    this->ui->realWaveform->setData(nullptr, false);
    this->ui->imagWaveform->setData(nullptr, false);
  } else {
    this->ui->realWaveform->setData(displayData, keepView);
    this->ui->imagWaveform->setData(displayData, keepView);
  }


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

  this->onCarrierSlidersChanged();
}

void
TimeWindow::adjustButtonToSize(QPushButton *button, QString text)
{
  if (text.size() == 0)
    text = button->text();

  button->setMaximumWidth(
        SuWidgetsHelpers::getWidgetTextWidth(button, text) +
        5 * SuWidgetsHelpers::getWidgetTextWidth(button, " "));
}

TimeWindow::TimeWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::TimeWindow)
{
  ui->setupUi(this);

  this->histogramDialog = new HistogramDialog(this);
  this->samplerDialog   = new SamplerDialog(this);
  this->dopplerDialog   = new DopplerDialog(this);

  // We can do this because both labels have the same font
  this->ui->notchWidthLabel->setFixedWidth(
        SuWidgetsHelpers::getWidgetTextWidth(
          this->ui->notchWidthLabel,
          QStringLiteral("XXXX.XX XHz")));

  this->ui->averagerSpanLabel->setFixedWidth(
        SuWidgetsHelpers::getWidgetTextWidth(
          this->ui->averagerSpanLabel,
          QStringLiteral("XXXX.XX XHz")));

  this->ui->realWaveform->setRealComponent(true);
  this->ui->imagWaveform->setRealComponent(false);

  this->ui->syncFreqSpin->setExtraDecimals(6);

#ifdef __APPLE__
  QFontMetrics m(this->ui->selStartButtonsWidget->font());
  // Fix Qt limitations in MacOS
  this->ui->selStartButtonsWidget->setMaximumHeight(7 * m.height() / 4);
  this->ui->selEndButtonsWidget->setMaximumHeight(7 * m.height() / 4);

  adjustButtonToSize(this->ui->selStartDecDeltaTButton, ">>");
  adjustButtonToSize(this->ui->selStartDecSampleButton, ">>");
  adjustButtonToSize(this->ui->selStartIncSampleButton, ">>");
  adjustButtonToSize(this->ui->selStartIncDeltaTButton, ">>");

  adjustButtonToSize(this->ui->selEndDecDeltaTButton, ">>");
  adjustButtonToSize(this->ui->selEndDecSampleButton, ">>");
  adjustButtonToSize(this->ui->selEndIncSampleButton, ">>");
  adjustButtonToSize(this->ui->selEndIncDeltaTButton, ">>");

  this->ui->gridLayout_9->setVerticalSpacing(6);
  this->ui->gridLayout_11->setVerticalSpacing(6);
  this->ui->gridLayout_12->setVerticalSpacing(6);
#endif

  this->recalcLimits();

  this->refreshUi();
  this->refreshMeasures();
  SigDiggerHelpers::instance()->populatePaletteCombo(this->ui->paletteCombo);
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
        SuWidgetsHelpers::formatQuantityFromDelta(
          time,
          1 / this->fs,
          "s",
          true)
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
    SUFREQ doppler = -TIME_WINDOW_SPEED_OF_LIGHT / this->centerFreq * (freq - ifFreq);
    this->ui->freqShiftLabel->setText(
          SuWidgetsHelpers::formatQuantity(
            freq,
            6,
            QStringLiteral("Hz"),
            true));
    this->ui->dopplerShiftLabel->setText(
          SuWidgetsHelpers::formatQuantity(
            doppler,
            5,
            QStringLiteral("m/s"),
            true));
  } else {
    this->ui->freqShiftLabel->setText(QStringLiteral("N/A"));
    this->ui->dopplerShiftLabel->setText(QStringLiteral("N/A"));
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
TimeWindow::onSaveAll(void)
{
  SigDiggerHelpers::openSaveSamplesDialog(
        this,
        this->getDisplayData(),
        this->getDisplayDataLength(),
        this->fs,
        0,
        static_cast<int>(this->getDisplayDataLength()),
        Suscan::Singleton::get_instance()->getBackgroundTaskController());
}

void
TimeWindow::onSaveSelection(void)
{
  SigDiggerHelpers::openSaveSamplesDialog(
        this,
        this->getDisplayData(),
        this->getDisplayDataLength(),
        this->fs,
        static_cast<int>(this->ui->realWaveform->getHorizontalSelectionStart()),
        static_cast<int>(this->ui->realWaveform->getHorizontalSelectionEnd()),
        Suscan::Singleton::get_instance()->getBackgroundTaskController());
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
TimeWindow::onAbort(void)
{
  this->taskController.cancel();
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
  const Palette *palette = SigDiggerHelpers::instance()->getPalette(index);

  if (palette != nullptr) {
    this->ui->realWaveform->setPalette(palette->getGradient());
    this->ui->imagWaveform->setPalette(palette->getGradient());
  }

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
TimeWindow::onChangePaletteContrast(int contrast)
{
  qreal realContrast = std::pow(
        static_cast<qreal>(10),
        static_cast<qreal>(contrast / 20.));

  this->ui->realWaveform->setPhaseDiffContrast(realContrast);
  this->ui->imagWaveform->setPhaseDiffContrast(realContrast);

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
    SUFLOAT relFreq = SU_ANG2NORM_FREQ(cd->getPeak());

    // Some UI feedback
    this->ui->syncFreqSpin->setValue(SU_NORM2ABS_FREQ(this->fs, relFreq));

    // Resize and process
    this->processedData.resize(this->getDisplayDataLength());

    CarrierXlator *cx = new CarrierXlator(
          this->getDisplayData(),
          this->processedData.data(),
          this->getDisplayDataLength(),
          relFreq);

    // Launch carrier translator
    this->taskController.process("xlateCarrier", cx);
  } else if (this->taskController.getName() == "xlateCarrier") {
    this->setDisplayData(&this->processedData, true);

    this->notifyTaskRunning(false);
  } else if (this->taskController.getName() == "triggerHistogram") {
    this->histogramDialog->show();
    this->notifyTaskRunning(false);
  } else if (this->taskController.getName() == "triggerSampler") {
    this->samplerDialog->show();
    this->notifyTaskRunning(false);
  } else if (this->taskController.getName() == "computeDoppler") {
    SUFLOAT lambda = static_cast<SUFLOAT>(299792458. / this->ui->refFreqSpin->value());
    // Oh my god. Please provide something better than this
    DopplerCalculator *dc =
        (DopplerCalculator *) this->taskController.getTask();
    std::vector<SUCOMPLEX> spectrum = std::move(dc->takeSpectrum());

    // If the selected wave was captured at a sample rate fs,
    // then the RBW is fs / data.size()
    // Therefore delta V is RBW * lambda

    this->notifyTaskRunning(false);

    this->dopplerDialog->setVelocityStep(this->fs / spectrum.size() * lambda);
    this->dopplerDialog->setSigmaV(static_cast<qreal>(dc->getSigma()));
    this->dopplerDialog->setCenterFreq(this->ui->refFreqSpin->value());
    this->dopplerDialog->setDominantVelocity(static_cast<qreal>(dc->getPeak()));
    this->dopplerDialog->giveSpectrum(std::move(spectrum));
    this->dopplerDialog->setMax(dc->getMax());
    this->dopplerDialog->show();
  }
}

void
TimeWindow::onTaskCancelled(void)
{
  this->ui->taskProgressBar->setEnabled(true);
  this->ui->taskStateLabel->setText("Idle");
  this->ui->taskProgressBar->setValue(0);

  this->notifyTaskRunning(false);
}

void
TimeWindow::onTaskError(QString error)
{
  this->ui->taskStateLabel->setText("Idle");
  this->ui->taskProgressBar->setValue(0);

  this->notifyTaskRunning(false);

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
          static_cast<qreal>(this->ui->averagerSlider->value())
          / static_cast<qreal>(this->ui->averagerSlider->maximum()),
          static_cast<qreal>(this->ui->dcNotchSlider->value())
          / static_cast<qreal>(this->ui->dcNotchSlider->maximum()));

    this->notifyTaskRunning(true);
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

  this->notifyTaskRunning(true);
  this->taskController.process("xlateCarrier", cx);
}

void
TimeWindow::onResetCarrier(void)
{
  this->setDisplayData(this->data, true);
  this->ui->syncFreqSpin->setValue(0);
}

void
TimeWindow::onCarrierSlidersChanged(void)
{
  qreal notchRelBw =
      static_cast<qreal>(this->ui->dcNotchSlider->value())
      / static_cast<qreal>(this->ui->dcNotchSlider->maximum());
  qreal avgRelBw =
      static_cast<qreal>(this->ui->averagerSlider->value())
      / static_cast<qreal>(this->ui->averagerSlider->maximum());

  this->ui->notchWidthLabel->setText(
        SuWidgetsHelpers::formatQuantity(
          this->fs * notchRelBw,
          6,
          "Hz"));

  this->ui->averagerSpanLabel->setText(
        SuWidgetsHelpers::formatQuantity(
          this->fs * avgRelBw,
          6,
          "Hz"));
}

void
TimeWindow::onHistogramSamples(const float *samples, unsigned int size)
{
  this->histogramDialog->feed(samples, size);
}

void
TimeWindow::onTriggerHistogram(void)
{
  SamplingProperties props;

  this->populateSamplingProperties(props);

  HistogramFeeder *hf = new HistogramFeeder(props);

  connect(
        hf,
        SIGNAL(data(const float *, unsigned int)),
        this,
        SLOT(onHistogramSamples(const float *, unsigned int)));

  this->histogramDialog->reset();
  this->histogramDialog->setProperties(props);
  this->histogramDialog->show();
  this->notifyTaskRunning(true);
  this->taskController.process("triggerHistogram", hf);
}

void
TimeWindow::onHistogramBlanked(void)
{
  if (this->histogramDialog->isVisible())
    this->onTriggerHistogram();
}

void
TimeWindow::onSampleSet(SigDigger::WaveSampleSet set)
{
  this->samplerDialog->feedSet(set);
}

void
TimeWindow::onTriggerSampler(void)
{
  SamplingProperties props;

  this->populateSamplingProperties(props);

  this->samplerDialog->reset();
  this->samplerDialog->setProperties(props);

  this->startSampling();
}

void
TimeWindow::onResample(void)
{
  if (this->samplerDialog->isVisible()) {
    this->samplerDialog->reset();
    this->startSampling();
  }
}

bool
TimeWindow::fineTuneSenderIs(const QPushButton *button) const
{
  QPushButton *sender = static_cast<QPushButton *>(this->sender());

  if (this->ui->lockButton->isChecked()) {
#define CHECKPAIR(a, b)                                   \
  if (button == this->ui->a || button == this->ui->b)     \
    return sender == this->ui->a || sender == this->ui->b

    CHECKPAIR(selStartIncDeltaTButton, selEndIncDeltaTButton);
    CHECKPAIR(selStartIncSampleButton, selEndIncSampleButton);
    CHECKPAIR(selStartDecDeltaTButton, selEndDecDeltaTButton);
    CHECKPAIR(selStartDecSampleButton, selEndDecSampleButton);
#undef CHECKPAIR
  }

  return button == sender;
}

void
TimeWindow::onFineTuneSelectionClicked(void)
{
  qint64 newSelStart =
      static_cast<qint64>(this->ui->realWaveform->getHorizontalSelectionStart());
  qint64 newSelEnd =
      static_cast<qint64>(this->ui->realWaveform->getHorizontalSelectionEnd());
  qint64 delta = newSelEnd - newSelStart;

#define CHECKBUTTON(btn) this->fineTuneSenderIs(this->ui->btn)

  if (CHECKBUTTON(selStartIncDeltaTButton))
    newSelStart += delta;

  if (CHECKBUTTON(selStartIncSampleButton))
    ++newSelStart;

  if (CHECKBUTTON(selStartDecDeltaTButton))
    newSelStart -= delta;

  if (CHECKBUTTON(selStartDecSampleButton))
    --newSelStart;

  if (CHECKBUTTON(selEndIncDeltaTButton))
    newSelEnd += delta;

  if (CHECKBUTTON(selEndIncSampleButton))
    ++newSelEnd;

  if (CHECKBUTTON(selEndDecDeltaTButton))
    newSelEnd -= delta;

  if (CHECKBUTTON(selEndDecSampleButton))
    --newSelEnd;

#undef CHECKBUTTON

  this->ui->imagWaveform->selectHorizontal(newSelStart, newSelEnd);
  this->ui->realWaveform->selectHorizontal(newSelStart, newSelEnd);
}

void
TimeWindow::onClkSourceButtonClicked(void)
{
  this->refreshUi();
}

void
TimeWindow::onCalculateDoppler(void)
{
  if (this->ui->realWaveform->getHorizontalSelectionPresent()) {
    const SUCOMPLEX *data = this->getDisplayData();
    qint64 selStart = static_cast<qint64>(
          this->ui->realWaveform->getHorizontalSelectionStart());
    qint64 selEnd = static_cast<qint64>(
          this->ui->realWaveform->getHorizontalSelectionEnd());

    if (selStart < 0)
      selStart = 0;

    if (selEnd >= static_cast<qint64>(this->data->size()))
      selEnd = static_cast<qint64>(this->data->size());

    DopplerCalculator *dc = new DopplerCalculator(
          this->ui->refFreqSpin->value(),
          data + selStart,
          static_cast<size_t>(selEnd - selStart),
          static_cast<SUFLOAT>(this->fs));

    this->notifyTaskRunning(true);
    this->taskController.process("computeDoppler", dc);
  }
}

