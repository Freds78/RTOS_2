
/*==================[inlcusiones]============================================*/

// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"
#include "FreeRTOSConfig.h"
#include "tipos.h"
#include <string.h>

/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/
tLedTecla tecla_led_config[CANT];

/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

// Prototipo de funcion de la tarea
void tarea_led( void* taskParmPtr );
void tarea_tecla( void* taskParmPtr );
void tarea_mensaje(void* taskParmPtr);

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
	// ---------- CONFIGURACIONES ------------------------------
	// Inicializar y configurar la plataforma
	boardConfig();

	// UART for debug messages
	debugPrintConfigUart( UART_USB, 115200 );
	debugPrintlnString( "\nParte_1_RTOS2\n" );

	// Led para dar se�al de vida
	gpioWrite( LEDB , ON );


	tecla_led_config[0].tecla 	= TEC1;
	tecla_led_config[0].mutex 				=  xSemaphoreCreateMutex();

	tecla_led_config[1].tecla 	= TEC2;
	tecla_led_config[1].mutex 				=  xSemaphoreCreateMutex();

	// Crear tarea en freeRTOS
	BaseType_t Led =
			xTaskCreate(
					tarea_led,                     // Funcion de la tarea a ejecutar
					( const char * )"tarea_led",   // Nombre de la tarea como String amigable para el usuario
					configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
					0,                          // Parametros de tarea
					tskIDLE_PRIORITY+1,         // Prioridad de la tarea
					0                           // Puntero a la tarea creada en el sistema
			);

	if(Led == pdFAIL)
	{
		while(1){
			debugPrintlnString("\nError al crear la tarea \n");
		}

	}
	BaseType_t tecla1 =
			xTaskCreate(
					tarea_tecla,                     // Funcion de la tarea a ejecutar
					( const char * )"tarea_tecla1",  // Nombre de la tarea como String amigable para el usuario
					configMINIMAL_STACK_SIZE*2, 	// Cantidad de stack de la tarea
					&tecla_led_config[0],           // Parametros de tarea
					tskIDLE_PRIORITY+1,         	// Prioridad de la tarea
					0                           	// Puntero a la tarea creada en el sistema
			);

	if(tecla1 == pdFAIL)
	{
		while(1){
			debugPrintlnString("\nError al crear la tarea \n");
		}

	}

	BaseType_t tecla2 =
			xTaskCreate(
					tarea_tecla,                     // Funcion de la tarea a ejecutar
					( const char * )"tarea_tecla2",  // Nombre de la tarea como String amigable para el usuario
					configMINIMAL_STACK_SIZE*2, 	 // Cantidad de stack de la tarea
					&tecla_led_config[1],            // Parametros de tarea
					tskIDLE_PRIORITY+1,         	// Prioridad de la tarea
					0                           	// Puntero a la tarea creada en el sistema
			);

	if(tecla2 == pdFAIL)
	{
		while(1){
			debugPrintlnString("\nError al crear la tarea \n");
		}

	}

	BaseType_t mensaje =
			xTaskCreate(
					tarea_mensaje,                     // Funcion de la tarea a ejecutar
					( const char * )"tarea_mensaje",  // Nombre de la tarea como String amigable para el usuario
					configMINIMAL_STACK_SIZE*2, 	 // Cantidad de stack de la tarea
					0, 						          // Parametros de tarea
					tskIDLE_PRIORITY+1,         	// Prioridad de la tarea
					0                           	// Puntero a la tarea creada en el sistema
			);

	if(mensaje == pdFAIL)
	{
		while(1){
			debugPrintlnString("\nError al crear la tarea \n");
		}

	}

	mutex1 = xSemaphoreCreateMutex();

	cola1 = xQueueCreate( 20, sizeof( tLedTecla) );

	if( cola1 == NULL ) {

		while(1){
			debugPrintlnString("Error al crear la cola1\n");
		}

	}

	// Iniciar scheduler
	vTaskStartScheduler();

	// ---------- REPETIR POR SIEMPRE --------------------------
	while( TRUE )
	{
		// Si cae en este while 1 significa que no pudo iniciar el scheduler
	}

	// NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
	// directamenteno sobre un microcontroladore y no es llamado por ningun
	// Sistema Operativo, como en el caso de un programa para PC.
	return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/
void tarea_tecla( void* taskParmPtr )
{
	tLedTecla* config = (tLedTecla*) taskParmPtr;

	fsmButtonInit( config );

	while( 1 )
	{
		fsmButtonUpdate( config );
		vTaskDelay( 1 / portTICK_RATE_MS );
	}
}


void tarea_led( void* taskParmPtr )
{
	// ---------- CONFIGURACIONES ------------------------------
	tLedTecla* config = (tLedTecla*) taskParmPtr;
	//const char mensaje[] = "LED ON"; // Mensaje a enviar
	char mensaje[9];
	strcpy(mensaje,"LED ON\r\n" );
	// ---------- REPETIR POR SIEMPRE --------------------------
	while( TRUE )
	{


		gpioWrite( LED1 , ON );
		vTaskDelay(500/portTICK_RATE_MS);
		xSemaphoreTake( mutex1 , portMAX_DELAY );			//abrir seccion critica
		xQueueSend(cola1, &mensaje, portMAX_DELAY);			// enviar mensaje a la cola
		xSemaphoreGive( mutex1 );							//cerrar seccion critica
		gpioWrite( LED1, OFF );
		vTaskDelay(1000/portTICK_RATE_MS);

	}
}

void tarea_mensaje(void* taskParmPtr) {
	// ---------- CONFIGURACIONES ------------------------------

	tLedTecla* config = (tLedTecla*) taskParmPtr;
	char  envio[13];
	int i;
	// ---------- REPETIR POR SIEMPRE --------------------------
	while(TRUE) {



		xQueueReceive(cola1, &envio, portMAX_DELAY);
		xSemaphoreTake( mutex1 , portMAX_DELAY );
		for(i=0; i < strlen(envio); i++){
			printf("%c\r", envio[i]);
		}
		xSemaphoreGive( mutex1 );

		vTaskDelay(1000/portTICK_RATE_MS);
	}
}

/*==================[fin del archivo]========================================*/
