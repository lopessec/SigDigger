//
//    Source.h: Signal source object
//    Copyright (C) 2018 Gonzalo José Carracedo Carballal
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

#ifndef CPP_SOURCE_H
#define CPP_SOURCE_H

#include <vector>

#include <Suscan/Compat.h>
#include <Suscan/Object.h>
#include <analyzer/source.h>

struct suscan_analyzer_gain_info;

namespace Suscan {
  class Source
  {
  private:
    suscan_source_config_t *config;
    suscan_source_t *instance; // TODO: Make it unique_ptr

    friend class Analyzer;

  public:
    class Config;
    class Device;
    class GainDescription;
    Source(Config const&);
    ~Source();
  };

  class Source::GainDescription {
    std::string name;
    SUFLOAT min;
    SUFLOAT max;
    SUFLOAT step;
    SUFLOAT def;

  public:
    GainDescription(const struct suscan_source_gain_desc *desc);
    GainDescription(const struct suscan_analyzer_gain_info *info);

    std::string
    getName(void) const
    {
      return this->name;
    }

    SUFLOAT
    getMin(void) const
    {
      return this->min;
    }

    SUFLOAT
    getMax(void) const
    {
      return this->max;
    }

    SUFLOAT
    getStep(void) const
    {
      return this->step;
    }

    SUFLOAT
    getDefault(void) const
    {
      return this->def;
    }
  };

  class Source::Device {
    suscan_source_device_t *owned = nullptr; // owned device
    const suscan_source_device_t *instance;  // Device pointer
    std::vector<std::string> antennas;
    std::vector<Source::GainDescription> gains;
    std::vector<double> rates;
    SUFREQ freqMin = 0;
    SUFREQ freqMax = 0;

    friend class Config;

  public:
    Device(); // Dummy constructor because Qt wants it
    Device(const Device &dev);
    Device(Device &&rv);
    Device(
        const std::string &name,
        const std::string &host,
        uint16_t port,
        const std::string &user = "",
        const std::string &password = "");
    Device(const suscan_source_device_t *dev, unsigned int channel);

    ~Device();
    void setDevice(const suscan_source_device_t *dev, unsigned int channel);

    Device &
    operator = (const Device &dev)
    {
      if (this != &dev) {
        if (dev.owned != nullptr) {
          SU_ATTEMPT(this->owned = suscan_source_device_dup(dev.owned));
          this->setDevice(this->owned, 0);
        } else {
          this->setDevice(dev.instance, 0);
        }
      }

      return *this;
    }

    Device &
    operator = (Device &&dev)
    {
      if (this != &dev) {
        std::swap(this->instance, dev.instance);
        std::swap(this->owned,    dev.owned);
        std::swap(this->antennas, dev.antennas);
        std::swap(this->rates,    dev.rates);
        std::swap(this->gains,    dev.gains);
      }
      return *this;
    }

    bool
    operator == (const Device &dev) const
    {
      return this->equals(dev);
    }

    const suscan_source_device_t *
    getInstance(void) const
    {
      return this->instance;
    }

    void
    setDevice(const Device &dev)
    {
      this->setDevice(dev.instance, 0);
    }

    bool
    equals(const Device &dev) const
    {
      return this->instance == dev.instance;
    }

    bool
    getParam(std::string const &key, std::string &val) const
    {
      const char *result;

      if (this->instance == nullptr)
        return false;


      if ((result = suscan_source_device_get_param(
             this->instance,
             key.c_str())) == nullptr)
        return false;

      val = result;

      return true;
    }

    const char *
    getParam(std::string const &key) const
    {
      if (this->instance == nullptr)
        return nullptr;

      return suscan_source_device_get_param(this->instance, key.c_str());
    }

    std::string
    getDriver(void) const
    {
      if (this->instance == nullptr)
        return "<Invalid device>";
      return suscan_source_device_get_driver(this->instance);
    }

    std::string
    getDesc(void) const
    {
      if (this->instance == nullptr)
        return "<Invalid device>";
      return suscan_source_device_get_desc(this->instance);
    }

    int
    getIndex(void) const
    {
      if (this->instance == nullptr)
        return -1;
      return suscan_source_device_get_index(this->instance);
    }

    bool
    isPopulated(void) const
    {
      if (this->instance == nullptr)
        return false;
      return suscan_source_device_is_populated(this->instance);
    }

    bool
    isRemote(void) const
    {
      if (this->instance == nullptr)
        return false;

      return suscan_source_device_is_remote(this->instance);
    }

    bool
    isAvailable(void) const
    {
      if (this->instance == nullptr)
        return false;
      return suscan_source_device_is_available(this->instance);
    }

    std::vector<std::string>::const_iterator
    getFirstAntenna(void) const
    {
      return this->antennas.begin();
    }

    std::vector<std::string>::const_iterator
    getLastAntenna(void) const
    {
      return this->antennas.end();
    }

    std::vector<std::string>::const_iterator
    findAntenna(std::string const &antenna) const
    {
      for (auto p = this->getFirstAntenna(); p != this->getLastAntenna(); ++p)
        if (*p == antenna)
          return p;

      return this->getLastAntenna();
    }

    std::vector<Source::GainDescription>::const_iterator
    getFirstGain(void) const
    {
      return this->gains.begin();
    }

    std::vector<Source::GainDescription>::const_iterator
    getLastGain(void) const
    {
      return this->gains.end();
    }

    std::vector<double>::const_iterator
    getFirstSampRate(void) const
    {
      return this->rates.begin();
    }

    std::vector<double>::const_iterator
    getLastSampRate(void) const
    {
      return this->rates.end();
    }

    SUFREQ
    getMinFreq(void) const
    {
      return this->freqMin;
    }

    SUFREQ
    getMaxFreq(void) const
    {
      return this->freqMax;
    }
  };

  class Source::Config {
  private:
    bool borrowed;
    Device devWrapper;

    // Convenience friendship
    friend class Analyzer;
    friend class Source;

  public:
    suscan_source_config_t *instance;
    std::string label(void) const;
    SUFREQ getFreq(void) const;
    SUFREQ getLnbFreq(void) const;
    unsigned int getSampleRate(void) const;
    unsigned int getDecimatedSampleRate(void) const;
    unsigned int getDecimation(void) const;
    enum suscan_source_type getType(void) const;
    bool getLoop(void) const;
    std::string getPath(void) const;
    std::string getAntenna(void) const;
    bool getDCRemove(void) const;
    bool getIQBalance(void) const;
    std::string getInterface(void) const;
    SUFLOAT getBandwidth(void) const;
    SUFLOAT getGain(const std::string &) const;
    std::string getParam(const std::string &key) const;
    SUFLOAT getPPM(void) const;

    const Source::Device &getDevice(void);
    enum suscan_source_format getFormat(void) const;

    void setFreq(SUFREQ freq);
    void setLnbFreq(SUFREQ freq);
    void setBandwidth(SUFLOAT bw);
    void setLoop(bool);
    void setDCRemove(bool);
    void setIQBalance(bool);
    void setFormat(enum suscan_source_format fmt);
    void setType(enum suscan_source_type type);
    void setLabel(const std::string &);
    void setPath(const std::string &);
    void setSampleRate(unsigned int value);
    void setDecimation(unsigned int);
    void setDevice(const Source::Device &dev);
    void setGain(const std::string &, SUFLOAT);
    void setAntenna(const std::string &);    
    void setInterface(std::string const &interface);
    void setParam(std::string const &key, std::string const &param);
    void setPPM(SUFLOAT);

    Config& operator=(const Config &);
    Config& operator=(Config &&);

    Suscan::Object serialize(void);

    Config();
    Config(const Suscan::Object &obj);
    Config(suscan_source_config_t *);
    Config(const Config &);
    Config(Config &&);
    Config(enum suscan_source_type type, enum suscan_source_format format);
    ~Config();

    static Source::Config wrap(suscan_source_config_t *config);
  };
};

#endif // CPP_SOURCE_H
