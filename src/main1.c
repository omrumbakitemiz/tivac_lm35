#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_adc.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"

//static inline void disable_interrupts() { asm("CPSID I"); }
static inline void enable_interrupts() { asm("CPSIE I"); }
static inline void wait_for_interrupt() { asm("WFI"); }

void init_adc(){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); // ADC0 ayarla
	SysCtlDelay(3); // 9 clock suresi bekler
	HWREG(ADC0_BASE + ADC_O_PC) = (ADC_PC_SR_250K); // ADC hizini 250 000 ornek/sn olarak ayarlar
	ADCHardwareOversampleConfigure(ADC0_BASE, 64); // 64 ornek al
	ADCSequenceDisable(ADC0_BASE, 1);	// durdur
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);	// calisma modunu ayarla (programdan tetikleyince calisan mod)
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH2 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 1);
}
void init_port_B() {
    volatile unsigned long delay;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB;
    delay = SYSCTL_RCGC2_R;
    GPIO_PORTB_DIR_R |= 0xFF;
    GPIO_PORTB_AFSEL_R &= ~0xFF;
    GPIO_PORTB_DEN_R |= 0xFF;
}
void init_port_A() {
    volatile unsigned long delay;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;
    delay = SYSCTL_RCGC2_R;
    GPIO_PORTA_DIR_R |= 0xff;
    GPIO_PORTA_AFSEL_R &= ~0xff;
    GPIO_PORTA_DEN_R |= 0xff;
}
void init_PortE() {
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTE_DIR_R |= 0XFF;
	GPIO_PORTE_AFSEL_R &= ~0XFF;
	GPIO_PORTE_DEN_R |= 0XFF;
}
void init_timer_0A(int period) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);	// timer_0 modulunu aktiflestir
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);	// timer_0 i periyodik olarak ayarla
    TimerLoadSet(TIMER0_BASE, TIMER_A, period);	// timer_0A modulunu "period" sayisindan geri sayacak sekilde ayarla
    IntEnable(INT_TIMER0A);	// timer_0A kesmesini aktiflestir
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);	// timer_0A'yi baslat
}
void init_timer_1A(int period){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);	// timer_0 modulunu aktiflestir
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);	// timer_0 i periyodik olarak ayarla
	TimerLoadSet(TIMER1_BASE, TIMER_A, period);	// timer_0A modulunu "period" sayisindan geri sayacak sekilde ayarla
	IntEnable(INT_TIMER1A);	// timer_0A kesmesini aktiflestir
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER1_BASE, TIMER_A);	// timer_0A'yi baslat
}

// 0'dan 9'a kadar olan sayilarin seven segment kodlari
// bit sirasi: g f e d c b a
uint8_t kodlar[10] = {
    0b0111111,
    0b0000110,
    0b1011011,
    0b1001111,
    0b1100110,
    0b1101101,
    0b1111101,
    0b0000111,
    0b1111111,
    0b1101111
};

unsigned long ulADC0Value[1];

// 1 saniyede kac kere ekran guncelleme kesmesi olacagi
#define EKRAN_KESME_HZ 400
int derece = 0;
int mV = 0;

// ekran guncelleme kesmesinin sayaci
int ekran_guncelle_sayac = 0;

/** Timer0A kesmesinde calistirilan fonksiyon */
void ekran_guncelleme_timer() {
    // timer interrupt clear (timer kesmesini algiladigimizi bildiriyoruz)
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    ekran_guncelle_sayac++;

    if (ekran_guncelle_sayac % 4 == 1) {
   	 int birler = derece % 10;
   	 GPIO_PORTA_DATA_R |= 0b11111111; // hepsini kapat
   	 GPIO_PORTB_DATA_R = kodlar[birler];
   	 GPIO_PORTA_DATA_R &= ~0b10000; // birler basamagini aktiflestir
    } else if (ekran_guncelle_sayac % 4 == 2) {
   	 int onlar = (derece / 10) % 10;
   	 GPIO_PORTA_DATA_R |= 0b11111111; // hepsini kapat
   	 GPIO_PORTB_DATA_R = kodlar[onlar];
   	 GPIO_PORTA_DATA_R &= ~0b100000; // onlar basamagini aktiflestir
    }
}

void adc_okuma(){
	// kesme bayragini sifirla (islemciye onceki kesmeyi aldigimizi bildiriyoruz)
	 ADCIntClear(ADC0_BASE, 1);

	 // ADC'ye baslama komutunu veriyoruz
	 ADCProcessorTrigger(ADC0_BASE, 1);

	 // ADC'nin islemi tamamlamasini bekle (surekli sor)
	 while (!ADCIntStatus(ADC0_BASE, 1, false));

	 // ADC'nin olctugu degeri oku
	 ADCSequenceDataGet(ADC0_BASE, 1, ulADC0Value);

	 derece = ((ulADC0Value[0]/10)); // 100 mv fazladan deger var.
}
int main() {
	SysCtlClockSet( SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    init_port_B();
    init_port_A();
    init_PortE();
    init_adc();
    init_timer_1A(SysCtlClockGet());
    // Ekran guncelleme islemi icin saniyede EKRAN_KESME_HZ tane kesme uretilecek
    init_timer_0A(SysCtlClockGet() / EKRAN_KESME_HZ);

    enable_interrupts(); // kesmeleri aktiflestir

    while (1) {
   	 wait_for_interrupt();
    }
}
