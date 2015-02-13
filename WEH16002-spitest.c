//
// Winstar WEH001602 OLED display code for raspberry pi  
// This is heavily based on ehajo.de code (c) Hannes Jochriem, 2013
// (http://dokuwiki.ehajo.de/artikel:displays:weh001602_spi_democode)
//
// raspi adaptation by woytekm
//

#include <bcm2835.h>

//
// we are using SPI pins as normal GPIO pins
//

#define MY_MISO RPI_BPLUS_GPIO_J8_19
#define MY_MOSI RPI_BPLUS_GPIO_J8_21
#define MY_CLK RPI_BPLUS_GPIO_J8_23
#define MY_CS0 RPI_BPLUS_GPIO_J8_24

#define DISPLAY_WIDTH 16

uint8_t G_clkval;

void my_spi_toggle_clk(void)
 {
  bcm2835_gpio_write(MY_CLK, G_clkval);
  usleep(3);
  G_clkval = 1 - G_clkval;
  bcm2835_gpio_write(MY_CLK, G_clkval);
  usleep(3);
  G_clkval = 1 - G_clkval;
 }


void my_spi_WEH001602_init(void)
 {
   bcm2835_gpio_fsel(MY_MISO, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(MY_MOSI, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(MY_CLK, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(MY_CS0, BCM2835_GPIO_FSEL_OUTP);

   bcm2835_gpio_write(MY_CLK, HIGH);
   bcm2835_gpio_write(MY_CS0, HIGH);
   bcm2835_gpio_write(MY_MOSI, LOW);

   G_clkval = 0;

   usleep(1000);
 }


void my_spi_WEH001602_out_cmd(uint8_t cmdbyte)
 {
   uint8_t i;

   bcm2835_gpio_write(MY_CS0, LOW);
    
   bcm2835_gpio_write(MY_MOSI, LOW);  /* send RS and RW bits - both 0 */
   my_spi_toggle_clk();
   bcm2835_gpio_write(MY_MOSI, LOW);
   my_spi_toggle_clk();

   for(i=0;i<8;i++)
    {
     if(cmdbyte & 128)
      bcm2835_gpio_write(MY_MOSI, HIGH);
     else
      bcm2835_gpio_write(MY_MOSI, LOW);
     my_spi_toggle_clk();
     cmdbyte = cmdbyte << 1;
    }

    bcm2835_gpio_write(MY_CS0, HIGH);
  }


void my_spi_WEH001602_out_data(uint8_t cmdbyte)
 {
  uint8_t i;

  bcm2835_gpio_write(MY_CS0, LOW);

  bcm2835_gpio_write(MY_MOSI, HIGH);  /* send RS=1 and RW=0 bits */
  my_spi_toggle_clk();
  bcm2835_gpio_write(MY_MOSI, LOW);
  my_spi_toggle_clk();

  for(i=0;i<8;i++)
   {
    if(cmdbyte & 128)
     bcm2835_gpio_write(MY_MOSI, HIGH);
    else
     bcm2835_gpio_write(MY_MOSI, LOW);
    my_spi_toggle_clk();
    cmdbyte = cmdbyte << 1;
   }

  bcm2835_gpio_write(MY_CS0, HIGH);
 }

void my_spi_WEH001602_out_text(uint8_t row, unsigned char *text)
 {
  char zeichen;
 
  if(row == 0)
   my_spi_WEH001602_out_cmd((0x80)+0);
  else
   my_spi_WEH001602_out_cmd((0x80)+64);	
	
  while ((zeichen = *text++)) 
   {	
    my_spi_WEH001602_out_data(zeichen);
   }
 }

main()
 {
   if (!bcm2835_init())
    return 1;

   my_spi_WEH001602_init();
                                          // command reference: http://www.adafruit.com/datasheets/NHD-0216KZW-AB5.pdf

   my_spi_WEH001602_out_cmd(0b00111001);  // Function set: 8bit, 2 Zeilen, 5x8 Punkte, Westeurop. Charset
   bcm2835_delay(10); 
   my_spi_WEH001602_out_cmd(0b00001100);  // Display on. Display an, Cursor aus, Blinken aus.
   bcm2835_delay(10);
   my_spi_WEH001602_out_cmd(0b00000001);  // Display clear
   bcm2835_delay(10);
   my_spi_WEH001602_out_cmd(0b00000010);  // Display home
   bcm2835_delay(10);
   my_spi_WEH001602_out_cmd(0b00000110);  // Entry mode: Dekrement, no shift.

   sleep(2);

   // scroll some demo text 

   uint8_t end = 0, scroll_pos,msglen,copy_len1, copy_len2;
   char *message = "quick brown fox jumps over a lazy dog * ";
   char scroll_sec[DISPLAY_WIDTH];

   msglen = strlen(message);
   scroll_pos = 0;

   copy_len1 = DISPLAY_WIDTH;

   while(!end)
   {
    if( (scroll_pos+DISPLAY_WIDTH) >= msglen)
     copy_len1 = msglen - scroll_pos;

    if(scroll_pos == msglen)
     {
      scroll_pos = 0;
      copy_len1 = DISPLAY_WIDTH;
     }

    memcpy(scroll_sec, message+scroll_pos, copy_len1);
    
    if(copy_len1 < DISPLAY_WIDTH)  // wrap message
     {
       copy_len2 = DISPLAY_WIDTH - copy_len1; 
       memcpy(scroll_sec+copy_len1, message, copy_len2);
     }

    my_spi_WEH001602_out_text(0,scroll_sec);

    usleep(60000); 

    scroll_pos++;
   } 

 }



