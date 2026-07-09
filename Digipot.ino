#include <Wire.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//#define DEBUG

#define POWER_STATUS_PIN (8)
#define RED_LED_PIN (2)
#define BLUE_LED_PIN (3)
#define SENSOR_PIN (4)

#define RED_LED_bm (PIN4_bm)
#define BLUE_LED_bm (PIN5_bm)

#define SENSOR_bm (PIN3_bm)
#define SENSOR_pos (3)

#define MIN_SAMPLES_0 (1)
#define MIN_SAMPLES_1 (5)
#define MAX_SAMPLES_0 (4)
#define MAX_SAMPLES_1 (8)
#define MIN_SAMPLES_START (9)
#define MAX_SAMPLES_START (50)

#define NUM_BITS (12)

#define VOL_UP_CMD (0x490)
#define VOL_DOWN_CMD (0xC90)
#define VOL_MUTE (0x290)
#define IR_DOWNTIME (500)

#define LED_ON_TICKS (1000)
#define MAX_COMBO (30)

volatile uint8_t vol = 1;
volatile uint8_t mute_flag = 0;
volatile uint8_t sent_vol = 0;

// Your register address, which can be found on your IC's datasheet
#define SUB_0_ADDR 0x2E //please correct me if wrong, datasheet says '0101 11'b + A0 = 0010 111 (0 or 1)
#define SUB_1_ADDR 0x2F
#define WRITE_CMD 0x00

/*Pinouts:
(11)PA0 = programming Pin
(8)PA1 = Power Data Input
(0)PA4 = LED Red
(1)PA5 = LED Blue
(2)PA6 = Volume Up Pushbutton
(3)PA7 = Volume Down Pushbutton

(7)PB0 = SCL
(6)PB1 = SDA
(5)PB2 = TestPoint
(4)PB3 = IRLED
*/

ISR(TCA0_OVF_vect) {
    static volatile uint8_t samp_count = 0;
    static volatile uint8_t bit_count = 0;
    static volatile uint16_t cmd_buffer = 0x00;

    static volatile uint16_t red_led_on_ticks = 0;
    static volatile uint16_t blue_led_on_ticks = 0;

    static volatile uint16_t on_ticks = 0;
    static volatile uint8_t combo_meter = 0;

    static volatile uint16_t last_cmd = 0;

    uint8_t sensor_state = (PORTB.IN & SENSOR_bm) >> SENSOR_pos;

    if (sensor_state) {
      // Check to see if we just got high
      if(on_ticks <= IR_DOWNTIME){
        ++on_ticks;
        if(on_ticks == IR_DOWNTIME){
          combo_meter = 0;
        }
      }

      if (samp_count) {
        if (bit_count <= NUM_BITS)
          ++bit_count;

        if (samp_count > MIN_SAMPLES_0 && samp_count <= MAX_SAMPLES_0) {
          // I guess we do nothing
        }
        else if (samp_count > MIN_SAMPLES_1 && samp_count <= MAX_SAMPLES_1) {
          cmd_buffer |= (1 << (NUM_BITS - bit_count));
        }
        else if(samp_count > MIN_SAMPLES_START && samp_count <= MAX_SAMPLES_START) {
          // We got a start bit
          cmd_buffer = 0x00;
          bit_count = 0;
        }

        // Check how many bits we got
        if (bit_count == NUM_BITS) {
          if (last_cmd == cmd_buffer) {
            if (combo_meter < MAX_COMBO)
              ++combo_meter;
          }

          else {
            combo_meter = 0;
          }

          last_cmd = cmd_buffer;

          if (cmd_buffer == VOL_UP_CMD && vol < 255) {
            mute_flag = 0;

            if (combo_meter == MAX_COMBO) {
              vol = vol < 250 ? vol + 5 : 255;
            } else ++vol;

            red_led_on_ticks = LED_ON_TICKS;
          }

          else if (cmd_buffer == VOL_DOWN_CMD && vol > 0) {
            mute_flag = 0;

            if (combo_meter == MAX_COMBO) {
              vol = vol > 5 ? vol - 5 : 0;
            } else --vol;

            blue_led_on_ticks = LED_ON_TICKS;
          }

          else if (cmd_buffer == VOL_MUTE) {
            if (combo_meter == 1) {
              mute_flag = !mute_flag;
            }
          }
            
#ifdef DEBUG
          Serial.print("Command: 0x");
          Serial.print(cmd_buffer, HEX);
          Serial.print(" Volume: ");
          Serial.println(vol);
#endif
        }
      }

      if (red_led_on_ticks) {
        --red_led_on_ticks;
        PORTA.OUTSET = RED_LED_bm;
      } else if (mute_flag) {
        PORTA.OUTSET = RED_LED_bm;
      }
      else {
        PORTA.OUTCLR = RED_LED_bm;
      }
      
      if (blue_led_on_ticks) {
        --blue_led_on_ticks;
        PORTA.OUTSET = BLUE_LED_bm;
      } else {
        PORTA.OUTCLR = BLUE_LED_bm;
      }

      // Clear this for future samples
      samp_count = 0;
    }

    else {
      on_ticks = 0;
      ++samp_count; // It's faster this way trust me
    }
    
    // Clear the overflow interrupt flag to allow the next interrupt
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

void timer_init() {
  TCA0.SINGLE.PER = 250; // Hopefully 200us?
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm;
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
  sei();
}

void setup() {
	Wire.begin();
  Serial.begin(115200);

  pinMode(SENSOR_PIN, INPUT);
  pinMode(POWER_STATUS_PIN, INPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  timer_init();
  
  Serial.println("Sony IR volume control with I2C digipot");
  Serial.print("CPU Frequency: ");
  Serial.print(F_CPU);
}

void loop() {

  uint8_t next_vol = mute_flag ?   0 : vol;
  if(next_vol != sent_vol){
    sent_vol = next_vol;
  
    // Write volume data
    Wire.beginTransmission(SUB_0_ADDR);  
    Wire.write(WRITE_CMD); 
    Wire.write(sent_vol);
    Wire.endTransmission();  

    Wire.beginTransmission(SUB_1_ADDR);  
    Wire.write(WRITE_CMD); 
    Wire.write(sent_vol);
    Wire.endTransmission();  

    Serial.print(" Volume: ");
    Serial.print(sent_vol);
    Serial.print(", Mute Flag: ");
    Serial.println(mute_flag);

  }
  
  delay(10);
}