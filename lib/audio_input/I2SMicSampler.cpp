#include "Arduino.h"
#include "I2SMicSampler.h"
#include "driver/i2s.h"
#include "soc/i2s_reg.h"

I2SMicSampler::I2SMicSampler(i2s_pin_config_t &i2sPins, bool fixSPH0645) : I2SSampler()
{
    m_i2sPins = i2sPins;
    m_fixSPH0645 = fixSPH0645;
}

void I2SMicSampler::configureI2S()
{
    if (m_fixSPH0645)
    {
        // FIXES for SPH0645
        REG_SET_BIT(I2S_TIMING_REG(getI2SPort()), BIT(9));
        REG_SET_BIT(I2S_CONF_REG(getI2SPort()), I2S_RX_MSB_SHIFT);
    }

    i2s_set_pin(getI2SPort(), &m_i2sPins);
}

void I2SMicSampler::processI2SData(uint8_t *i2sData, size_t bytesRead)
{
    int32_t *samples = (int32_t *)i2sData;
    for (int i = 0; i < bytesRead / 4; i++)
    {
        addSample(samples[i] >> 11);
    }
}
