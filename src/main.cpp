//Blibliotecas
  #include <Arduino.h> 
  #include <EEPROM.h> //Blibioteca memoria EEPROM 4Kb
  #include <memorysaver.h> //Blbioteca UTFT LCD SD/CARD 
  #include <UTFT.h>
  #include "EEPROMAnything.h"
//Parametrizacao de funcoes 
  void primeiraTela();
  void telaManual();
  void telaCalibracao();
  void telaConfiguracoes();
  void oversampling();
  void selecaobocal();
  void titulos(String sup,String inf);  
  void selecao(int ns);  
  float pressao(float ovsa_p, int zero_p, float constante_p);
  float vazao(float ovsa_v, int zero_v, float constante_v, float bocal_v);

//Define a orientação do touch futuramente
#define TOUCH_ORIENTATION  PORTRAIT

//-------Pino dos transmissores 
#define TRANSMISSOR1   A5
#define TRANSMISSOR2   A7
//-------Pino dos botões
#define PIN_ENTER     54  
#define PIN_BAIXO     55 
#define PIN_CIMA      56 
#define PIN_VOLTAR    57 
//-------Pino dos reles e inversor
#define RELE1         8
#define RELE2         9
#define RELE3         10
#define RELE4         11
#define INVERSOR      13


//Declara quas fontes usaremos 
extern uint8_t SmallFont[];
extern uint8_t BigFont[];

//O modelo do modulo deve ser escolhido na ducumentação UTFT na seguinte configuração : <display model>,38,39,40,41
UTFT myGLCD(ITDB32S_V2,38,39,40,41);

//Configura os pinos do touch via Sheld
//URTouch  myTouch( 6, 5, 4, 3, 2);

//variavel externa no qual esta a BMP
extern unsigned int vectus[0x3458];
//variaveis da função overS
  unsigned long amostragem1 = 0;
  unsigned long amostragem2 = 0; 
  unsigned int counterAmostragem = 256;
//variaveis da função overS  
  String tituloSup = "VecTus";
  String tituloInf = "www.vectus.com.br";
//variaveis Globais
  
  bool check;  
  float oversampling1;
  float oversampling2;
  float fatorBocal[3] = {0.2161 , 0.7807, 2.9070}; 
  String butBocal[3] = {"Bocal P","Bocal M","Bocal G"};  
  int cont,nSelecoes, selecionado, ultSelecionado, tamBocal = 0;
  bool selAlterada= false;
  bool sair = true;
  float ftrScala1 = 0.15788165091994032819492789656887;
  float ftrScala2 = 0.15;
  float zero, zero2 = 0;

  
  //Estouro do timers
ISR(TIMER2_OVF_vect){
  TCNT2=194; //Reinicializa o Timer2
    oversampling();     
}
ISR(TIMER1_OVF_vect){
  TCNT1=0xF230; //Reinicializa o Timer2
  selecao(nSelecoes);        
}
  
//--------------------------------------------------------------------------------

void setup() {

 // ******************************************
 // Configuraçôes de inicialização de tela
 //*******************************************
 // inicializa display LDC e configura sua orientação: 0-PORTAIT / 1-LANDSCAPE
  myGLCD.InitLCD(1);
 // Configura fonte do display:  SmallFont / Bigfont / SevenSeqNunFont
  myGLCD.setFont(SmallFont);  
 // Limpa a tela e desenha um novo quadro
  myGLCD.clrScr();
 //Cor do backgroud padrao de inicialização 
  myGLCD.fillScr(VGA_WHITE);
    
  // ***************************************************************************
  //Estouro = timer *prescaler * ciclo de maquina   = =(254-)* 1024 * 59,6E9
  //ciclo de maquina = 1/16MHz = 59,6E9
  //Timer(bits - valor timer) =(254 - x)
  //****************************************************************************
  // Configuração dos registradores do TIMER2 interuupção por estouro
  //*************************************************************************

  TCCR2A = 0x00; //Timer em modo normal
  TCCR2B = 0x07; //Prescaler 1:1024
  TCNT2 = 194;   //4ms overflow again
  TIMSK2 = 0x01; //Habilita interrupção do timer2
  
  TCCR1A = 0x00; //Timer em modo normal
  TCCR1B = 0x05; //Prescaler 1:1024
  TCNT1 = 0xC350;   //130ms overflow again
  TIMSK1 = 0x01; //Habilita interrupção do timer1
  
 // **********************************************
 // Define Entradas e Saidas analogicas e digitais 
 //************************************************


  pinMode(PIN_ENTER,INPUT);
  pinMode(PIN_BAIXO,INPUT);
  pinMode(PIN_CIMA,INPUT);
  pinMode(PIN_VOLTAR,INPUT);
  pinMode(RELE1,OUTPUT);
  pinMode(RELE2,OUTPUT);
  pinMode(RELE3,OUTPUT);
  pinMode(RELE3,OUTPUT);

  

}
//-------------------------------------------------------------------------------------------------------
void loop() {

  myGLCD.fillScr(VGA_WHITE);
  titulos(tituloSup, tituloInf);
  myGLCD.drawBitmap(50,100,200,67,vectus);
  delay(2000); 
  
  EEPROM_readAnything(0, check);
  if(check == TRUE){
	  EEPROM_updateAnything(1, ftrScala1);
      EEPROM_updateAnything(5, ftrScala2);
      EEPROM_updateAnything(9, zero1);
      EEPROM_updateAnything(13, zero2);
  }
  else if(check == FALSE){
	  myGLCD.fillScr(VGA_WHITE);
      titulos(tituloSup, tituloInf);
	  
	  myGLCD.setColor(VGA_BLACK);
      myGLCD.setBackColor(VGA_WHITE);
	  myGLCD.setFont(BigFont);
	  
      myGLCD.print("Não há configuracoes iniciais.", CENTER ,100);
	  myGLCD.print("Favor configurar o equipamento.", CENTER ,200);
	  delay(3000);
  }
  
  
  myGLCD.fillScr(VGA_WHITE);
  titulos("selecione", tituloInf);
  sair=true;
  selAlterada= false;
  
  primeiraTela(); 
  
  switch(selecionado){
    case 0:
       //       telaAutomatico();
       break;
    case 1:
    telaManual();
      break;
    case 2:
        //    telaBluetooth();
       break;
     case 3:
        telaConfiguracoes();
       break;
   }
  
}

void primeiraTela(){
  nSelecoes = 3;
  selecionado=0;
  titulos(tituloSup, tituloInf);
  
  while (sair)
  {
    do
    {
      if(digitalRead(PIN_ENTER)){
        sair = false;
        selAlterada=false;
      }
    } while (selAlterada);
    
      selAlterada=true;     
  if(selecionado == 0){
      
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(40, 40, 150, 110);
      myGLCD.setColor(VGA_WHITE);
      myGLCD.fillRoundRect(41, 41, 149, 109);
      myGLCD.setBackColor(VGA_WHITE);
      myGLCD.setColor(VGA_BLACK);
      myGLCD.print("Automatico",55,70);
    
    }else{
  
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(40, 40, 150, 110);
      myGLCD.setColor(VGA_GRAY);
      myGLCD.fillRoundRect(41, 41, 149, 109);
      myGLCD.setBackColor(VGA_GRAY);
      myGLCD.setColor(255,255,255);
      myGLCD.print("Automatico",55,70);
    }

  if(selecionado==1){
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(170, 40, 280, 110);
      myGLCD.setColor(VGA_WHITE);
      myGLCD.fillRoundRect(171, 41, 279, 109);
      myGLCD.setBackColor(VGA_WHITE);
      myGLCD.setColor(VGA_BLACK);
      myGLCD.print("Manual",200,70);
    }else{
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(170, 40, 280, 110);
      myGLCD.setColor(VGA_GRAY);
      myGLCD.fillRoundRect(171, 41, 279, 109);
      myGLCD.setBackColor(VGA_GRAY);
      myGLCD.setColor(255,255,255);
      myGLCD.print("Manual",200,70);

    }

  if(selecionado==2){
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(40, 130, 150, 200);
      myGLCD.setColor(VGA_WHITE);
      myGLCD.fillRoundRect(41, 131, 149, 199);
      myGLCD.setBackColor(VGA_WHITE);
      myGLCD.setColor(VGA_BLACK);
      myGLCD.print("Bluetooth",60,160);
    }else{
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(40, 130, 150, 200);
      myGLCD.setColor(VGA_GRAY);
      myGLCD.fillRoundRect(41, 131, 149, 199);
      myGLCD.setBackColor(VGA_GRAY);
      myGLCD.setColor(255,255,255);
      myGLCD.print("Bluetooth",60,160);
      
    }
  if(selecionado==3){
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(170, 130, 280, 200);
      myGLCD.setColor(VGA_WHITE);
      myGLCD.fillRoundRect(171, 131, 279, 199);
      myGLCD.setBackColor(VGA_WHITE);
      myGLCD.setColor(VGA_BLACK);
      myGLCD.print("Configuracoes",175,160); 
    }
    else{
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(170, 130, 280, 200);
      myGLCD.setColor(VGA_GRAY);
      myGLCD.fillRoundRect(171, 131, 279, 199);
      myGLCD.setBackColor(VGA_GRAY);
      myGLCD.setColor(255,255,255);
      myGLCD.print("Configuracoes",175,160); 
    }
  }
  
}
void telaConfiguracoes(){
  myGLCD.fillScr(VGA_WHITE);
  titulos("selecione", tituloInf);
  nSelecoes=2;
  selecionado=0;
  sair = true;
  selAlterada= false;
  while (sair)
  {  
    do{
      if(digitalRead(PIN_ENTER)){
        switch (selecionado)
        {
        case 0:
          telaCalibracao();
          myGLCD.fillScr(VGA_WHITE);
          sair = true;
          selAlterada= false;
          break;
        case 1:
          /* code */
          break;
        case 2:
          /* code */
          break;
      
      }
      
      }
    }while(selAlterada);
    selAlterada= true;

    if(selecionado == 0){
       myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 20, 290, 80);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.fillRoundRect(31, 21, 289, 79);
       myGLCD.setBackColor(VGA_WHITE);
       myGLCD.setColor(VGA_BLACK);
       myGLCD.setFont(BigFont);
       myGLCD.print("Calibracao", CENTER , 42); 
    }else{
        myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 20, 290, 80);
       myGLCD.setColor(VGA_GRAY);
       myGLCD.fillRoundRect(31, 21, 289, 79);
       myGLCD.setBackColor(VGA_GRAY);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.setFont(BigFont);
       myGLCD.print("Calibração", CENTER , 42);
     }

     if(selecionado == 1){
      
       myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 90, 290, 150);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.fillRoundRect(31, 91, 289, 149);
       myGLCD.setBackColor(VGA_WHITE);
       myGLCD.setColor(VGA_BLACK);
       myGLCD.setFont(BigFont);
       myGLCD.print("Medio", CENTER , 112);
     }else{
      
       myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 90, 290, 150);
       myGLCD.setColor(VGA_GRAY);
       myGLCD.fillRoundRect(31, 91, 289, 149);
       myGLCD.setBackColor(VGA_GRAY);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.setFont(BigFont);
       myGLCD.print("Medio", CENTER , 112);
  
       
     }

     if(selecionado == 2){

       myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 160, 290, 220);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.fillRoundRect(31, 161, 289, 219);
       myGLCD.setBackColor(VGA_WHITE);
       myGLCD.setColor(VGA_BLACK);
       myGLCD.setFont(BigFont);
       myGLCD.print("Grande", CENTER , 182); 

     }else{
       
       myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 160, 290, 220);
       myGLCD.setColor(VGA_GRAY);
       myGLCD.fillRoundRect(31, 161, 289, 219);
       myGLCD.setBackColor(VGA_GRAY);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.setFont(BigFont);
       myGLCD.print("Grande", CENTER , 182); 
     }
  }
  
}

void telaCalibracao(){
  myGLCD.fillScr(VGA_WHITE);
  titulos("Calibracao", tituloInf);
  nSelecoes=2;
  selecionado=0;
  sair = true;
  selAlterada= false;

  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_WHITE);
  myGLCD.setFont(BigFont);
  myGLCD.print("Transmissor 1", CENTER , 21);
  myGLCD.drawRoundRect(35, 40, 290, 90);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.fillRoundRect(36, 41, 289, 89);
  myGLCD.setBackColor(VGA_WHITE);
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setFont(BigFont);
  myGLCD.print("Pa", 230 , 57);

  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_WHITE);
  myGLCD.setFont(BigFont);
  myGLCD.print("Transmissor 2", CENTER , 95);
  myGLCD.drawRoundRect(35, 113, 290, 163);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.fillRoundRect(36, 114, 289, 162);
  myGLCD.setBackColor(VGA_WHITE);
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setFont(BigFont);
  myGLCD.print("Pa", 230 , 130);

  while (sair)
  {
    do{ 
      myGLCD.setBackColor(VGA_WHITE);
      myGLCD.setColor(VGA_BLACK);
      myGLCD.setFont(BigFont);
      myGLCD.printNumF(pressao(oversampling1, zero, ftrScala1), 2, 100 , 57);
      myGLCD.printNumF(pressao(oversampling2, zero2, ftrScala2), 2, 100 , 130);

      if(digitalRead(PIN_ENTER)){
        switch (selecionado)
        {
        case 0:
          sair = false;
          selAlterada=false;
          break;
        case 1:
          /* code */
          break;
        case 2:
          /* code */
          break;
        }
      }
    }while (selAlterada);
    selAlterada= true;
    if(selecionado == 0){
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(15, 180, 95, 220);
      myGLCD.setColor(VGA_WHITE);
      myGLCD.fillRoundRect(16, 181, 94, 219);
      myGLCD.setBackColor(VGA_WHITE);
      myGLCD.setColor(VGA_BLACK);
      myGLCD.setFont(SmallFont);
      myGLCD.print("Voltar", 33 , 195); 
    }else{
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(15, 180, 95, 220);
      myGLCD.setColor(VGA_GRAY);
      myGLCD.fillRoundRect(16, 181, 94, 219);
      myGLCD.setBackColor(VGA_GRAY);
      myGLCD.setColor(VGA_WHITE);
      myGLCD.setFont(SmallFont);
      myGLCD.print("Voltar", 33 , 195);
    } 
    if(selecionado == 1){
          
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(119, 180, 199, 220);
      myGLCD.setColor(VGA_WHITE);
      myGLCD.fillRoundRect(120, 181, 198, 219);
      myGLCD.setBackColor(VGA_WHITE);
      myGLCD.setColor(VGA_BLACK);
      myGLCD.setFont(SmallFont);
      myGLCD.print("Calibr. 1", 125 , 195);
    }else{
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(119, 180, 199, 220);
      myGLCD.setColor(VGA_GRAY);
      myGLCD.fillRoundRect(120, 181, 198, 219);
      myGLCD.setBackColor(VGA_GRAY);
      myGLCD.setColor(VGA_WHITE);
      myGLCD.setFont(SmallFont);
      myGLCD.print("Calibr. 1", 125 , 195);
    }
    if(selecionado == 2){ 
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(223, 180, 303, 220);
      myGLCD.setColor(VGA_WHITE);
      myGLCD.fillRoundRect(224, 181, 302, 219);
      myGLCD.setBackColor(VGA_WHITE);
      myGLCD.setColor(VGA_BLACK);
      myGLCD.setFont(SmallFont);
      myGLCD.print("Calibr. 2", 229 , 195);
    }else{        
      myGLCD.setColor(VGA_BLACK);
      myGLCD.drawRoundRect(223, 180, 303, 220);
      myGLCD.setColor(VGA_GRAY);
      myGLCD.fillRoundRect(224, 181, 302, 219);
      myGLCD.setBackColor(VGA_GRAY);
      myGLCD.setColor(VGA_WHITE);
      myGLCD.setFont(SmallFont);
      myGLCD.print("Calibr. 2", 229 , 195);
    }
  }  
}


void telaManual(){
  myGLCD.fillScr(VGA_WHITE);
  titulos("Controle manual", tituloInf);
  nSelecoes=1;
  selecionado = 0;
  sair=true;
  selAlterada= false;

   
  while (sair){
    do{
       
        myGLCD.setFont(BigFont);
        myGLCD.setBackColor(VGA_WHITE);
        myGLCD.setColor(VGA_BLACK);
        myGLCD.print("       ", 70 , 47);
        myGLCD.printNumF(pressao(oversampling2, zero, ftrScala1),2, 70 , 47);
        myGLCD.print("       ", 70 , 132);
        myGLCD.printNumF(vazao(oversampling2,zero2,ftrScala2,fatorBocal[tamBocal]),2,70,132);




        if(digitalRead(PIN_ENTER)){
          if(selecionado==0)sair=false;
           if(selecionado==1){
          selecaobocal();
          myGLCD.fillScr(VGA_WHITE);
          sair = true;
          nSelecoes=1;
          selecionado = 0;
          }
        selAlterada=false;
      }

    }while(selAlterada);
    selAlterada=true;


    myGLCD.setColor(VGA_BLACK);
    myGLCD.drawRoundRect(30, 25, 290, 85);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.fillRoundRect(31, 26, 289, 84);
    myGLCD.setBackColor(VGA_WHITE);
    myGLCD.setColor(VGA_BLACK);
    myGLCD.print("Pa", 210 , 47);

    myGLCD.setColor(VGA_BLACK);
    myGLCD.drawRoundRect(30, 110, 290, 170);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.fillRoundRect(31, 111, 289, 169);
    myGLCD.setBackColor(VGA_WHITE);
    myGLCD.setColor(VGA_BLACK);
    myGLCD.print("m3/h", 200 , 132);
  

  if(selecionado == 0){

    myGLCD.setColor(VGA_BLACK);
    myGLCD.drawRoundRect(30, 185, 130, 220);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.fillRoundRect(31, 186, 129, 219);
    myGLCD.setFont(SmallFont);
    myGLCD.setBackColor(VGA_WHITE);
    myGLCD.setColor(VGA_BLACK);
    myGLCD.print("Voltar", 58 , 198);

    myGLCD.setColor(VGA_BLACK);
    myGLCD.drawRoundRect(190, 185, 290, 220);
    myGLCD.setColor(VGA_GRAY);
    myGLCD.fillRoundRect(191, 186, 289, 219);
    myGLCD.setFont(SmallFont);
    myGLCD.setBackColor(VGA_GRAY);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.print(butBocal[tamBocal], 222 , 198);
    }

  else{
    myGLCD.setColor(VGA_BLACK);
    myGLCD.drawRoundRect(30, 185, 130, 220);
    myGLCD.setColor(VGA_GRAY);
    myGLCD.fillRoundRect(31, 186, 129, 219);
    myGLCD.setFont(SmallFont);
    myGLCD.setBackColor(VGA_GRAY);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.print("Voltar", 58 , 198);

    myGLCD.setColor(VGA_BLACK);
    myGLCD.drawRoundRect(190, 185, 290, 220);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.fillRoundRect(191, 186, 289, 219);
    myGLCD.setFont(SmallFont);
    myGLCD.setBackColor(VGA_WHITE);
    myGLCD.setColor(VGA_BLACK);
    myGLCD.print(butBocal[tamBocal], 222, 198);
      }
  }
}
void oversampling(){
  // Faz a conversão de 256 amostras do sinal analogico
  amostragem1 += analogRead(TRANSMISSOR1); 
  amostragem2 += analogRead(TRANSMISSOR2);
  counterAmostragem--;
  
  if(counterAmostragem <= 0){
    // divide as amostras por 16
    amostragem1 >>= 4;
    amostragem2 >>= 4;
    //retira o valor para variavel global
    oversampling1 = float(amostragem1);
    oversampling2 = float(amostragem2);
    counterAmostragem = 256;//reinicia o contador 
    //zera as amostragem
    amostragem1 = 0; 
    amostragem2 = 0;
    }
}
void selecaobocal(){
  myGLCD.fillScr(VGA_WHITE);
  titulos("selecione o tamanho", tituloInf);
  nSelecoes=2;
  selecionado=0;
  sair = true;
  selAlterada= false;
  int x=0;
  while (sair)
  {
    do{
      if(digitalRead(PIN_ENTER)&&(x>1)){
        tamBocal=selecionado;
        
        sair = false;
        selAlterada=false;
      }
      x++;
    }while(selAlterada);
    selAlterada=true;
    if(selecionado == 0){
       myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 20, 290, 80);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.fillRoundRect(31, 21, 289, 79);
       myGLCD.setBackColor(VGA_WHITE);
       myGLCD.setColor(VGA_BLACK);
       myGLCD.setFont(BigFont);
       myGLCD.print("Pequeno", CENTER , 42); 
    }else{
        myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 20, 290, 80);
       myGLCD.setColor(VGA_GRAY);
       myGLCD.fillRoundRect(31, 21, 289, 79);
       myGLCD.setBackColor(VGA_GRAY);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.setFont(BigFont);
       myGLCD.print("Pequeno", CENTER , 42); 
     }

     if(selecionado == 1){
      
       myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 90, 290, 150);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.fillRoundRect(31, 91, 289, 149);
       myGLCD.setBackColor(VGA_WHITE);
       myGLCD.setColor(VGA_BLACK);
       myGLCD.setFont(BigFont);
       myGLCD.print("Medio", CENTER , 112);
     }else{
      
       myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 90, 290, 150);
       myGLCD.setColor(VGA_GRAY);
       myGLCD.fillRoundRect(31, 91, 289, 149);
       myGLCD.setBackColor(VGA_GRAY);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.setFont(BigFont);
       myGLCD.print("Medio", CENTER , 112);
  
       
     }

     if(selecionado == 2){

       myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 160, 290, 220);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.fillRoundRect(31, 161, 289, 219);
       myGLCD.setBackColor(VGA_WHITE);
       myGLCD.setColor(VGA_BLACK);
       myGLCD.setFont(BigFont);
       myGLCD.print("Grande", CENTER , 182); 

     }else{
       
       myGLCD.setColor(VGA_BLACK);
       myGLCD.drawRoundRect(30, 160, 290, 220);
       myGLCD.setColor(VGA_GRAY);
       myGLCD.fillRoundRect(31, 161, 289, 219);
       myGLCD.setBackColor(VGA_GRAY);
       myGLCD.setColor(VGA_WHITE);
       myGLCD.setFont(BigFont);
       myGLCD.print("Grande", CENTER , 182); 
     }
  }
  




}

void titulos(String sup,String inf){
  String superior = sup;
  String inferior = inf;
  
  myGLCD.setColor(VGA_RED); //cor do titulo superior
  myGLCD.fillRect(0, 0, 319, 13); 
  myGLCD.setColor(VGA_BLUE); //cor do titulo inferior
  myGLCD.fillRect(0, 226, 319, 239);
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(255, 0, 0);
  myGLCD.print(superior, CENTER, 1);
  myGLCD.setBackColor(VGA_BLUE);
  myGLCD.setColor(255,255,0);
  myGLCD.print(inferior, CENTER, 227);
}
 float pressao(float ovsa_p, int zero_p, float constante_p){
  float p_p = 0.0;
  ovsa_p -= zero_p;
  p_p = ovsa_p * constante_p;
  return p_p;  
  }

float vazao(float ovsa_v, int zero_v, float constante_v, float bocal_v){
  float p_v =0.0;
  float v =0.0;
  ovsa_v -= zero;
  p_v = ovsa_v * constante_v;
  v = sqrt(p_v);
  v *= bocal_v;
  return v;
}
void selecao(int ns){
  
  if(digitalRead(PIN_CIMA)){
    ultSelecionado=selecionado;
    selecionado--;
    selAlterada = false;
  }
  else if(digitalRead(PIN_BAIXO)){
    ultSelecionado=selecionado;
    selecionado++;    
    selAlterada = false;
  }

  if(selecionado<0)selecionado=ns;
  else if(selecionado>ns)selecionado=0;

}