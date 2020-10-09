/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  *
  * A simple project for displaying strings/bitmaps received via the Bluetooth
  * (UART) HC08 module onto the SSD1306 (I2C) display
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "font8x8.h"
#include "SSD1306Commands.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */



//Init UART buffers/lines
#define buffer_size  				128
#define line_size  					32

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */
static uint8_t SSD1306_Buffer[COLUMNS * ROWS / 8]; //SSD1306 uses 8 bit addressing to change pixels
//- means no updating singular pixels unless you use a buffer

uint8_t currentPage = 0;
uint8_t currentColumn = 0;

uint8_t data;

uint8_t _readOpcode = SSD1306_DEF_SA | 0x01;   // contains the I2C address of the device for reading
uint8_t _writeOpcode = SSD1306_DEF_SA & 0xFE;  // contains the I2C address of the device for writing

bool readFlag = false;
bool bitmapFlag = false;
uint8_t requestPixel = 0x06;
int currentXPos = 0;
int currentYPos = 0;

char rx_buffer[buffer_size];
uint8_t rx_char;
char rx_line[line_size];

//Used for interrupt
volatile int rx_in=0;
volatile int rx_out=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/**
 * @brief Writes a single command to the command register of SSD1306
 * @param uint8_t command (command to be written)
 * @retval None
 */
void SSD1306_WriteSingleCommand(uint8_t command){
	HAL_StatusTypeDef ret;

	ret = HAL_I2C_Mem_Write(&hi2c1, SSD1306_DEF_SA, 0x00, 1, &command, 1, HAL_MAX_DELAY);

}

/**
 * @brief Writes a multiple commands (or commands with parameters) to the multiple command register of SSD1306
 * @param uint8_t* commands (pointer to commands/parameters to be written)
 * @retval None
 */
void SSD1306_WriteMultiCommands(uint8_t* commands){

	HAL_I2C_Mem_Write(&hi2c1,  SSD1306_DEF_SA, 0x80, 1, commands, sizeof(commands), HAL_MAX_DELAY);

}

/**
 * @brief Writes data to the data address of SSD1306 - must write 8 bits/8 pixels at a time
 * @param uint8_t* data - pointer to data to be written, uint16_t dataSize
 * @retval None
 */
void SSD1306_WriteData(uint8_t* data, uint16_t dataSize){

	HAL_I2C_Mem_Write(&hi2c1, SSD1306_DEF_SA, DATA_MODE, 1, data, dataSize, HAL_MAX_DELAY);
}

/**
 * @brief Writes data to the multiple data address of SSD1306 - must write 8 bits/8 pixels at a time
 * @param uint8_t* data - pointer to data to be written, uint16_t dataSize
 * @retval None
 */
void SSD1306_WriteMultiData(uint8_t* data, uint16_t dataSize){

	HAL_I2C_Mem_Write(&hi2c1, SSD1306_DEF_SA, 0xC0, 1, data, dataSize, HAL_MAX_DELAY);
}

/**
 * @brief Sets memory addressing mode (Horizontal, Vertical, Page) on the SSD1306
 * @param uint8_t mode - mode to be written (See SSD1306Commands.h for values)
 * @retval None
 */
void SSD1306_SetMemoryAddressingMode(uint8_t mode){

	uint8_t commands[2] = {SET_MEMORY_ADDRESSING_MODE, mode};

	SSD1306_WriteSingleCommand(commands[0]);
	SSD1306_WriteSingleCommand(commands[1]);

	//SSD1306_WriteMultiCommands(commands);
}

/**
 * @brief Sets the current page address start and end for data to be written (Min 0, Max 7)
 * @param uint8_t start, end - start and end page addresses
 * @retval None
 */
void SSD1306_SetPageAddress(uint8_t start, uint8_t end){

	uint8_t commands[3] = {SET_PAGE_ADDRESS, start, end};

	SSD1306_WriteSingleCommand(commands[0]);
	SSD1306_WriteSingleCommand(commands[1]);
	SSD1306_WriteSingleCommand(commands[2]);

	//SSD1306_WriteMultiCommands(commands);
}

/**
 * @brief Sets the current column address start and end for data to be written (Min 0, Max 127)
 * @param uint8_t start, end - start and end column addresses
 * @retval None
 */
void SSD1306_SetColumnAddress(uint8_t start, uint8_t end){

	uint8_t commands[3] = {SET_COLUMN_ADDRESS, start, end};

	SSD1306_WriteSingleCommand(commands[0]);
	SSD1306_WriteSingleCommand(commands[1]);
	SSD1306_WriteSingleCommand(commands[2]);

	//SSD1306_WriteMultiCommands(commands);

}

/**
 * @brief Sets the current row address start and end for data to be written (Min 0, Max 127)
 * @param uint8_t row, row address for data to be written to
 * @retval None
 */
void SSD1306_SetRowAddress(uint8_t row){

	uint8_t commands[2] = {SET_DISPLAY_START_LINE, row};

	SSD1306_WriteMultiCommands(commands);
}

/**
 * @brief Sets the contrast on SSD1306
 * @param uint8_t contrastValue - value between 0-255 (0x00 - 0xFF)
 * @retval None
 */
void SSD1306_WriteContrast(uint8_t contrastValue){

	SSD1306_WriteSingleCommand(SET_CONTRAST);
	SSD1306_WriteSingleCommand(contrastValue);
}

/**
 * @brief UNUSED
 * @param
 * @retval None
 */
void SSD1306_UpdateScreenFromBuffer(){
}

/**
 * @brief Clears the SSD1306_Buffer (resets all pixels to 0), best to call SSD1306_UpdateScreen after
 * @param None
 * @retval None
 */
void clearBuffer(){

	for(int i = 0; i < sizeof(SSD1306_Buffer); i++) {
	        SSD1306_Buffer[i] = 0;
	    }
}

/**
 * @brief Fills the SSD1306_Buffer (resets all pixels to 1), best to call SSD1306_UpdateScreen after
 * @param None
 * @retval None
 */
void fillBuffer(){

	for (int i = 0; i < sizeof(SSD1306_Buffer); i++) {
		        SSD1306_Buffer[i] = 1;
		    }
}

/**
 * @brief Clears the screen by first clearing the SSD1306_Buffer and then updating the screen from the buffer
 * @param None
 * @retval None
 */
void SSD1306_ClearScreen(){

	SSD1306_SetMemoryAddressingMode(HORIZONTAL_ADDRESSING_MODE);

	SSD1306_SetPageAddress(0, MAX_PAGE);

	SSD1306_SetColumnAddress(0, MAX_COLUMN);

	clearBuffer();

	for (uint8_t j = 0; j <= MAX_PAGE; j++){

		SSD1306_WriteData(&SSD1306_Buffer[COLUMNS*j],COLUMNS);

	}

	currentColumn = 0;
	currentPage = 0;

}

/**
 * @brief Clears the screen by writing empty data (uint8_t 0x00) to the data register of SSD1306
 * @param None
 * @retval None
 */
void SSD1306_ClearScreenAddressing(){
	SSD1306_SetMemoryAddressingMode(HORIZONTAL_ADDRESSING_MODE);

	SSD1306_SetPageAddress(0, MAX_PAGE);

	SSD1306_SetColumnAddress(0, MAX_COLUMN);

	uint8_t emptyPage [1024] = {0};

	SSD1306_WriteData(emptyPage, 1024);

	currentColumn = 0;
	currentPage = 0;

}

/**
 * @brief Writes a single 8x8 Character to the requested page and column of SSD1306
 * @param uint8_t character, page, column - character to be written, page to be written to (0-7), column to be written to (0-127)
 * @retval 0,1 depending on success
 */
int SSD1306_WriteChar(uint8_t character, uint8_t page, uint8_t column){

	//Check if character is within limit of font chars
	if (character < 32 || character > 126){return 0;}

	SSD1306_SetMemoryAddressingMode(HORIZONTAL_ADDRESSING_MODE);

	SSD1306_SetPageAddress(page, MAX_PAGE);

	SSD1306_SetColumnAddress(column, MAX_COLUMN);

	SSD1306_WriteData(Font8x8[character - 32], 8);

	return(1);

}

/**
 * @brief Writes a string in 8x8 Font to the SSD1306, splits string by spaces and checks string will fit on remaining space
 * @brief Uses variables currentColumn and currentPage to select address
 * @param uint8_t character, page, column - character to be written, page to be written to (0-7), column to be written to (0-127)
 * @retval 0,1 depending on success
 */
int SSD1306_WriteString(char* str){

		if (strlen(str) > 128){return 0;}

		char splitStrings[8][17] = {{'\0'}}; //can store 16 words of 16 characters with terminating /0 (Max that can fit on screen)

		int j,cnt;
	    j=0; cnt=0;
	    //SPLITTING STRING INTO WORDS BY SPACES
	    for(int i=0;i<=(strlen(str));i++)
	    {
	        // if space or NULL found, assign SPACE + NULL into splitStrings[cnt]
	        if(str[i]==' '||str[i]=='\0')
	        {
	        	splitStrings[cnt][j] = ' ';
	            splitStrings[cnt][j+1]='\0';
	            cnt++;  //for next word
	            j=0;    //for next word, init index to 0
	        }
	        //If we cant fit the entire word into buffer, move part of word to next word
	        else if (j < 15)
	        {
	            splitStrings[cnt][j]=str[i];
	            j++;
	        }
	        else{
	        	 splitStrings[cnt][j] = ' ';
	        	 splitStrings[cnt][j+1]='\0';
	        	 cnt++;  //for next word
	        	 j=0;    //for next word, init index to 0
	        }
	    }


	    //DISPLAYING WORDS TO SCREEN
	    for(int i = 0; i < cnt; i++){


			int length = strlen(splitStrings[i]);

			if((length * 8) + currentColumn < MAX_COLUMN){
				for(int k = 0; k < length; k++){
					SSD1306_WriteChar(splitStrings[i][k], currentPage, currentColumn);
					currentColumn+=8;
				}

			}
			else if(currentPage < MAX_PAGE){
				currentPage +=1;
				currentColumn = 0;
				if((length * 8) + currentColumn < MAX_COLUMN){
					for(int k = 0; k < length; k++){
						SSD1306_WriteChar(splitStrings[i][k], currentPage, currentColumn);
						currentColumn+=8;
					}
				}
				else{
					currentPage -=1;
					return 0;
				}
			}
	    }

	    return 1;
}

/**
 * @brief Inserts a singular pixel into the SSD1306_Buffer
 * @param int x, int y - location of pixel
 * @retval None
 */
void SSD1306_InsertPixelInBuffer(int x, int y){

	if(x >= COLUMNS || y >= ROWS) {
	        return;
	    }

	//As SSD1306 takes byte as input to RAM, we need to shift any other pixels around
	//You cannot access the memory in SSD1306 by pixel only so screen buffer is needed
	SSD1306_Buffer[x + (y / 8) * COLUMNS] |= 1 << (y % 8);

}

/**
 * @brief Updates the SSD1306 screen from the SSD1306_Buffer
 * @param None
 * @retval None
 */
void SSD1306_UpdateScreen(){
	for (uint8_t j = 0; j <= MAX_PAGE; j++){
			SSD1306_WriteData(&SSD1306_Buffer[COLUMNS*j],COLUMNS);
		}
}

/**
 * @brief Randomises all pixels in the SSD1306_Buffer
 * @param None
 * @retval None
 */
void SSD1306_RandomiseBuffer(){
	uint8_t randomUint;

	for (int i = 0; i < COLUMNS * ROWS / 8; i++){

		randomUint = rand() % 255;
		SSD1306_Buffer[i] = randomUint;
	}
}

/**
 * @brief Initialisation of SSD1306 - sets charge pump and display on
 * @param None
 * @retval None
 */
void SSD1306_Init_Test(void) {
	HAL_Delay(100);

	uint8_t commands[4] = { SET_CHARGE_PUMP, 0x14, SET_DISPLAY_POWER_OFF, SET_DISPLAY_POWER_ON};

	SSD1306_WriteMultiCommands(commands);


	HAL_Delay(100);

	for (uint8_t i = 0; i < 255; i++){
		SSD1306_WriteContrast(i);
	}

	SSD1306_WriteContrast(0x7F);

	SSD1306_WriteSingleCommand(SET_INVERSE_DISPLAY);

	HAL_Delay(100);

	SSD1306_WriteSingleCommand(SET_NORMAL_DISPLAY);

	HAL_Delay(100);

	SSD1306_WriteSingleCommand(SET_DISPLAY_GDDRAM);

	SSD1306_RandomiseBuffer();

	SSD1306_UpdateScreen();

	SSD1306_ClearScreenAddressing();

	SSD1306_RandomiseBuffer();

	SSD1306_UpdateScreen();

	SSD1306_ClearScreen();

	//SSD1306_WriteString("");

	//SSD1306_PixelTest();

}

/**
 * @brief UART recieve complete callback
 * @param UART_HandleTypeDef *huart
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

	__NOP();

	if(bitmapFlag == true){
		SSD1306_Buffer[currentXPos] = rx_char;
		currentXPos +=1;

		if(currentXPos == 1024){
			SSD1306_UpdateScreen();
			bitmapFlag = false;
			//HAL_UART_Receive_DMA ( &huart1, &rx_char, 1 );
		}
		else{
			HAL_UART_Transmit(&huart1, &requestPixel , 1, HAL_MAX_DELAY);
		}

	}
	else{

		bitmapFlag = false;

		if (rx_char == 0x04){
			//rx_buffer[rx_in] = rx_char;
			readFlag = true;
		}

		rx_buffer[rx_in] = rx_char;

		rx_in+=1;
	}

}

/**
 * @brief Clears rx_line of any stored data
 * @param None
 * @retval None
 */
void clear_rx_line(){

    for (int i = 0; i < line_size; i++){

        rx_line[i] = '\0';

        }

}

/**
 * @brief Reads from the rx buffer to rx_line until end of message char (0x04)
 * @param None
 * @retval None
 */
void read_line() {

	if (bitmapFlag == true){
		return;
	}

	int i;
	i = 0;

	//Critical Section
	HAL_NVIC_DisableIRQ(USART1_IRQn);
	while ((i==0) || (rx_line[i-1] != 0x04)) {

		if (rx_in == rx_out) {
			// End Critical Section - need to allow rx interrupt to get new characters for buffer
			        	HAL_NVIC_EnableIRQ(USART1_IRQn);

			            while (rx_in == rx_out) {

			            }
			// Start Critical Section - don't interrupt while changing global buffer variables
			            HAL_NVIC_DisableIRQ(USART1_IRQn);
			        }

		rx_line[i] = rx_buffer[rx_out];
		i++;
		rx_out = (rx_out + 1) % buffer_size;
		__NOP();
	}
	//readFlag = true;
	//End Critical Section
	HAL_NVIC_EnableIRQ(USART1_IRQn);

}


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  SSD1306_Init_Test();
  //HAL_UART_Receive_IT (&huart1, &rx_char, 1);
  HAL_UART_Receive_DMA ( &huart1, &rx_char, 1 );
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (readFlag == true){
		  read_line();
		  //NEED TO PUT CHECKS HERE FOR RX_LINE CURRENT SIZE
	  		  switch(rx_line[0]){

	  		  case 0x11:

	  		  case 0x12:
	  			  switch(rx_line[1]){

	  			  //Contrast change
	  			  case 'C':
	  				 __NOP();
	  		      //Bitmap Incoming
	  			  case 'B':
	  				HAL_NVIC_DisableIRQ(USART1_IRQn);
	  				  SSD1306_ClearScreen();
	  				  currentXPos = 0;
	  				  bitmapFlag = true;
	  				  readFlag = false;

	  				  HAL_UART_Transmit(&huart1, &requestPixel , 1, HAL_MAX_DELAY);
	  				HAL_NVIC_EnableIRQ(USART1_IRQn);

	  			  case 'R':

	  			  default:

	  				  __NOP();
	  			  }


	  		  case 0x13:

	  		  case 0x14:

	  		  default:
	  			  __NOP();

	  		  }
	  	  }



    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
