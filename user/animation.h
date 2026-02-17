#ifndef __ANIMATION_H__
#define __ANIMATION_H__

uint32_t rand_simple(void);
void random64(uint8_t num,_Bool color);
void random32(uint8_t num,_Bool color);
void random16(uint8_t num,_Bool color);
void bigmiddle64(uint8_t num,_Bool color);

void Normalwrite32(uint8_t num,_Bool color);
void Widen32(uint8_t num,_Bool color);
void Compression32(uint8_t num,_Bool color);
void Rectupdown32(uint8_t num,_Bool color);

void Draw_RadialLines(int num,_Bool color);
void Floattext_16(uint8_t num,_Bool color);
void Normalwrite16(uint8_t num,_Bool color);

#endif
