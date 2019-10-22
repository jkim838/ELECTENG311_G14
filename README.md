# Linear Compressor Controller Software, 2018
#### Lead C Programmer: Oliver Kim (@jkim838, jkim838@aucklanduni.ac.nz)
#### Hardware Contributors: Norman Peni, Samisoni Tupou, Patrick Louis Ah Wong
#### The University of Auckland, Department of Electrical, Computer Systems and Software Engineering

## Overview

### Introduction

The aim of this project is to develop a software which controls the output and display operating condition of linear compressor for refrigerators. 

In operation, the controller software constantly communicates with the master in fixed-size JSON-formatted USART with the current operating condition of the linear compressor. The master system can control operating speed of the compressor by transmitting a fixed-size JSON-formatted text via USART.

Included in this document is an overview on development/testing environment and a complete list of components for the controller software.

### Environment

  * OS: Windows 10
  * Atmel Studio 7
  * Programmed for ATMEGA328P
  * Prototyped with XPLAINED MINI Developer Kit
  
  **DESKTOP**
  * CPU: AMD Ryzen™ 7 1700X CPU @ 3.90 GHz
  * GPU: nVidia GTX1080 8GB GP104
  * nVidia Graphics Driver Version: 418.67
  * CUDA Driver Version: 10.1
  * Memory: 16GB
  
  **LAPTOP**
  * CPU: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60 GHz
  * GPU: nVidia GTX1070 8GB GP104M (Mobile)
  * nVidia Graphics Driver Version: 418.67
  * CUDA Driver Version: 10.1
  * Memory: 16GB

## Components

#### Core File

  * main.c
  
  * Makefile
  
  * controller_software.componentinfo.xml
  
  * controller_software.cproj

#### 1. Analogue to Digital Converter (ADC)

  * adc_setup.c
  ```
  contains definition to setup analogue to digital converter
  ```

  * adc_setup.h
  ```
  contains prototypes to setup analogue to digital converter
  ```

#### 2. Communications with Master

  * comm_setup.c
  ```
  contains definition to setup USART to communicate with the master system in JSON format
  ```
  
  * comm_setup.h
  ```
  contains prototype to setup USART to communicate with the master system in JSON format
  ```
  
  * debug_usart.c
  ```
  contains debugging parameters and definitions for USART
  ```
  
  * debug_usart.h
  ```
  contains debugging parameters and prototypes for USART
  ```

#### 3. Timer

  * timer_setup.c
  ```
  contains definitions to setup timer which simulates a pulse-width modulated actuator voltage
  ```
  
  * timer_setup.h
  ```
  contains prototypes to setup timer which simulates a pulse-width modulated actuator voltage
  ```

#### 4. Miscellaneous

  * macro_definitions.h
  ```
  contains defintions for all macro used in the controller program
  ```
  
  * calculations.c
  ```
  contains definitions for calculating operation parameters such as airflow
  ```
  * calculations.h
  ```
  contains prototypes for calculating operation parameters such as airflow
  ```
