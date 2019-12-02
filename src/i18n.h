/*
* Copyright 2019 Stepan Perun
* This program is free software.
* 
* Translators:
* Anton Midyukov
* Heimen Stoffels
* Pavel Fric
* Martin Gansser
* 
* License: Gnu Lesser General Public License
* http://www.gnu.org/licenses/lgpl.html
*/


#ifndef I18N_H
#define I18N_H


typedef struct _MsgIdStr MsgIdStr;

struct _MsgIdStr
{
	const char *msgid;
	const char *msgstr;
};

typedef struct _Langs Langs;

struct _Langs
{
	const char *lang_sys;
	const char *lang_name;
	MsgIdStr *msgidstr;
	uint num;
};


MsgIdStr cs_CZ_msgidstr_n[] =
{
    { "Audio equalizer", 	"Audioekvalizér"	},
    { "Bandwidth Hz", 		"Šířka pásma Hz"    },
    { "Brightness", 		"Jas"     			},
    { "Channels", 			"Kanály"     		},
    { "Contrast", 			"Kontrast"     		},
    { "Files", 				"Soubory"     		},
    { "Frequency Hz", 		"Kmitočet Hz"     	},
    { "Hue", 				"Odstín"     		},
    { "Level dB", 			"Úroveň dB"     	},
    { "Saturation", 		"Sytost"     		},
    { "Scanner", 			"Skener"     		},	
    { "Undefined", 			"Nestanoveno"		},
    { "Video equalizer", 	"Videoekvalizér" 	}
};

MsgIdStr de_DE_msgidstr_n[] =
{
    { "Audio equalizer", 	"Audio-Equalizer"  	},
    { "Bandwidth Hz", 		"Bandbreite Hz"  	},
    { "Brightness", 		"Helligkeit"  		},
    { "Channels", 			"Kanäle"  			},
    { "Contrast", 			"Kontrast"  		},
    { "Files", 				"Dateien"  			},
    { "Frequency Hz", 		"Frequenz Hz"  		},
    { "Hue", 				"Farbton"  			},
    { "Level dB", 			"Pegel dB"  		},
    { "Saturation", 		"Sättigung"  		},
    { "Scanner", 			"Scanner"  			},
    { "Undefined", 			"Nicht definiert"  	},
    { "Video equalizer", 	"Video-Equalizer" 	}
};

MsgIdStr nl_NL_msgidstr_n[] =
{
    { "Audio equalizer", 	"Audio-equalizer" 	},
    { "Bandwidth Hz", 		"Bandbreedte Hz"	},
    { "Brightness", 		"Helderheid"     	},
    { "Channels",		 	"Kanalen"     		},
    { "Contrast", 			"Contrast"     		},
    { "Files", 				"Bestanden"     	},
    { "Frequency Hz", 		"Frequentie Hz"     },
    { "Hue", 				"Tint"     			},
    { "Level dB", 			"Niveau dB"     	},
    { "Saturation", 		"Verzadiging"     	},
    { "Scanner", 			"Scanner"     		},
    { "Undefined", 			"Niet-opgegeven" 	},
    { "Video equalizer", 	"Video-equalizer"	}
};

MsgIdStr ru_RU_msgidstr_n[] =
{
    { "Audio equalizer", 	"Аудио эквалайзер"  },
    { "Bandwidth Hz", 		"Полоса Hz"     	},
    { "Brightness", 		"Яркость"     		},
    { "Channels", 			"Каналы"     		},
    { "Contrast", 			"Контрастность"     },
    { "Files", 				"Файлы"     		},
    { "Frequency Hz", 		"Частота Hz"     	},
    { "Hue", 				"Оттенок"     		},
    { "Level dB", 			"Уровень dB"     	},
    { "Saturation", 		"Насыщенность"     	},
    { "Scanner", 			"Сканер"     		},
    { "Undefined", 			"Не определено"     },
    { "Video equalizer", 	"Видео эквалайзер"  },
    { "Not available during recording.", "Недоступно во время записи." },
    { "Live recording stopped.", "Запись в прямом эфире остановлена." },
    { "Live playback stopped.", "Воспроизведение прямого эфира остановлено." }
};

MsgIdStr uk_UA_msgidstr_n[] =
{
    { "Audio equalizer", 	"Аудіо еквалайзер"  },
    { "Bandwidth Hz", 		"Смуга Hz"     		},
    { "Brightness", 		"Яскравість"     	},
    { "Channels", 			"Канали"     		},
    { "Contrast", 			"Контраст"     		},
    { "Files", 				"Файли"     		},
    { "Frequency Hz", 		"Частота Hz"     	},
    { "Hue", 				"Відтінок"     		},
    { "Level dB", 			"Рівень dB"     	},
    { "Saturation", 		"Насиченість"     	},
    { "Scanner", 			"Сканер"     		},
    { "Undefined", 			"Не визначено"     	},
    { "Video equalizer", 	"Відео еквалайзер"  },
    { "Not available during recording.", "Недоступно під час запису." },
    { "Live recording stopped.", "Запис в прямому ефірі зупинено." },
    { "Live playback stopped.", "Відтворення прямого ефіру зупинено." }
};


Langs langs_n[] =
{
    { "en_EN", "English",   NULL, 0 },
    { "cs_CZ", "Czech",     cs_CZ_msgidstr_n, G_N_ELEMENTS ( cs_CZ_msgidstr_n ) },
    { "de_DE", "German", 	de_DE_msgidstr_n, G_N_ELEMENTS ( de_DE_msgidstr_n ) },
    { "nl_NL", "Dutch",     nl_NL_msgidstr_n, G_N_ELEMENTS ( nl_NL_msgidstr_n ) },
    { "ru_RU", "Russian",   ru_RU_msgidstr_n, G_N_ELEMENTS ( ru_RU_msgidstr_n ) },
    { "uk_UA", "Ukrainian", uk_UA_msgidstr_n, G_N_ELEMENTS ( uk_UA_msgidstr_n ) }
};

#endif // I18N_H
