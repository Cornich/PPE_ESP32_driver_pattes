static const float positionDef[8]= { 75, 90, 75, 90, 75, 90, 75, 90};
static const float positionBas[8]= { 60,115, 60,115, 60,115, 60,115};


#define nbPosAvancer 12


const float dha=135, dba=10; // défaut haut, défaut bas AVANT
const float dhr=45, dbr=95; // défaut haut, défaut bas aRRIÈRE
const float pha[nbPosAvancer/2 -1]={ 70,70,35,25,45};
const float phr[nbPosAvancer/2 -1]={ 70,70,35,25,45};
const float pba[nbPosAvancer/2 -1]={110,70,70,150,150};
const float pbr[nbPosAvancer/2 -1]={110,70,70,150,150};

// les mouvements sont symétriques. Pour modifier un côté ou l'autre, voir DecXXX dans task_servos.cpp
static const float positionMar[nbPosAvancer][8]= {
    //Séquence avant droit
 {pha[0],pba[0], dha, dba, dhr, dbr, phr[0],pbr[0]},//met la patte devant
 {pha[1],pba[1], dha, dba, dhr, dbr, phr[1],pbr[1]},//la tend
 {pha[2],pba[2], dha, dba, dhr, dbr, phr[2],pbr[2]},//la met en arrière
 {pha[3],pba[3], dha, dba, dhr, dbr, phr[3],pbr[3]},//plie pour ramener
 {pha[4],pba[4], dha, dba, dhr, dbr, phr[4],pbr[4]},//ramène
 { dha  , dba  , dha, dba, dhr, dbr, dhr   ,dbr  },//position initiale
    //Séquence avant gauche
 { dha, dba,pha[0], pba[0],phr[0], pbr[0], dhr, dbr},//met la patte devant
 { dha, dba,pha[1], pba[1],phr[1], pbr[1], dhr, dbr},//la tend
 { dha, dba,pha[2], pba[2],phr[2], pbr[2], dhr, dbr},//la met en arrière
 { dha, dba,pha[3], pba[3],phr[3], pbr[3], dhr, dbr},//plie pour ramener
 { dha, dba, pha[4],pba[4],phr[4], pbr[4], dhr, dbr},//ramène
 { dha, dba, dha, dba,dhr, dbr, dhr, dbr}//position initiale

};
