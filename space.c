#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include<string.h>
#include<time.h>

#define WIDTH 800 
#define HEIGHT 600 

#define NUM_LEVEL 5
#define MAX_SPEED_X 300
#define MAX_SPEED_y 300

#define laser_speed 1000
#define alien_speed 200
#define mship_speed 100

#define MAX_SCORES 200
#define MAX_POWERS 20
#define SCORE_FILE "scoreboard.txt" 



bool paused=false;
bool game_over=false;
bool score_saved = false;
bool soundOn = true;
Texture2D soundOnlogo;
Texture2D soundOfflogo;

int scoreboard_count = 0;
float scrollOffset = 0;  //I will track how much the scoreboard list I have scrolled, measured in pixels.


struct ScoreEntry{
    char Name[50];
    int score;
} ;

struct ScoreEntry scoreboard_arr[MAX_SCORES];
struct ScoreEntry score_entry;


typedef enum {
    SCREEN_START,
    SCREEN_MENU,
    SCREEN_HELP,
    SCREEN_SETTINGS,
    SCREEN_LEADERBOARD,
    SCREEN_NAME_ENTRY,
    SCREEN_LEVEL_SELECT,
    SCREEN_GAMEPLAY,
    SCREEN_GAME_OVER
} Screen;


struct level
{
   Texture2D game_back[6];
   bool clicked;
   Vector2 alien_level_speed;
   Vector2 big_alien_freq;
   int flagtime;
};

struct level_button
{
    Texture2D button_pic;
    bool button_clicked;
    Rectangle rect;   
};



struct alien
{
  Texture2D picture;
  Vector2 Position;
  Vector2 Speed;
  int int_pos;
  bool active;
};


struct laser{
  Texture2D picture;
  
  Vector2 Position;
  Vector2 Speed;
  bool active;
  
};
//rlamia code
typedef enum Powertype{
  POWER_SHIELD,
  POWER_BUBBLE,
  POWER_LIFE,
  POWER_LASER,
  POWER_EXTRA_POINTS,
  POWER_TRIPLE_SHOT,
  POWER_MAGNET,
  POWER_SPEED_BOOST,
  POWER_RAPID_FIRE,
  POWER_MEGA_WEAPON,
}Powertype;

typedef struct Power{
    
    bool active;
    Vector2 positionpo;
    Vector2 velocitypo;
    Powertype typepo;

} Power;

static inline Power CreatePower(Vector2 position, Vector2 velocity, Powertype type)
    {
        return(Power)
        {
            .active=true,
            .positionpo=position,
            .velocitypo=velocity,
            .typepo=type,
    };
}

static inline void PowerUpdate(Power *p ,float frametime){
    if(!p->active){
        return;
    }
    p->positionpo = Vector2Add(p->positionpo, Vector2Scale(p->velocitypo, frametime));
}

//rlamiacode
bool is_pressed(Vector2 mousepos, bool mousepressed, Rectangle rect) {
    if (CheckCollisionPointRec(mousepos, rect) && mousepressed) {
        return true;
    }
    return false;
}


Music music;
Sound sound1;
Sound sound2;
Sound sound3;
Sound sound4;

void SaveScoreboard() {
    FILE *f = fopen(SCORE_FILE, "w");
    if (f == NULL) return;
    for (int i = 0; i < scoreboard_count; i++) {
        fprintf(f, "%s,%d\n", scoreboard_arr[i].Name, scoreboard_arr[i].score);
    }
    fclose(f);
}


void LoadScoreboard() {
    FILE *f = fopen(SCORE_FILE, "r");
    if (f == NULL) return; // first run,no file yet

    char line[80];
    scoreboard_count = 0;
    while (scoreboard_count < MAX_SCORES && fgets(line, sizeof(line), f)) {
        char *comma = strchr(line, ',');
        if (!comma) continue;
        *comma = '\0';
        strncpy(scoreboard_arr[scoreboard_count].Name, line, 49);
        scoreboard_arr[scoreboard_count].Name[49] = '\0';
        scoreboard_arr[scoreboard_count].score = atoi(comma + 1); // score normally  written in file as string , atoi converts ASCII to intergers
        scoreboard_count++;
    }
    fclose(f);
}




void InsertScoreSorted(const char *name, int score) {
    const char *finalName = (strlen(name) == 0) ? "Anonymous" : name;

    if (scoreboard_count >= MAX_SCORES && score <= scoreboard_arr[MAX_SCORES - 1].score) {
        return; // Score isn't high enough to make it to the list
    }

    int pos = scoreboard_count;
    if (pos >= MAX_SCORES) pos = MAX_SCORES - 1;

    while (pos > 0 && scoreboard_arr[pos - 1].score < score) {
        if (pos < MAX_SCORES) {
            scoreboard_arr[pos] = scoreboard_arr[pos - 1];
        }
        pos--;
    }

    strncpy(scoreboard_arr[pos].Name, finalName, 49);
    scoreboard_arr[pos].Name[49] = '\0';
    scoreboard_arr[pos].score = score;

    if (scoreboard_count < MAX_SCORES) {
        scoreboard_count++;
    }

    SaveScoreboard();
}



int main() 
{

  InitWindow(WIDTH, HEIGHT, "ASTRAL CONQUERERS");
  SetRandomSeed((unsigned int)time(NULL)); //without it the aliens aree generated randomly in the same pattern evry single time
  InitAudioDevice();
  LoadScoreboard();
  SetTargetFPS(60);
  
  //GameState currentState ;
  Screen currentScreen = SCREEN_START;

  int selectedLevel = -1;   
  int score =0;
  int letterCount = 0;
  int life=3;
 int highScore = (scoreboard_count > 0) ? scoreboard_arr[0].score : 0;

  score_entry.Name[0] = '\0'; 

  float ground = 98.0 * HEIGHT / 100;
  float w = 50;
  float h = 50;
  float animTimer = 0.0;
  float frameDuration = 0.25;  
  float alien_time= 1.0;
   int collisionnum=0;
   float colTimer = 0.0;
   const float colDuration = 1.0; // 1 second of invulnerability after a hit



   //game special effects r lamia
   float shieldTimer = 0.0;
const float shieldDuration = 5.0;   
float speedBoostTimer = 0.0;
const float speedBoostDuration = 4.0;
float rapidFireTimer = 0.0;
const float rapidFireDuration = 5.0;
float bubbleTimer = 0.0;
const float bubbleDuration = 5.0;  
float magnetTimer = 0.0;
const float magnetDuration = 5.0;  
float tripleShotTimer = 0.0;
const float tripleShotDuration = 3.0;
float megaWeaponTimer = 0.0;
const float megaWeaponDuration = 5.0;
float laserTimer = 0.0;
const float laserDuration = 5.0;
float shotCooldown = 0.0;
const float normalFireDelay = 0.3;
const float rapidFireDelay = 0.1;
// rlamia
  //bool laser_active=false;
  Texture2D spaceship;
  Texture2D mship;
  Texture2D Life;
  Texture2D Laser;
  //Texture2D background[6];
  
  Texture2D name;

  Texture2D start;
    Texture2D menu;
    Texture2D game;
    Texture2D score_header;
    Texture2D help_menu;
    Texture2D scoreboard;
    Texture2D startbutton;
    Texture2D playbutton;
    Texture2D helpbutton;
    Texture2D settingsbutton;
    Texture2D exitbutton;
    Texture2D leaderbutton;
    Texture2D backbutton;
    Texture2D enter_name;

    Texture2D Game_over_back;
    Texture2D game_over_text;
    Texture2D high_score_text;
    Texture2D your_score_text;
    Texture2D play_again;
    Texture2D Re_turn;
  

    //power texture
    Texture2D shield, extlife, extpoints, laser, megaweapon, rapidfire;
    Texture2D magnet, bubble, triple_shot, speed_boost;
    Texture2D powerTextures[10];
   
    Texture2D game_time_set_on=LoadTexture("assets/Settings/set_on.png");
    Texture2D game_time_soundon=LoadTexture("assets/Settings/sound.png");
    Texture2D game_time_soundoff=LoadTexture("assets/Settings/mute.png");
    Texture2D game_time_pause=LoadTexture("assets/Settings/pause.png");
    Texture2D game_time_end=LoadTexture("assets/Settings/game_finish.png");
    Texture2D game_time_start=LoadTexture("assets/Settings/pause.png");
    Texture2D game_time_set_off=LoadTexture("assets/Settings/setting.png");
  

  scoreboard=LoadTexture("assets/Background/Back5.png");
  score_header=LoadTexture("assets/Background/score_title.png");
  help_menu=LoadTexture("assets/Background/help_menu_new.png");
  spaceship=LoadTexture("assets/Spaceship/ship1.png");
  Laser=LoadTexture("assets/Laser/56.png");
  mship=LoadTexture("assets/Big_alien/alien1.png");

  
  Life = LoadTexture("assets/Life/Life5.png");
  music=LoadMusicStream("assets/Sound/shadow_operations-loop1.ogg");
  sound1=LoadSound("assets/Sound/alienshoot1.wav");
  sound2=LoadSound("assets/Sound/8bit_bomb_explosion.wav");
  sound3=LoadSound("assets/Sound/gameoverdark.wav");
   sound4=LoadSound("assets/Sound/powerup.wav");


  //power
   shield = LoadTexture("assets/power/shield.png");
  extlife = LoadTexture("assets/power/extra_life.png");
  extpoints = LoadTexture("assets/power/extra_points.png");
  laser = LoadTexture("assets/power/laser.png");
  megaweapon = LoadTexture("assets/power/mega_weapon.png");
  rapidfire = LoadTexture("assets/power/rapid_fire.png");
  magnet = LoadTexture("assets/power/magnet.png");
  bubble = LoadTexture("assets/power/bubble.png");
  triple_shot = LoadTexture("assets/power/triple_shot.png");
  speed_boost = LoadTexture("assets/power/speed_boost.png");

powerTextures[POWER_SHIELD]      = shield;
powerTextures[POWER_BUBBLE]      = bubble;
powerTextures[POWER_LIFE]        = extlife;
powerTextures[POWER_LASER]       = laser;
powerTextures[POWER_EXTRA_POINTS]= extpoints;
powerTextures[POWER_TRIPLE_SHOT] = triple_shot;
powerTextures[POWER_MAGNET]      = magnet;
powerTextures[POWER_SPEED_BOOST] = speed_boost;
powerTextures[POWER_RAPID_FIRE]  = rapidfire;
powerTextures[POWER_MEGA_WEAPON] = megaweapon;

//power rlamia

  Vector2 position = {(WIDTH)/2 -(w/2), ground};
  Vector2 positionl= {(WIDTH)/2 -(w/2), ground};
  Vector2 positiona={(WIDTH)/2 -(w/2), 0};
  Vector2 positionm={(WIDTH)/2 -(w/2), -3000};
  Vector2 position_life={(WIDTH)-w/2, 10};

   Vector2 positionstart = { WIDTH * 2/5, HEIGHT * 1 / 2 };
    Vector2 positionplay = { WIDTH * 1 / 3, HEIGHT * 1 / 11 };
    Vector2 positionhelp = { WIDTH * 1 / 3, HEIGHT * 3 / 11 };
    Vector2 positionset = { WIDTH * 1 / 3, HEIGHT * 5/ 11};
    Vector2 positionexit = { WIDTH * 1 / 3, HEIGHT * 7 / 11};
    Vector2 positionlead = { WIDTH * 1 / 3, HEIGHT * 9 / 11 };

    Rectangle startButtonRect = { positionstart.x, positionstart.y, WIDTH / 5, HEIGHT / 4.5};
    Rectangle playButtonRect = { positionplay.x, positionplay.y, WIDTH / 3, HEIGHT / 11 };
    Rectangle helpButtonRect = { positionhelp.x, positionhelp.y, WIDTH / 3, HEIGHT / 11};
    Rectangle settingsButtonRect = {positionset.x, positionset.y, WIDTH / 3, HEIGHT / 11 };
    Rectangle exitButtonRect = { positionexit.x, positionexit.y, WIDTH / 3, HEIGHT / 11};
    Rectangle leaderButtonRect = { positionlead.x, positionlead.y, WIDTH / 3, HEIGHT / 11};

    start = LoadTexture("assets/Background/Mainbg.png");
    menu = LoadTexture("assets/Background/back4.png");
    game = LoadTexture("assets/Space_pic/408-0.png");
    startbutton = LoadTexture("assets/Buttons/startbutton.png");
    playbutton = LoadTexture("assets/Buttons/Playbutton.png");
    helpbutton = LoadTexture("assets/Buttons/Helpbutton.png");
    settingsbutton = LoadTexture("assets/Buttons/settiingsbutton.png");
    exitbutton = LoadTexture("assets/Buttons/Exitbutton.png");
    leaderbutton = LoadTexture("assets/Buttons/Leaderboard.png");
    backbutton=LoadTexture("assets/Buttons/Back_button.png");
    name= LoadTexture("assets/Background/Back6.png");
    enter_name=LoadTexture("assets/Buttons/Enter_name.png");

    Game_over_back=LoadTexture("assets/Game_end/Game_over_back.png");
    game_over_text=LoadTexture("assets/Game_end/Game_over_new.png");
    high_score_text=LoadTexture("assets/Game_end/high_score_new.png");
    your_score_text=LoadTexture("assets/Game_end/your_score_new.png");
    play_again=LoadTexture("assets/Game_end/play_again.png");
    Re_turn=LoadTexture("assets/Game_end/Return.png");
    
    soundOfflogo=LoadTexture("assets/Settings/sound_off.png");
    soundOnlogo=LoadTexture("assets/Settings/sound_on.png");



  Vector2 speed = Vector2Zero();   //Vector2 Vector2Zero(void) {return (Vector2){ 0.0f, 0.0f };}
  Vector2 speedl = Vector2Zero();
  Vector2 speeda = Vector2Zero();
  Vector2 speedm = Vector2Zero();

PlayMusicStream(music); 
 int spriteIndex = 0;

   struct level Level_arr[5];
   for(int i=0;i<5;i++)
   {
    Level_arr[i].clicked=false;
    
  for (int j = 0; j < 6 ; j++) {
    char path[50];
    sprintf(path, "assets/Space_pic/level%d_Space%d.png", i + 1,j+1);
    Level_arr[i].game_back[j] = LoadTexture(path);
  } 


   }
struct level_button levelButtons[NUM_LEVEL];

   for (int i = 0; i < NUM_LEVEL; i++) {
    char path[50];
    sprintf(path, "assets/Levels/Level_%d.png", i + 1);
    levelButtons[i].button_pic = LoadTexture(path);

    levelButtons[i].button_clicked = false;
    levelButtons[i].rect = (Rectangle){
        WIDTH * 1 / 3,
        HEIGHT * (1 + i * 2) / 11,
        WIDTH / 3,
        HEIGHT / 11
    };
}
//power rlamia
Power powers[MAX_POWERS];

  for(int i=0; i<MAX_POWERS; i++){
    powers[i].active = false;
   }

  float powstartime= 0.0f;
  float pownextime=(float)GetRandomValue(1,100)/10.0f;

//rlamia

  struct laser laser_arr[10];
  for(int i=0;i<10;i++)
  {
    laser_arr[i].active=false;
    laser_arr[i].Speed.x = 0;
  }
  int j=-1;

  
 struct alien Alien[10];
Texture2D alienTex = LoadTexture("assets/Alien/alien10001.png");
int numAliens = 10;
int slotWidth = WIDTH/numAliens;

for (int k = 0; k < numAliens; k++) {
    Alien[k].picture = alienTex;
    Alien[k].Speed.x = 0;
    Alien[k].Speed.y = alien_speed;
    Alien[k].active = true;

    Alien[k].int_pos = -GetRandomValue(100, 900); 
    int slotStart = k * slotWidth;
    Alien[k].Position.x = slotStart + GetRandomValue(0, slotWidth - (int)w);
    Alien[k].Position.y = Alien[k].int_pos;
}
 


  while (!WindowShouldClose())
   {
    
  UpdateMusicStream(music);
  /*
  if(!paused || !game_over)
   {
    PlayMusicStream(music);
   }
  */
   if(paused)
   {
    PauseMusicStream(music);
   }
   else
   {
    ResumeMusicStream(music);
   }
   
   Vector2 mouseposition = GetMousePosition();

        bool mousepressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

     switch(currentScreen)
     {
         case SCREEN_START:

        if (is_pressed(mouseposition, mousepressed, startButtonRect))
        {
            currentScreen = SCREEN_MENU;
        }

        break;

       case SCREEN_MENU:

        if (is_pressed(mouseposition, mousepressed, playButtonRect))
        {
            letterCount = 0;
            score_entry.Name[0] = '\0';
            currentScreen = SCREEN_NAME_ENTRY;
        }

        else if (is_pressed(mouseposition, mousepressed, helpButtonRect))
        {
            currentScreen = SCREEN_HELP;
        }

        else if (is_pressed(mouseposition, mousepressed, settingsButtonRect))
        {
            currentScreen = SCREEN_SETTINGS;
        }

        else if (is_pressed(mouseposition, mousepressed, leaderButtonRect))
        {
            currentScreen = SCREEN_LEADERBOARD;
        }

        else if (is_pressed(mouseposition, mousepressed, exitButtonRect))
        {
            return 0;
        }

        break;
        case SCREEN_HELP:
    {
        Rectangle BackButtonRect ={ WIDTH/2 - 80,HEIGHT - 80,150,60};

        if (is_pressed(mouseposition, mousepressed, BackButtonRect))
        {
            currentScreen = SCREEN_MENU;
        }

        break;

     }
     case SCREEN_SETTINGS:
    {

     Rectangle BackButtonRect = { WIDTH/2 - 80, HEIGHT - 80, 150, 60 };
    //Rectangle soundButtonRect = { WIDTH/2 - 60, HEIGHT/2 - 60, 120, 120 };
    Rectangle soundOnButtonRect = { WIDTH/4 - 100, HEIGHT/2 - 60, 200, 120 };
    Rectangle soundOffButtonRect = { WIDTH*3/4 - 100, HEIGHT/2 - 60, 200, 120 };
         

    if (is_pressed(mouseposition, mousepressed, BackButtonRect))
    {
        currentScreen = SCREEN_MENU;
    }
    else if (is_pressed(mouseposition, mousepressed, soundOnButtonRect))
    {
        if(soundOn!=true)
        {
         soundOn = !soundOn;
        float vol = soundOn ? 1.0f : 0.0f;
        SetMusicVolume(music, vol);
        SetSoundVolume(sound1, vol);
        SetSoundVolume(sound2, vol);
        SetSoundVolume(sound3, vol);
        SetSoundVolume(sound4, vol);
        }
        
    }
    else if (is_pressed(mouseposition, mousepressed, soundOffButtonRect))
    {
        if(soundOn!=false)
        {
         soundOn = !soundOn;
        float vol = soundOn ? 1.0f : 0.0f;
        SetMusicVolume(music, vol);
        SetSoundVolume(sound1, vol);
        SetSoundVolume(sound2, vol);
        SetSoundVolume(sound3, vol);
        SetSoundVolume(sound4, vol); 
        }
        
    }


        break;
    }
    
    case SCREEN_LEADERBOARD:
    {
        Rectangle BackButtonRect = {WIDTH/2-80,HEIGHT-80,150,60};

        if (is_pressed(mouseposition, mousepressed, BackButtonRect))
        {
            currentScreen = SCREEN_MENU;
            scrollOffset = 0;
        }

        break;
    }
    case SCREEN_NAME_ENTRY:
    {
        int key=GetCharPressed(); //GetCharPressed() is a raylib function that returns the Unicode codepoint of a
                                   // character key that was pressed since the last frame

        while (key>0)
        {
            if ((key>=32) && (key<=125) && (letterCount<49))
            {
                score_entry.Name[letterCount]=(char)key;
                letterCount++;
                score_entry.Name[letterCount]='\0';
            }

            key = GetCharPressed();
        }
    

        if (IsKeyPressed(KEY_BACKSPACE))
        {
            if (letterCount > 0)
            {
                letterCount--;

                score_entry.Name[letterCount] = '\0';
            }
        }


        if (IsKeyPressed(KEY_ENTER) && letterCount>0)
        {
            selectedLevel = -1;
            currentScreen = SCREEN_LEVEL_SELECT;
        }

        break;
    }
        case SCREEN_LEVEL_SELECT:

        for (int i=0;i<NUM_LEVEL;i++)
        {
            if (is_pressed(mouseposition, mousepressed,levelButtons[i].rect))
            {
                selectedLevel = i;
                levelButtons[i].button_clicked = true;
                 Level_arr[i].clicked = true;


                 //reset  korlam the whole game, other wise game will stay paused
                    game_over = false;
                    score_saved = false;
                    paused = false;
                    score = 0;
                    life = 3;
                    collisionnum = 0;
                    colTimer = 0.0f;

                    shieldTimer = 0.0f;
                    bubbleTimer = 0.0f;
                    laserTimer = 0.0f;
                    tripleShotTimer = 0.0f;
                    magnetTimer = 0.0f;
                    speedBoostTimer = 0.0f;
                    rapidFireTimer = 0.0f;
                    megaWeaponTimer = 0.0f;
                    shotCooldown = 0.0f;

                     position = (Vector2){ (WIDTH)/2 - (w/2), ground };
                     speed = Vector2Zero();
                    positionm = (Vector2){ (WIDTH)/2 - (w/2), -3000 };
                     speedm = Vector2Zero();

                     for (int k = 0; k < 10; k++) {
                     laser_arr[k].active = false;
                     laser_arr[k].Speed.x = 0;
                        }
                    j = -1;

                 for (int k = 0; k < MAX_POWERS; k++) powers[k].active = false;
                 powstartime = 0.0f;
                 pownextime = (float)GetRandomValue(1, 100) / 10.0f;

                 for (int k = 0; k < numAliens; k++) {
                 Alien[k].active = true;
                 Alien[k].int_pos = -GetRandomValue(100, 900);
                 int slotStart = k * slotWidth;
                 Alien[k].Position.x = slotStart + GetRandomValue(0, slotWidth - (int)w);
                 Alien[k].Position.y = Alien[k].int_pos;
                 }

                animTimer = 0.0f;
                spriteIndex = 0;

                currentScreen = SCREEN_GAMEPLAY;
                break;
            }
        }

        break;
       
         case SCREEN_GAMEPLAY:

        if (IsKeyPressed(KEY_P))
        {
            paused = !paused;
        }

        break;

        case SCREEN_GAME_OVER:
    {
        Rectangle returnButtonRect ={ WIDTH*3/4 - 80,HEIGHT - 80,150,60};
        Rectangle play_again_ButtonRect ={ WIDTH/4 - 80,HEIGHT - 80,150,60};
        if (is_pressed(mouseposition, mousepressed, returnButtonRect))
        {
            currentScreen = SCREEN_MENU;
        }
        
        else if (is_pressed(mouseposition, mousepressed, play_again_ButtonRect))
        {
           
    game_over = false;
    score_saved = false;
    paused = false;
    score = 0;
    life = 3;
    collisionnum = 0;
    colTimer = 0.0;

     shieldTimer = 0.0f;
    bubbleTimer = 0.0f;
    laserTimer = 0.0f;
    tripleShotTimer = 0.0f;
    magnetTimer = 0.0f;
    speedBoostTimer = 0.0f;
    rapidFireTimer = 0.0f;
    megaWeaponTimer = 0.0f;
    shotCooldown = 0.0f;
    for (int k = 0; k < MAX_POWERS; k++) powers[k].active = false;
    powstartime = 0.0f;
    pownextime = (float)GetRandomValue(1, 100) / 10.0f;

    
    position = (Vector2){ (WIDTH)/2 - (w/2), ground };
    speed = Vector2Zero();

    
    positionm = (Vector2){ (WIDTH)/2 - (w/2), -3000 };
    speedm = Vector2Zero();

    
    for (int i = 0; i < 10; i++) {
        laser_arr[i].active = false;
        laser_arr[i].Speed.x = 0;
    }
    j = -1;

    for (int k = 0; k < numAliens; k++) {
        Alien[k].active = true;
        Alien[k].int_pos = -GetRandomValue(100, 900);
        int slotStart = k * slotWidth;
        Alien[k].Position.x = slotStart + GetRandomValue(0, slotWidth - (int)w);
        Alien[k].Position.y = Alien[k].int_pos;
    }

    
    animTimer = 0.0f;
    spriteIndex = 0;
     currentScreen = SCREEN_GAMEPLAY;
        }
       
        break;
    }
}

     

  
  if (currentScreen == SCREEN_GAMEPLAY)
{
    if(!paused && !game_over)

  {
  float dt = GetFrameTime();

  if (colTimer > 0.0) {
    colTimer -= dt;
}

    animTimer += dt;
if (animTimer >= frameDuration) {
    animTimer = 0.0f;
    spriteIndex = (spriteIndex + 1) % 6;
}
//power rlamia
if (shieldTimer > 0.0) shieldTimer -= dt;
if (bubbleTimer > 0.0) bubbleTimer -= dt;
if (laserTimer > 0.0) laserTimer -= dt;
if (tripleShotTimer > 0.0) tripleShotTimer -= dt;
if (magnetTimer > 0.0) magnetTimer -= dt;
if (speedBoostTimer > 0.0) speedBoostTimer -= dt;
if (rapidFireTimer > 0.0) rapidFireTimer -= dt;
if (megaWeaponTimer > 0.0) megaWeaponTimer -= dt;
if (shotCooldown > 0.0) shotCooldown -= dt;

//power rlamia
    position = Vector2Add(position, Vector2Scale(speed, dt));
    
    for (int i = 0; i < 10; i++) {
        if (!Alien[i].active) continue;  
        Alien[i].Position = Vector2Add(Alien[i].Position, Vector2Scale(Alien[i].Speed, dt));
        if(Alien[i].Position.y>600)
        {
          Alien[i].Position.y=Alien[i].int_pos;
        }
        }
    speeda.y=alien_speed;
    positiona = Vector2Add(positiona, Vector2Scale(speeda, dt));  


   speedm.y=mship_speed;
   positionm = Vector2Add(positionm, Vector2Scale(speedm, dt)); 
   if(positionm.y>800)
   {
      positionm.y=-3000;
      positionm.x=(int)(positionm.x+(WIDTH/3))%WIDTH;
   }


   if (position.x<0) {
      position.x=0;
    } else if (position.x+w>WIDTH)
      position.x = WIDTH - w;

    if (position.y < h) {
      position.y = h;
    } else if (position.y  >HEIGHT )
      position.y = HEIGHT;
   
    //power rlamia
     powstartime+=dt;

    if(powstartime>=pownextime){
    powstartime=0.0f;
    pownextime=(float)GetRandomValue(1,100)/10.0f;
   
   for(int i=0;i<MAX_POWERS; i++){
    if(!powers[i].active){
      Vector2 positionpo={(float)GetRandomValue(0,WIDTH), 0};
      Vector2 velocity={ (float)GetRandomValue(-30,30),(float)GetRandomValue(60,150) };
      powers[i] = CreatePower(positionpo, velocity, (Powertype)GetRandomValue(0,9));
      break;
    }
   } 
  }
  for(int i=0; i<MAX_POWERS;i++){
    PowerUpdate(&powers[i], dt);
    if(powers[i].positionpo.y>HEIGHT) {
      powers[i].active= false;
    } }
    //rlamia
 if (magnetTimer > 0.0) {
    float pullRadius=200.0;
    float pullStrength=250.0;
    Vector2 shipPos={position.x+w/2,position.y-h/2};
    for (int i = 0; i < MAX_POWERS; i++) {
        if (!powers[i].active) continue;
        Vector2 toShip = Vector2Subtract(shipPos,powers[i].positionpo);
        float dist = Vector2Length(toShip);
        if (dist < pullRadius && dist > 1.0f) {
            Vector2 pull = Vector2Scale(Vector2Normalize(toShip), pullStrength * dt);
            powers[i].positionpo = Vector2Add(powers[i].positionpo, pull);
        }
    }
} 
//rlamia
  
  float speedMultiplier=(speedBoostTimer>0.0f)?1.6f:1.0f;
  //rlamia



    if (IsKeyDown(KEY_RIGHT)) {
      //speed.x = MAX_SPEED_X;
       speed.x = MAX_SPEED_X * speedMultiplier;//rlamia
    } else if (IsKeyDown(KEY_LEFT)) {
      //speed.x = -MAX_SPEED_X;
      speed.x = -MAX_SPEED_X * speedMultiplier;//rlamia
    }
    else if(IsKeyDown(KEY_UP))
    {
       // speed.y=-MAX_SPEED_y;
        speed.y=-MAX_SPEED_y * speedMultiplier;//rlamia
    }
    else if(IsKeyDown(KEY_DOWN))
    {
       // speed.y=MAX_SPEED_y;
         speed.y=MAX_SPEED_y * speedMultiplier;//rlamia
    }
    else {
      speed.x = 0;
      speed.y=0;
    }

    if (IsKeyPressed(KEY_SPACE)) {
     if (shotCooldown <= 0.0f) {
     if (tripleShotTimer > 0.0f) {
        float offsets[3]={ -20.0f, 0.0f, 20.0f };
        for (int s = 0; s < 3; s++) {
            j = (j + 1) % 10;
            laser_arr[j].Position = (Vector2){ position.x + offsets[s], position.y };
            laser_arr[j].Speed = (Vector2){ 0, -laser_speed };
            laser_arr[j].active = true;
        }
    } else {
        j = (j + 1) % 10;
        laser_arr[j].Position = position;
        laser_arr[j].Speed.y = -laser_speed;
        laser_arr[j].active = true;
    }
    PlaySound(sound1);
    shotCooldown = (rapidFireTimer>0.0f) ? rapidFireDelay : normalFireDelay;
     // rlamia for shotcooldown, if prob dlt it
    } 
}
    for (int i = 0; i < 10; i++) {
    if (laser_arr[i].active) {
        laser_arr[i].Position = Vector2Add(laser_arr[i].Position, Vector2Scale(laser_arr[i].Speed, dt));
        if (laser_arr[i].Position.y < 0) {
            laser_arr[i].active = false;
        }
    }
}
  

   

  
Vector2 shipcenter={position.x+w/2,position.y+h/2};
float shipradius=h/2; 

//rlamia
for (int i=0;i<MAX_POWERS;i++) {
    if (!powers[i].active) continue;

    Vector2 powercenter = {powers[i].positionpo.x + 50/2,powers[i].positionpo.y + 50/2};
    float powerradius =80/2;

    if (CheckCollisionCircles(shipcenter,shipradius,powercenter,powerradius)) {
        powers[i].active =false;
        PlaySound(sound4);

        switch(powers[i].typepo) {
            case POWER_SHIELD:
                shieldTimer = shieldDuration;
                break;
            case POWER_BUBBLE:
                bubbleTimer = bubbleDuration;
                break;
            case POWER_LIFE:
            if(life<3){
                life++;
               }
                break;
            case POWER_LASER:
                laserTimer=laserDuration;
                break;
            case POWER_EXTRA_POINTS:
                score+=20; 
                break;
            case POWER_TRIPLE_SHOT:
                tripleShotTimer=tripleShotDuration;
                break;
            case POWER_MAGNET:
                magnetTimer=magnetDuration;
                break;
            case POWER_SPEED_BOOST:
                speedBoostTimer=speedBoostDuration;
                break;
            case POWER_RAPID_FIRE:
                rapidFireTimer=rapidFireDuration;
                break;
            case POWER_MEGA_WEAPON:
                for (int k = 0; k < 10; k++) {
                if (Alien[k].active) {
                Alien[k].Position.y = Alien[k].int_pos;
                score+=5;
               }
               }
                PlaySound(sound2);
                break;
        }
    }
}
//rlamia
  
Vector2 mshipCenter={positionm.x+w,positionm.y+h*2.5/2};
float mshipRadius=h*2.5/2;

for (int i = 0; i < 10; i++) {
    if (!laser_arr[i].active) continue;

    
    Vector2 laserCenter = {
        laser_arr[i].Position.x + w/2.5 + (w/5.0)/2.0,
        laser_arr[i].Position.y -(h*1.5)+(h/2.0)/2.0
    };
    float laserRadius = (w/5.0)/2.0;
     if (CheckCollisionCircles(laserCenter,laserRadius,mshipCenter,mshipRadius))
        {
             PlaySound(sound2);
            laser_arr[i].active = false;
            score+=50;
            collisionnum++;
            if(collisionnum>10)
            {
              positionm= (Vector2){(WIDTH)/2 -(w/2), -3000};
              collisionnum=0;
            }   // sent it back to the top instead of killing it forever
            continue;
            
        }

       
         
//rlamia
    for (int k = 0; k < 10; k++) {
        if (!Alien[k].active) continue;

        Vector2 alienCenter = {
            Alien[k].Position.x + (w/1.5)/2.0,
            Alien[k].Position.y + (h/1.5)/2.0
        };
        float alienRadius = (w/1.5)/2.0;

        if (CheckCollisionCircles(laserCenter,laserRadius,alienCenter,alienRadius))
        {
             PlaySound(sound2);
            laser_arr[i].active = false;
            Alien[k].Position.y = Alien[k].int_pos;  
            score+=5;
            break;
            
        }
    
    }
}
 //rlamia
  if (laserTimer > 0.0f) {
    Rectangle laserBeam ={ position.x+w/2-5,0,10,position.y};
    for (int k = 0; k < 10; k++) {
        if (!Alien[k].active) continue;
        Rectangle alienRect={ Alien[k].Position.x, Alien[k].Position.y,w/1.5,h/1.5 };
        if (CheckCollisionRecs(laserBeam, alienRect)) {
            Alien[k].Position.y=Alien[k].int_pos;
            score+=5;
        }
    }
}
  


for (int k = 0; k < 10; k++) {
        if (!Alien[k].active) continue;

        Vector2 alienCenter = {Alien[k].Position.x + (w/1.5)/2.0,Alien[k].Position.y + (h/1.5)/2.0};
        float alienRadius = (w/1.5)/2.0;
if (colTimer <=0 && CheckCollisionCircles(shipcenter,shipradius,alienCenter,alienRadius))
        {
           if (shieldTimer > 0.0f) {
            shieldTimer = 0.0f;      
            colTimer = colDuration;
        }
        else if (bubbleTimer > 0.0f) {
            bubbleTimer = 0.0f;     
            colTimer = colDuration;
        }
        else {
            PlaySound(sound2);
            life--;
            position = (Vector2){ (WIDTH)/2 - (w/2), ground };
            colTimer = colDuration;
        }
        break;

      }
      }
  if (colTimer<=0 && CheckCollisionCircles(shipcenter, shipradius, mshipCenter, mshipRadius))
{
     if (shieldTimer > 0.0f) {
        shieldTimer = 0.0f;
        colTimer = colDuration;
    }
    else if (bubbleTimer > 0.0f) {
        bubbleTimer = 0.0f;
        colTimer = colDuration;
    }
    else {
        PlaySound(sound2);
        life = 0;
        colTimer = colDuration;
    }

}

 if (life <= 0 && !score_saved)
{
    InsertScoreSorted(score_entry.Name, score);
    score_saved = true;
    game_over = true;
   highScore = (scoreboard_count>0)?scoreboard_arr[0].score:0; 
     PlaySound(sound3);
   currentScreen = SCREEN_GAME_OVER;
}
/*
if (life <= 0) {

       game_over=true;
       

    }
*/

    
 }
}
  
  

   
   
   

    BeginDrawing();
    ClearBackground((Color){ 80, 80, 80, 255 } );

    switch (currentScreen)
  {
    
    case SCREEN_START:

       DrawTexturePro(start,
                       (Rectangle){0, 0, start.width, start.height},
                       (Rectangle){0, 0, WIDTH, HEIGHT},
                       Vector2Zero(), 0, WHITE);

        DrawTexturePro(startbutton,
                       (Rectangle){0, 0, startbutton.width, startbutton.height},
                       startButtonRect,
                       Vector2Zero(), 0, WHITE);

        break;
    case SCREEN_MENU:

       DrawTexturePro(menu,
                       (Rectangle){0, 0, menu.width, menu.height},
                       (Rectangle){0, 0, WIDTH, HEIGHT},
                       Vector2Zero(), 0, WHITE);
                     
             DrawTexturePro(playbutton,
                       (Rectangle){0, 0, playbutton.width, playbutton.height},
                       playButtonRect,
                       Vector2Zero(), 0, WHITE);
              DrawTexturePro(helpbutton,
                       (Rectangle){0, 0,helpbutton.width, helpbutton.height},
                       helpButtonRect,
                       Vector2Zero(), 0, WHITE);
               DrawTexturePro(settingsbutton,
                       (Rectangle){0, 0, settingsbutton.width, settingsbutton.height},
                        settingsButtonRect,
                       Vector2Zero(), 0, WHITE);

                 DrawTexturePro(exitbutton,
                       (Rectangle){0, 0,exitbutton.width,exitbutton.height},
                       exitButtonRect,
                       Vector2Zero(), 0, WHITE);

                 DrawTexturePro(leaderbutton,
                       (Rectangle){0, 0, leaderbutton.width, leaderbutton.height},
                       leaderButtonRect,
                       Vector2Zero(), 0, WHITE);
            break;

    case SCREEN_HELP:
    {
        Rectangle BackButtonRect = { WIDTH/2 - 80, HEIGHT - 80, 150, 60};
    DrawTexturePro(help_menu,
                   (Rectangle){0, 0, help_menu.width, help_menu.height},
                   (Rectangle){0, 0, WIDTH, HEIGHT},
                   Vector2Zero(), 0, WHITE);

    DrawTexturePro(backbutton,
                   (Rectangle){0, 0, backbutton.width, backbutton.height},
                   BackButtonRect,
                   Vector2Zero(), 0, WHITE);

        break;
    }

     case SCREEN_SETTINGS:
    {
        

        DrawTexturePro(
            menu,
            (Rectangle){0, 0, menu.width, menu.height},
            (Rectangle){0, 0, WIDTH, HEIGHT},
            Vector2Zero(),
            0,
            WHITE
        );

        DrawText("SETTINGS",WIDTH/2 - MeasureText("SETTINGS",40)/2,100,40, WHITE);
        
         Rectangle soundOnButtonRect = { WIDTH/4-100, HEIGHT/2-60,200,120};

          DrawTexturePro(soundOnlogo,
            (Rectangle){0, 0, soundOnlogo.width, soundOnlogo.height},
        soundOnButtonRect,
        Vector2Zero(), 0, WHITE
         );

         Rectangle soundOffButtonRect = { WIDTH*3/4 - 100, HEIGHT/2 - 60, 200, 120 };
         

         DrawTexturePro(soundOfflogo,
            (Rectangle){0, 0, soundOfflogo.width, soundOfflogo.height},
        soundOffButtonRect,
        Vector2Zero(), 0, WHITE
         );
        /*
       
        Texture2D currentSoundIcon = soundOn ? soundOnIcon : soundOffIcon;

        DrawTexturePro(
        currentSoundIcon,
        (Rectangle){0, 0, currentSoundIcon.width, currentSoundIcon.height},
        soundButtonRect,
        Vector2Zero(), 0, WHITE
         );

        */
        
        Rectangle BackButtonRect =
        { WIDTH/2 - 80,HEIGHT - 80,150,60};

        DrawTexturePro(
            backbutton,
            (Rectangle){0, 0,
                         backbutton.width,
                         backbutton.height},
            BackButtonRect,
            Vector2Zero(),
            0,
            WHITE
        );

        break;
    }


    
    case SCREEN_LEADERBOARD:
    {

       DrawTexturePro(scoreboard,
                       (Rectangle){0, 0, scoreboard.width,scoreboard.height},
                       (Rectangle){0, 0, WIDTH, HEIGHT},
                       Vector2Zero(), 0, WHITE);

        
        DrawTexturePro(score_header,
                       (Rectangle){0, 0, score_header.width,score_header.height},
                       (Rectangle){WIDTH/5, 30, WIDTH/2 +20, HEIGHT/10 +10},
                       Vector2Zero(), 0, WHITE);

        Rectangle BackButtonRect = { WIDTH/2 - 80, HEIGHT - 80, 150, 60};
     DrawTexturePro(backbutton,
                   (Rectangle){0, 0, backbutton.width, backbutton.height},
                   BackButtonRect,
                   Vector2Zero(), 0, WHITE);

       Rectangle listArea = { WIDTH*16/80, HEIGHT*11/60, WIDTH*44/80, HEIGHT*42/60 };

if (CheckCollisionPointRec(GetMousePosition(), listArea)) {
    scrollOffset -= GetMouseWheelMove()*30;
}

int rowHeight = HEIGHT*5/60;
float contentHeight = scoreboard_count * rowHeight;
float maxScroll = contentHeight - listArea.height;
if (maxScroll < 0) maxScroll = 0;
if (scrollOffset < 0) scrollOffset = 0;
if (scrollOffset > maxScroll) scrollOffset = maxScroll;

BeginScissorMode((int)listArea.x, (int)listArea.y, (int)listArea.width, (int)listArea.height);
for (int i = 0; i < scoreboard_count; i++) {
    float rowY = listArea.y + i * rowHeight - scrollOffset;
    if (rowY + rowHeight < listArea.y || rowY > listArea.y + listArea.height) continue; // skip offscreen rows

    Rectangle row = { listArea.x, rowY, listArea.width, rowHeight - 5 };
    DrawRectangleRounded(row, 0.3, 8, (Color){200,200,200,255});
    DrawRectangleRoundedLinesEx(row, 0.3, 8, 2, (Color){10,17,40,255});
    DrawText(TextFormat("%d. %s", i+1, scoreboard_arr[i].Name), row.x+15, row.y+10, 20, DARKBLUE);
    DrawText(TextFormat("%d", scoreboard_arr[i].score), row.x + row.width - 80, row.y+10, 20, MAROON);
}
EndScissorMode(); //ScissorMode only draw pixels inside this rectangle,nahole full background scroll er part e chole ashbe
        break;
    }


   
    case SCREEN_NAME_ENTRY:
    {
        int max_word_length=15;
          DrawTexturePro(name,
                   (Rectangle){0, 0, name.width, name.height},
                   (Rectangle){0, 0, WIDTH, HEIGHT},
                   Vector2Zero(), 0, WHITE);

    float box_ratio = (float)enter_name.width / enter_name.height;
    float boxW = WIDTH * 0.4f;             
    float boxH = boxW / box_ratio;

    Rectangle nameBox = {WIDTH/2 - boxW/2,HEIGHT/3,boxW,boxH};

     DrawTexturePro(enter_name,
               (Rectangle){0, 0, enter_name.width, enter_name.height},
               nameBox,
               Vector2Zero(), 0, WHITE);
    
           Rectangle box = {WIDTH/2 - 300/2,nameBox.y + nameBox.height + 5,300,100};


         Color neonBlue   = (Color){ 60, 100, 255, 255 };
         Color neonPurple = (Color){ 170, 50, 255, 255 };

DrawRectangleRec(box, Fade(neonBlue, 0.15f));

for (int i = 4; i >= 1; i--) {
    Rectangle glowRect = {
        box.x - i * 2,
        box.y - i * 2,
        box.width + i * 4,
        box.height + i * 4
    };
    Color glowColor = ColorLerp(neonBlue, neonPurple, (float)i/4.0f);
    DrawRectangleLinesEx(glowRect, 2, Fade(glowColor, 0.08f * (5 - i)));
}

DrawRectangleLinesEx(box,3,neonBlue);
    int padding = 15;
  DrawText(score_entry.Name,
         box.x + padding,
         box.y + (box.height - 30) / 2,  
         30,
         WHITE);
        break;}


    case SCREEN_LEVEL_SELECT:
   {
         DrawTexturePro(menu,
                       (Rectangle){0, 0, menu.width, menu.height},
                       (Rectangle){0, 0, WIDTH, HEIGHT},
                       Vector2Zero(), 0, WHITE);


        for (int i = 0; i < NUM_LEVEL; i++) {
     DrawTexturePro(levelButtons[i].button_pic,
                   (Rectangle){0, 0, levelButtons[i].button_pic.width, levelButtons[i].button_pic.height},
                   levelButtons[i].rect,
                   Vector2Zero(), 0, WHITE);
      }

        break;
    }

    
    case SCREEN_GAMEPLAY:
      {
     for(int i=0;i<NUM_LEVEL;i++)
      {
       if(levelButtons[i].button_clicked == true)
       {
           DrawTexturePro(Level_arr[i].game_back[spriteIndex],
                   (Rectangle){0, 0, Level_arr[i].game_back[spriteIndex].width,Level_arr[i].game_back[spriteIndex].height},
                   (Rectangle){0, 0,WIDTH, HEIGHT},
                   Vector2Zero(), 0, WHITE);
       } 
      
    }
      
    position_life.x = WIDTH - w/2;
    for(int i=0;i<life;i++)
    {
        DrawTexturePro(Life,
                   (Rectangle){0, 0, Life.width, Life.height},
                   (Rectangle){position_life.x, position_life.y,30,30},
                   Vector2Zero(),0, WHITE);

        position_life.x-=(35);
    }

    DrawTexturePro(spaceship,
                   (Rectangle){0, 0, spaceship.width, spaceship.height},
                   (Rectangle){position.x, position.y-h,w,h},
                   Vector2Zero(), 0, WHITE);
    //rlamia
     if (laserTimer > 0.0f) {
              DrawRectangle((int)(position.x + w/2 - 5), 0, 10, (int)position.y, (Color){0, 255, 255, 180});
             }   
     if (shieldTimer > 0.0f) {
    float pad = 15.0f;  
    DrawTexturePro(shield,
                   (Rectangle){0, 0, shield.width, shield.height},
                   (Rectangle){position.x - pad, position.y - h - pad, w + pad*2, h + pad*2},
                   Vector2Zero(), 0, WHITE);
    }

if (bubbleTimer > 0.0f) {
    float pad = 15.0f;
    DrawTexturePro(bubble,
                   (Rectangle){0, 0, bubble.width, bubble.height},
                   (Rectangle){position.x - pad, position.y - h - pad, w + pad*2, h + pad*2},
                   Vector2Zero(), 0, WHITE);
    }
    //rlamia


    for (int i = 0; i < 10; i++) {
    if (laser_arr[i].active) {
        DrawTexturePro(Laser,
                       (Rectangle){0, 0, Laser.width, Laser.height},
                       (Rectangle){laser_arr[i].Position.x +w/2.5, laser_arr[i].Position.y - (h*1.5), w/5, h/2},
                       Vector2Zero(), 0, WHITE);
    }
    }
    for (int i = 0; i < 10; i++) {
        if (!Alien[i].active) continue;  
        DrawTexturePro(Alien[i].picture,
                       (Rectangle){0, 0, Alien[i].picture.width, Alien[i].picture.height},
                       (Rectangle){Alien[i].Position.x, Alien[i].Position.y, w/1.5, h/1.5},
                       Vector2Zero(), 0, WHITE);
    
    }
     //rlamia
      for (int i = 0; i < MAX_POWERS; ++i)
     {
        if(powers[i].active){
          DrawTexturePro(powerTextures[powers[i].typepo],
    (Rectangle){0,0, powerTextures[powers[i].typepo].width, powerTextures[powers[i].typepo].height},
    (Rectangle){powers[i].positionpo.x, powers[i].positionpo.y, 50, 50},
    Vector2Zero(), 0, WHITE);
        }
      }

      //rlamia
       DrawTexturePro(mship,
                   (Rectangle){0, 0, mship.width, mship.height},
                   (Rectangle){positionm.x, positionm.y, w*2, h*3},
                   Vector2Zero(), 0, WHITE);
     Rectangle groundRect = {WIDTH/80, HEIGHT/60, 200, 35};
     
    DrawRectangleRec(groundRect,BLUE); 
    DrawRectangleLinesEx(groundRect, 3, DARKPURPLE);

     DrawText(TextFormat("SCORE-%04d", score), WIDTH/80 +2, HEIGHT/60 +2, 30, WHITE);//(const char *text, int posX, int posY, int fontSize, Color color); 
     

    
     if(paused)
    {
      DrawText("Paused", WIDTH/2 -30, HEIGHT/2, 20, YELLOW);

    }

    
     
     }

        
        break;


    case SCREEN_GAME_OVER:
    {
     DrawTexturePro(Game_over_back,
                       (Rectangle){0, 0, Game_over_back.width, Game_over_back.height},
                       (Rectangle){0, 0, WIDTH, HEIGHT},
                       Vector2Zero(), 0, WHITE);
                       
     float ratio1=(float)game_over_text.width / game_over_text.height;
     Rectangle gameOverRect = { WIDTH/2 - WIDTH*0.5f/2, HEIGHT * 0.08f, WIDTH*0.5f, WIDTH*0.5f/ratio1 };

     DrawTexturePro(game_over_text,
                       (Rectangle){0, 0, game_over_text.width, game_over_text.height},
                       (Rectangle) gameOverRect,
                       Vector2Zero(), 0,WHITE);

    float ratio2 = (float)your_score_text.width / your_score_text.height;
        Rectangle yourScoreRect = { WIDTH/4 - WIDTH*0.3f/2, HEIGHT/2, WIDTH*0.3f, WIDTH*0.3f/ratio2 };
        DrawTexturePro(your_score_text,
                       (Rectangle){0, 0, your_score_text.width, your_score_text.height},
                       yourScoreRect, Vector2Zero(), 0, WHITE);

        float ratio3 = (float)high_score_text.width / high_score_text.height;
        Rectangle highScoreRect = { WIDTH*3/4 - WIDTH*0.3f/2, HEIGHT/2, WIDTH*0.3f, WIDTH*0.3f/ratio3 };
        DrawTexturePro(high_score_text,
                       (Rectangle){0, 0, high_score_text.width, high_score_text.height},
                       highScoreRect, Vector2Zero(), 0, WHITE);

        DrawText(TextFormat("%d", score),
                 yourScoreRect.x + yourScoreRect.width/2 - MeasureText(TextFormat("%d", score), 50)/2,
                 yourScoreRect.y + yourScoreRect.height + 10, 50, WHITE);

        DrawText(TextFormat("%d", highScore),
                 highScoreRect.x + highScoreRect.width/2 - MeasureText(TextFormat("%d", highScore), 50)/2,
                 highScoreRect.y + highScoreRect.height + 10, 50, WHITE);

          Rectangle play_again_ButtonRect ={ WIDTH/4 - 80,HEIGHT - 80,150,60};
           DrawTexturePro(play_again,
                       (Rectangle){0, 0, play_again.width, play_again.height},
                       play_again_ButtonRect, Vector2Zero(), 0, WHITE);

          Rectangle return_ButtonRect ={ WIDTH*3/4 - 80,HEIGHT - 80,150,60};
          DrawTexturePro(Re_turn,
                       (Rectangle){0, 0, Re_turn.width, Re_turn.height},
                       return_ButtonRect, Vector2Zero(), 0, WHITE);

      
        break;

}
  }

     
    /*
    int fontSize = 36;
    const char *scoreStr = TextFormat("%d", score);
    const char *highStr  = TextFormat("%d", highScore);

    int scoreTextW = MeasureText(scoreStr, fontSize);
    int highTextW  = MeasureText(highStr, fontSize);

    DrawText(scoreStr,
             yourScoreRect.x + yourScoreRect.width/2 - scoreTextW/2,
             yourScoreRect.y + yourScoreRect.height + 10,
             fontSize, WHITE);

    DrawText(highStr,
             highScoreRect.x + highScoreRect.width/2 - highTextW/2,
             highScoreRect.y + highScoreRect.height + 10,
             fontSize, WHITE);
    
    */

     
  

EndDrawing();

}
  UnloadTexture(spaceship);
    UnloadTexture(start);
    UnloadTexture(menu);
   // UnloadTexture(scoreboard_bg);
    //UnloadTexture(name_bg);
    UnloadTexture(startbutton);
    UnloadTexture(playbutton);
    UnloadTexture(helpbutton);
    UnloadTexture(settingsbutton);
    UnloadTexture(exitbutton);
    UnloadTexture(leaderbutton);
   // UnloadTexture(alienTex);
    UnloadTexture(mship);
    UnloadTexture(Laser);
    UnloadTexture(Life);

    //for (int i = 0; i < 6; i++) UnloadTexture(background[i]);
    

   
   UnloadTexture(Alien[0].picture);
    UnloadMusicStream(music);
    UnloadSound(sound1);
    UnloadSound(sound2);
    UnloadSound(sound3);
   UnloadSound(sound4);


   UnloadTexture(soundOnlogo);
UnloadTexture(soundOfflogo);

UnloadTexture(game_time_set_on);
UnloadTexture(game_time_soundon);
UnloadTexture(game_time_soundoff);
UnloadTexture(game_time_pause);
UnloadTexture(game_time_end);
UnloadTexture(game_time_start);
UnloadTexture(game_time_set_off);

UnloadTexture(scoreboard);
UnloadTexture(score_header);
UnloadTexture(help_menu);

UnloadTexture(shield);
UnloadTexture(extlife);
UnloadTexture(extpoints);
UnloadTexture(laser);
UnloadTexture(megaweapon);
UnloadTexture(rapidfire);
UnloadTexture(magnet);
UnloadTexture(bubble);
UnloadTexture(triple_shot);
UnloadTexture(speed_boost);

UnloadTexture(name);
UnloadTexture(game);
UnloadTexture(backbutton);
UnloadTexture(enter_name);

UnloadTexture(Game_over_back);
UnloadTexture(game_over_text);
UnloadTexture(high_score_text);
UnloadTexture(your_score_text);
UnloadTexture(play_again);
UnloadTexture(Re_turn);

for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 6; j++) {
        UnloadTexture(Level_arr[i].game_back[j]);
    }
}

for (int i = 0; i < NUM_LEVEL; i++) {
    UnloadTexture(levelButtons[i].button_pic);
}

    CloseAudioDevice();
    CloseWindow();


  return 0;

}