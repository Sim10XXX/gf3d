#include <SDL.h>            

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"
#include "gfc_string.h"
#include "gfc_actions.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "gf3d_texture.h"
#include "gf3d_draw.h"

#include "entity.h"
#include "player.h"
#include "block.h"
#include "map.h"

extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 33;
static float fps = 0;

void parse_arguments(int argc,char *argv[]);
void game_frame_delay();

void exitGame()
{
    _done = 1;
}

void draw_origin()
{
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(-100,0,0),gfc_vector3d(100,0,0)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(1,0,0,1));
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(0,-100,0),gfc_vector3d(0,100,0)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(0,1,0,1));
    gf3d_draw_edge_3d(
        gfc_edge3d_from_vectors(gfc_vector3d(0,0,-100),gfc_vector3d(0,0,100)),
        gfc_vector3d(0,0,0),gfc_vector3d(0,0,0),gfc_vector3d(1,1,1),0.1,gfc_color(0,0,1,1));
}

void dino_think(Entity *self) {
    GFC_Vector2D w;
    float m = 0.5;
    const Uint8* keys;

    keys = SDL_GetKeyboardState(NULL);
    if (!self) return;
    if (keys[SDL_SCANCODE_UP]) {
    //if (gfc_input_key_down('w')) {
        w = gfc_vector2d_from_angle(self->rotation.z);
        self->position.x += w.x * m;
        self->position.y += w.y * m;
        //slog("yaaa");
    }
    //self->rotation.x+= 0.01;
    //slog("Thinking is happening");
}

int main(int argc,char *argv[])
{
    //local variables
    Model *sky,*dino;
    GFC_Matrix4 skyMat,dinoMat;

    Entity *player;
    //initializtion    
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0);
    slog("gf3d begin");
    //gfc init
    gfc_input_init("config/input.cfg");
    gfc_config_def_init();
    gfc_action_init(1024);
    //gf3d init
    gf3d_vgraphics_init("config/setup.cfg");
    gf3d_materials_init();
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);
    gf3d_draw_init();//3D
    gf2d_draw_manager_init(1000);//2D
    
    //game init
    srand(SDL_GetTicks());
    slog_sync();

    //game setup
    gf2d_mouse_load("actors/mouse.actor");
    sky = gf3d_model_load("models/sky.model");
    gfc_matrix4_identity(skyMat);
    dino = gf3d_model_load("models/dino.model");
    gfc_matrix4_identity(dinoMat);
        //camera
    gf3d_camera_set_scale(gfc_vector3d(1,1,1));
    gf3d_camera_set_position(gfc_vector3d(15,-15,10));
    gf3d_camera_look_at(gfc_vector3d(0,0,0),NULL);
    gf3d_camera_set_move_step(0.2);
    gf3d_camera_set_rotate_step(0.05);
    
    gf3d_camera_enable_free_look(1);
    //windows
    entity_system_init(1000);

    player = spawn_player();
    playerData* pdata = player->data;
    pdata->mapData = load_map_from_cfg("config/map.cfg");
    //if (!pdata->mapData) return 400;
    Entity* startBlock = pdata->mapData->startBlock;
    player->position = startBlock->position;
    player->rotation = startBlock->rotation;

    Entity* block = spawn_block(2);
    block->position = gfc_vector3d(0,30,0);
    block->rotation = gfc_vector3d(0, 0, 0);//GFC_HALF_PI
    block->scale = gfc_vector3d(1, 1, 1);

    block = spawn_block(2);
    block->position = gfc_vector3d(70, 30, 30);
    block->rotation = gfc_vector3d(GFC_PI, 0, 0);//GFC_HALF_PI
    block->scale = gfc_vector3d(2, 1, 3);
    //if (ent)
    //{
    //    ent->model = dino;
    //    ent->think = player_think;
        //ent->position = gfc_vector3d(0, 0, 0);
    //}

    // main game loop    
    Entity* stadium = entity_new();
    stadium->model = gf3d_model_load("models/stadium.model");
    stadium->colliding = 1;

    while(!_done)
    {
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();
        //camera updaes
        gf3d_camera_controls_update();
        gf3d_camera_update_view();
        gf3d_camera_get_view_mat4(gf3d_vgraphics_get_view_matrix());
        
        gf3d_vgraphics_render_start();
        
        entity_think_all();
        entity_update_all();
        
        
        
        entity_draw_all();
            //3D draws
        
                gf3d_model_draw_sky(sky,skyMat,GFC_COLOR_WHITE);
                /*gf3d_model_draw(
                    dino,
                    dinoMat,
                    GFC_COLOR_WHITE,
                    0);*/
                draw_origin();
            //2D draws
                gf2d_mouse_draw();
                gf2d_font_draw_line_tag("ALT+F4 to exit",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));
                
                //draw speed text

                int w = 1050, h = 680;
                //SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);
                char outputtext[20] = "Speed: ";
                char speed[10];
                float s = gfc_vector3d_magnitude(pdata->positionVelocity) * 100;
                sprintf(speed, "%i", _cvt_ftoi_fast(s));
                

                strcat(outputtext, speed);
                //slog("String: %s", outputtext);
                gf2d_font_draw_line_tag(outputtext, FT_H1, GFC_COLOR_BLACK, gfc_vector2d(w - 8, h - 8));
                gf2d_font_draw_line_tag(outputtext, FT_H1, GFC_COLOR_WHITE, gfc_vector2d(w-10, h-10));


                //draw current time

                //char time[10];
                //sprintf(time, "%i", pdata->framecount);

                sprintf(outputtext, "Time: %.2f", (pdata->framecount)/30.0);
                if (pdata->gameState == 0) {
                    gf2d_font_draw_line_tag(outputtext, FT_H1, GFC_COLOR_BLACK, gfc_vector2d(w / 2, h - 8));
                    gf2d_font_draw_line_tag(outputtext, FT_H1, GFC_COLOR_WHITE, gfc_vector2d(w / 2 - 2, h - 10));
                }
                else if (pdata->gameState == 1) {
                    gf2d_font_draw_line_tag(outputtext, FT_Large, GFC_COLOR_BLACK, gfc_vector2d(w / 2, h / 2 - 8));
                    gf2d_font_draw_line_tag(outputtext, FT_Large, GFC_COLOR_WHITE, gfc_vector2d(w / 2 - 2, h / 2 - 10));
                }
                


        gf3d_vgraphics_render_end();
        if (gfc_input_command_down("exit"))_done = 1; // exit condition
        game_frame_delay();
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    exit(0);
    slog_sync();
    return 0;
}

void parse_arguments(int argc,char *argv[])
{
    int a;

    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"--debug") == 0)
        {
            __DEBUG = 1;
        }
    }    
}

void game_frame_delay()
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    then = now;
    slog_sync();// make sure logs get written when we have time to write it
    now = SDL_GetTicks();
    diff = (now - then);
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then,0.001);
//     slog("fps: %f",fps);
}
/*eol@eof*/
