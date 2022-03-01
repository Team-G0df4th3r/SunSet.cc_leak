#include "custom_sounds.hpp"

__forceinline void setup_sounds()
{
	CreateDirectory("csgo\\sound", nullptr);
	FILE* file = nullptr;

	file = fopen(("csgo\\sound\\metallic.wav"), ("wb"));
	fwrite(metallic, sizeof(unsigned char), 64700, file); //-V575
	fclose(file);

	file = fopen(("csgo\\sound\\cod.wav"), ("wb"));
	fwrite(cod, sizeof(unsigned char), 11752, file);
	fclose(file);

	file = fopen(("csgo\\sound\\bubble.wav"), ("wb"));
	fwrite(bubble, sizeof(unsigned char), 41550, file);
	fclose(file);

	file = fopen(("csgo\\sound\\stapler.wav"), ("wb"));
	fwrite(stapler, sizeof(unsigned char), 105522, file);
	fclose(file);

	file = fopen(("csgo\\sound\\bell.wav"), ("wb"));
	fwrite(bell, sizeof(unsigned char), 42154, file);
	fclose(file);

	file = fopen(("csgo\\sound\\flick.wav"), ("wb"));
	fwrite(flick, sizeof(unsigned char), 152168, file);
	fclose(file);

	file = fopen(("csgo\\sound\\kill1.wav"), ("wb"));
	fwrite(kill1, sizeof(unsigned char), 248910, file);
	fclose(file);

	file = fopen(("csgo\\sound\\kill2.wav"), ("wb"));
	fwrite(kill2, sizeof(unsigned char), 235086, file);
	fclose(file);

	file = fopen(("csgo\\sound\\kill3.wav"), ("wb"));
	fwrite(kill3, sizeof(unsigned char), 262734, file);
	fclose(file);

	file = fopen(("csgo\\sound\\kill4.wav"), ("wb"));
	fwrite(kill4, sizeof(unsigned char), 253518, file);
	fclose(file);

	file = fopen(("csgo\\sound\\kill5.wav"), ("wb"));
	fwrite(kill5, sizeof(unsigned char), 322652, file);
	fclose(file);

	file = fopen(("csgo\\sound\\kill6.wav"), ("wb"));
	fwrite(kill6, sizeof(unsigned char), 271950, file);
	fclose(file);

	file = fopen(("csgo\\sound\\kill7.wav"), ("wb"));
	fwrite(kill7, sizeof(unsigned char), 318044, file);
	fclose(file);

	file = fopen(("csgo\\sound\\kill8.wav"), ("wb"));
	fwrite(kill8, sizeof(unsigned char), 253518, file);
	fclose(file);

	file = fopen(("csgo\\sound\\kill9.wav"), ("wb"));
	fwrite(kill9, sizeof(unsigned char), 304206, file);
	fclose(file);

	file = fopen(("csgo\\sound\\kill10.wav"), ("wb"));
	fwrite(kill10, sizeof(unsigned char), 327246, file); 
	fclose(file);

	file = fopen(("csgo\\sound\\kill11.wav"), ("wb"));
	fwrite(kill11, sizeof(unsigned char), 396380, file);
	fclose(file);
}