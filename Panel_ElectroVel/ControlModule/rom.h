// EEPROM, флаг, детектор данных
#define DATA_FLAG 0xAA

struct Settings 
{
  byte backLightMode;
	byte frontLight;
};

void loadSettings( Settings &settings ) 
{
	if ( EEPROM.read( 0x0 ) == DATA_FLAG )
	{
		EEPROM.get( 0x1, settings ); // Если маркер есть, загружаем настройки из EEPROM
		
		return;
  }
	
	settings.backLightMode = 0x4;
	settings.frontLight = 0x1;
}

void saveSettings( Settings &settings ) 
{
	//EEPROM.put( 0x0, DATA_FLAG );
	//EEPROM.put( 0x1, settings );
}
