
// Struktur, beschreibend Unterschrift und Situation patch
struct defpatch {
 const char* sig; // Signaturen
 uint32_t sigsize; // Länge Signaturen
 int32_t poffset;  // Vorurteil zu Punkte patch aus das Ende Signaturen
};



//***********************************************************************
//* Suche Signaturen und Auferlegung patch
//***********************************************************************
uint32_t patch(struct defpatch fp, uint8_t* buf, uint32_t fsize, uint32_t ptype);

//****************************************************
//* Verfahren patch unter dem anders Chipsätze und Aufgaben
//****************************************************

uint32_t pv7r22 (uint8_t* buf, uint32_t fsize);
uint32_t pv7r22_2 (uint8_t* buf, uint32_t fsize);
uint32_t pv7r2 (uint8_t* buf, uint32_t fsize);
uint32_t pv7r11 (uint8_t* buf, uint32_t fsize);
uint32_t pv7r1 (uint8_t* buf, uint32_t fsize);
uint32_t perasebad (uint8_t* buf, uint32_t fsize);

