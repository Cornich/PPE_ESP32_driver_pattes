


static const float positionDef[8]= { 80, 90, 80, 90, 75, 90, 75, 90};
static const float positionBas[8]= { 60,115, 60,115, 60,115, 60,115};

static const float positionMar[12][8]= {
    //Séquence avant droit
 { 115,  90,  80,  90,  80,  90, 115,  90},//met la patte devant
 { 115,  50,  80,  90,  80,  90, 115,  50},//la tend
 {  80,  50,  80,  90,  80,  90,  80,  50},//la met en arrière
 {  70, 130,  80,  90,  80,  90,  70, 130},//plie pour ramener
 {  80, 130,  80,  90,  75,  90,  75, 130},//ramène
 {  80,  90,  80,  90,  75,  90,  75,  90},//position initiale
    //Séquence avant gauche

 {  80,  90, 115,  90, 115,  90,  80,  90},//met la patte devant
 {  80,  90, 115,  50, 115,  50,  80,  90},//la tend
 {   80,  90, 80,  50, 80,  50,  80,  90 },//la met en arrière
 {   80,  90, 70, 130, 70, 130,  80,  90 },//plie pour ramener
 {   80,  90, 80, 130, 75, 130,  75,  90 },//ramène
 {   80,  90, 80,  90, 75,  90,  75,  90 },//position initiale
};