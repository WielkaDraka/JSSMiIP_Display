/*
 * ts.c
 */

#include "stm32f7xx_hal.h"
#include "LCD_Driver/ts.h"

static I2C_HandleTypeDef hI2cAudioHandler = {0};


static void I2Cx_MspInit(I2C_HandleTypeDef *i2c_handler)
{
  GPIO_InitTypeDef  gpio_init_structure;

    /* AUDIO and LCD I2C MSP init */

    /*** Configure the GPIOs ***/
    /* Enable GPIO clock */
    DISCOVERY_AUDIO_I2Cx_SCL_SDA_GPIO_CLK_ENABLE();

    /* Configure I2C Tx as alternate function */
    gpio_init_structure.Pin = DISCOVERY_AUDIO_I2Cx_SCL_PIN;
    gpio_init_structure.Mode = GPIO_MODE_AF_OD;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FAST;
    gpio_init_structure.Alternate = DISCOVERY_AUDIO_I2Cx_SCL_SDA_AF;
    HAL_GPIO_Init(DISCOVERY_AUDIO_I2Cx_SCL_SDA_GPIO_PORT, &gpio_init_structure);

    /* Configure I2C Rx as alternate function */
    gpio_init_structure.Pin = DISCOVERY_AUDIO_I2Cx_SDA_PIN;
    HAL_GPIO_Init(DISCOVERY_AUDIO_I2Cx_SCL_SDA_GPIO_PORT, &gpio_init_structure);

    /*** Configure the I2C peripheral ***/
    /* Enable I2C clock */
    DISCOVERY_AUDIO_I2Cx_CLK_ENABLE();

    /* Force the I2C peripheral clock reset */
    DISCOVERY_AUDIO_I2Cx_FORCE_RESET();

    /* Release the I2C peripheral clock reset */
    DISCOVERY_AUDIO_I2Cx_RELEASE_RESET();

    /* Enable and set I2Cx Interrupt to a lower priority */
    HAL_NVIC_SetPriority(DISCOVERY_AUDIO_I2Cx_EV_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(DISCOVERY_AUDIO_I2Cx_EV_IRQn);

    /* Enable and set I2Cx Interrupt to a lower priority */
    HAL_NVIC_SetPriority(DISCOVERY_AUDIO_I2Cx_ER_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(DISCOVERY_AUDIO_I2Cx_ER_IRQn);

}


static void I2Cx_Init(I2C_HandleTypeDef *i2c_handler)
{
  if(HAL_I2C_GetState(i2c_handler) == HAL_I2C_STATE_RESET)
  {
	 i2c_handler->Instance = DISCOVERY_AUDIO_I2Cx;

    i2c_handler->Init.Timing           = DISCOVERY_I2Cx_TIMING;
    i2c_handler->Init.OwnAddress1      = 0;
    i2c_handler->Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    i2c_handler->Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    i2c_handler->Init.OwnAddress2      = 0;
    i2c_handler->Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    i2c_handler->Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

    /* Init the I2C */
    I2Cx_MspInit(i2c_handler);
    HAL_I2C_Init(i2c_handler);
  }
}

static void I2Cx_Error(I2C_HandleTypeDef *i2c_handler, uint8_t Addr)
{
  /* De-initialize the I2C communication bus */
  HAL_I2C_DeInit(i2c_handler);

  /* Re-Initialize the I2C communication bus */
  I2Cx_Init(i2c_handler);
}

static HAL_StatusTypeDef I2Cx_WriteMultiple(I2C_HandleTypeDef *i2c_handler,
                                            uint8_t Addr,
                                            uint16_t Reg,
                                            uint16_t MemAddress,
                                            uint8_t *Buffer,
                                            uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_I2C_Mem_Write(i2c_handler, Addr, (uint16_t)Reg, MemAddress, Buffer, Length, 1000);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initiaize the I2C Bus */
    I2Cx_Error(i2c_handler, Addr);
  }
  return status;
}


static HAL_StatusTypeDef I2Cx_ReadMultiple(I2C_HandleTypeDef *i2c_handler,
                                           uint8_t Addr,
                                           uint16_t Reg,
                                           uint16_t MemAddress,
                                           uint8_t *Buffer,
                                           uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_I2C_Mem_Read(i2c_handler, Addr, (uint16_t)Reg, MemAddress, Buffer, Length, 1000);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* I2C error occurred */
    I2Cx_Error(i2c_handler, Addr);
  }
  return status;
}

/**
  * @brief  Initializes Touchscreen low level.
  * @retval None
  */
void TS_IO_Init(void)
{
  I2Cx_Init(&hI2cAudioHandler);
}

/**
  * @brief  Writes a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address
  * @param  Value: Data to be written
  * @retval None
  */
void TS_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
  I2Cx_WriteMultiple(&hI2cAudioHandler, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT,(uint8_t*)&Value, 1);
}

/**
  * @brief  Reads a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address
  * @retval Data to be read
  */
uint8_t TS_IO_Read(uint8_t Addr, uint8_t Reg)
{
  uint8_t read_value = 0;

  I2Cx_ReadMultiple(&hI2cAudioHandler, Addr, Reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&read_value, 1);

  return read_value;
}

/**
  * @brief  TS delay
  * @param  Delay: Delay in ms
  * @retval None
  */
void TS_IO_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}
