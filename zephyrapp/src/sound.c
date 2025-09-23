#include "nrf5340_application.h"
#include "nrf5340_application_bitfields.h"
#include "sound.h"
#include "sbcdec.h"

#ifndef FUCK_ZEPHYR

	#include <zephyr/kernel.h>
	#include <zephyr/drivers/gpio.h>
	#include "misc.h"

#endif

#define AUDIO_AMPLITUDE		512

#define BUFFER_SIZE			512		//better be a multiple of 4
static uint16_t __attribute__((aligned(4))) mBuffer[BUFFER_SIZE] = {};
static uint8_t mPlayingSecondHalf = 0;	

static const uint8_t *mSoundDatanfow, *mSoundDataEnd;

static void soundPrvScaleData(uint16_t *dst, const int16_t *src, uint32_t numItems)
{
	const uint32_t *src32 = (uint32_t*)src;
	uint32_t *dst32 = (uint32_t*)dst;
	uint32_t numWords = numItems / 2;


	//for each: *dst++ = (uint32_t)(*src++ + 32768) / (65536 / AUDIO_AMPLITUDE);

	do {
		*dst32++ = ((*src32++ ^ 0x80008000) & 0xff80ff80) >> 7;
	} while (--numWords);
}

static bool soundPrvProduceSamples(uint16_t *dst, uint32_t nsamples)
{
	static int16_t __attribute__((aligned(4))) decodedData[SBC_MAX_SAMPLES_OUT];
	static uint8_t decodedDataAvail = 0;
	bool anyProduced = false;

	while (nsamples) {

		if (decodedDataAvail) {

			uint32_t now;

			now = decodedDataAvail < nsamples ? decodedDataAvail : nsamples;
			soundPrvScaleData(dst, decodedData + SBC_MAX_SAMPLES_OUT - decodedDataAvail, now);
			decodedDataAvail -= now;
			nsamples -= now;
			dst += now;

			anyProduced = true;
		}
		else {

			if (mSoundDatanfow < mSoundDataEnd) {

				decodedDataAvail = sbcDecode(decodedData, &mSoundDatanfow);
			}
			else {

				do {
					*dst++ = AUDIO_AMPLITUDE / 2;
				} while (--nsamples);
			}
		}
	}

	return anyProduced;
}

static void soundPrvScheduleNextHalf(void)
{
	uint8_t halfToFill = mPlayingSecondHalf, halfToPlay = 1 - halfToFill;

	//asap start playback of next buffer
	NRF_PWM1_S->SEQ[0].PTR = (uintptr_t)(halfToPlay ? mBuffer + BUFFER_SIZE / 2 : mBuffer);
	NRF_PWM1_S->SEQ[0].CNT = BUFFER_SIZE / 2;
	NRF_PWM1_S->TASKS_SEQSTART[0] = 1;
	
	//clear irq
	NRF_PWM1_S->EVENTS_SEQEND[0] = 0;

	//accounting
	mPlayingSecondHalf = halfToPlay;

	//produce more samples
	if (!soundPrvProduceSamples(halfToFill ? mBuffer + BUFFER_SIZE / 2 : mBuffer, BUFFER_SIZE / 2)) {

		//stop after current playback
		NRF_PWM1_S->INTENCLR = PWM_INTEN_SEQEND0_Msk;
	}
}

void __attribute__((used)) PWM1_IRQHandler(void)
{
	soundPrvScheduleNextHalf();
}

static void soundPrvForceStop(void)
{
	NRF_PWM1_S->INTENCLR = PWM_INTEN_SEQEND0_Msk;
	NRF_PWM1_S->TASKS_STOP = 1;
	NRF_PWM1_S->EVENTS_SEQEND[0] = 0;
}

static void soundPrvStart(const uint8_t *data, uint32_t len)		//assumes sound is not playing currently
{
	uint32_t i;
	bool decodeSuccess;

	soundPrvForceStop();

	for (i = 0; i < BUFFER_SIZE; i++)
		mBuffer[i] = AUDIO_AMPLITUDE / 2;

	sbcDecInit();

	mSoundDatanfow = data;
	mSoundDataEnd = data + len;
	mPlayingSecondHalf = 0;

	decodeSuccess = soundPrvProduceSamples(mBuffer + BUFFER_SIZE / 2, BUFFER_SIZE / 2);		//fill second half, since we'll start there
	
	if (decodeSuccess)	//this is unlikely to be the case - only if sound was entirely empty
		NRF_PWM1_S->INTENSET = PWM_INTEN_SEQEND0_Msk;

	soundPrvScheduleNextHalf();
}

bool soundIsPlaying(void)
{
	if (NRF_PWM1_S->INTEN & PWM_INTEN_SEQEND0_Msk)
		return true;

	return !(NRF_PWM1_S->EVENTS_SEQEND[0] & PWM_EVENTS_SEQEND_EVENTS_SEQEND_Msk);
}

void soundPlay(const uint8_t *data, uint32_t len, enum SoundPlayMode playbackMode)
{
	switch (playbackMode) {
		case SoundReplaceCurrent:
			if (soundIsPlaying())
				soundPrvForceStop();
			break;

		case SoundWaitForCurrent:
			while (soundIsPlaying());
			break;

		case SoundDoNothingIfPlaying:
			if (soundIsPlaying())
				return;
	}

	soundPrvStart(data, len);
}

void soundStopAll(void)
{
	if (soundIsPlaying())
		soundPrvForceStop();
}

void soundPower(bool on)
{
	if (on) {

		NRF_P1_S->PIN_CNF[8] = (NRF_P1_S->PIN_CNF[8] &~ (GPIO_PIN_CNF_MCUSEL_Msk | GPIO_PIN_CNF_PULL_Msk | GPIO_PIN_CNF_DRIVE_Msk | GPIO_PIN_CNF_INPUT_Msk | GPIO_PIN_CNF_DIR_Msk)) |
			(GPIO_PIN_CNF_MCUSEL_Peripheral << GPIO_PIN_CNF_MCUSEL_Pos) | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
			(GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);

		NRF_PWM1_S->MODE = 0;
		NRF_PWM1_S->COUNTERTOP = AUDIO_AMPLITUDE;
		NRF_PWM1_S->DECODER = 0;
		NRF_PWM1_S->PRESCALER = 0;
		NRF_PWM1_S->PSEL.OUT[0] = (0 << 31) /* connected */ + (1 << 5) /* port 1 */ + (8 << 0) /* pin 8 */;
		NRF_PWM1_S->ENABLE = 1;

		NRF_PWM1_S->LOOP = 0;
		NRF_PWM1_S->SEQ[0].REFRESH = 0;
		NRF_PWM1_S->SEQ[0].ENDDELAY = 0;


		#ifndef FUCK_ZEPHYR

			IRQ_DIRECT_CONNECT(PWM1_IRQn, 2 /* prio */, PWM1_IRQHandler, 0 /* flags */);
			irq_enable(PWM1_IRQn);

		#else
			NVIC_EnableIRQ(PWM1_IRQn);
		#endif
	}
	else {

		#ifndef FUCK_ZEPHYR
			irq_disable(PWM1_IRQn);
		#else
			NVIC_DisableIRQ(PWM1_IRQn);
		#endif

		NRF_PWM1_S->ENABLE = 0;
		NRF_P1_S->PIN_CNF[8] = (NRF_P1_S->PIN_CNF[8] &~ (GPIO_PIN_CNF_MCUSEL_Msk | GPIO_PIN_CNF_PULL_Msk | GPIO_PIN_CNF_DRIVE_Msk | GPIO_PIN_CNF_INPUT_Msk | GPIO_PIN_CNF_DIR_Msk)) |
			(GPIO_PIN_CNF_MCUSEL_Msk << GPIO_PIN_CNF_MCUSEL_Pos) | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
			(GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
	}
}

static const struct gpio_dt_spec pin_speaker =
	GPIO_DT_SPEC_GET(DT_NODELABEL(cat_misc), speaker_gpios);

void test_speaker()
{
	init_pin(&pin_speaker, "pin_speaker", GPIO_OUTPUT_INACTIVE);

	for (int i = 0; i < 50; i++)
	{
		gpio_pin_toggle_dt(&pin_speaker);
		k_msleep(1);
	}
}