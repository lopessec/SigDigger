//
//    AudioMediator.cpp: Mediate audio panel events
//    Copyright (C) 2019 Gonzalo José Carracedo Carballal
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

#include "UIMediator.h"

using namespace SigDigger;

void
UIMediator::connectAudioPanel(void)
{
  connect(
        this->ui->audioPanel,
        SIGNAL(changed(void)),
        this,
        SLOT(onAudioChanged(void)));

  connect(
        this->ui->audioPanel,
        SIGNAL(volumeChanged(float)),
        this,
        SIGNAL(audioVolumeChanged(float)));

  connect(
        this->ui->audioPanel,
        SIGNAL(recordStateChanged(bool)),
        this,
        SIGNAL(audioRecordStateChanged(void)));
}

void
UIMediator::onAudioChanged(void)
{
  switch (this->ui->audioPanel->getDemod()) {
    case AM:
    case FM:
      this->ui->spectrum->setFilterSkewness(MainSpectrum::SYMMETRIC);
      break;

    case USB:
      this->ui->spectrum->setFilterSkewness(MainSpectrum::UPPER);
      break;

    case LSB:
      this->ui->spectrum->setFilterSkewness(MainSpectrum::LOWER);
      break;
  }

  emit audioChanged();
}
