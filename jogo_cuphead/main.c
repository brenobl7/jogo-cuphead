#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/keyboard.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

int main() {
    // Inicializaï¿½ï¿½es bï¿½sicas
    al_init();
    al_set_app_name("Dark Punch");
    al_init_image_addon();
    al_install_mouse();
    al_install_keyboard();
    al_install_audio();
    al_init_acodec_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    // Recursos do jogo
    ALLEGRO_DISPLAY* display = al_create_display(800, 600);
    al_set_window_position(display, 300, 100);

    ALLEGRO_BITMAP* Menu = al_load_bitmap("./imagens/Menu.JPG");
    ALLEGRO_BITMAP* sprite = al_load_bitmap("./imagens/teste.PNG");
    ALLEGRO_BITMAP* lobo = al_load_bitmap("./imagens/teste.PNG");
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    ALLEGRO_SAMPLE* menu_audio = al_load_sample("./audios/boss2.OGG");
    ALLEGRO_SAMPLE* jogo_audio = al_load_sample("./audios/boss1.OGG");
    ALLEGRO_FONT* fonte = al_load_font("./fonte.ttf",30,0);
    ALLEGRO_FONT* vida = al_load_font("./fonte.ttf",30,0);

    if (!Menu || !sprite || !menu_audio || !jogo_audio || !lobo) {
        printf("Erro ao carregar recursos.\n");
        return -1;
    }

    // Variï¿½veis de controle

    int pos_x = 300, pos_y = 400; // Posiï¿½ï¿½o inicial do personagem
    int current_frame_y = 240;    // Coordenada inicial do estado parado
    float frame = 0.0;            // Controle de animaï¿½ï¿½o
    float vel_y = 0;              // Velocidade vertical
    float gravity = 0.5;          // Gravidade
    bool is_jumping = false;      // Estado de pulo
    bool is_attacking = false;    // Estado de ataque
    bool facing_left = false;     // Direï¿½ï¿½o do personagem
    bool moving_left = false;     // Movimento para esquerda
    bool moving_right = false;    // Movimento para direita
    const int ground_y = 400;     // Altura do chï¿½o
    bool in_menu = true;          // Estado do menu
    int lobo_x = -50,lobo_y = 450;  //posiï¿½ï¿½es iniciais do lobo
    float frame_lobo = 0.0;        // frame do lobo
    int current_frame_y_lobo = 179;  // imagem atual
    int tamanhos = 10; // limitador de sprite
    bool lobo_direita = false,lobo_esquerda = false, lobo_ataque = false, ultimo_clique = NULL ; // variaveis de movimento
    int largura_lobo = 138 ;//  largura da sprite lobo
    int vida_samurai = 200; //vida do samurai
    int vida_lobo = 150; //vida lobo
    int contador = 0;

    // Configurando eventos
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_start_timer(timer);

    // Configurar reproduï¿½ï¿½o de ï¿½udio
    al_reserve_samples(2);
    al_play_sample(menu_audio, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL); // Loop no menu

    // Loop principal
    while (true) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }

        if (in_menu) {
            // Controle do menu
            int mouse_x = event.mouse.x;
            int mouse_y = event.mouse.y;

            if (266 <= mouse_x && mouse_x <= 532 && 225 <= mouse_y && mouse_y <= 300) {
                if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1) {
                    in_menu = false;
                    al_stop_samples(); // Para o ï¿½udio do menu
                    al_play_sample(jogo_audio, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL); // Loop no jogo

                    // Carrega os recursos do jogo
                    Menu = al_load_bitmap("./imagens/background.PNG");
                    sprite = al_load_bitmap("./imagens/sprite_certas.PNG");
                    lobo = al_load_bitmap("./imagens/lobo.PNG");
                }
            }
        } else {
            // Controle do teclado
            if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                // Volta para o menu ao pressionar ESC
                if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    in_menu = true;
                    vida_lobo = 100;
                    vida_samurai = 200;
                    contador = 0;
                    al_stop_samples(); // Para o ï¿½udio do jogo
                    al_play_sample(menu_audio, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL); // Loop no menu
                    lobo_x = -50;

                    // Restaura o estado inicial
                    Menu = al_load_bitmap("./imagens/Menu.JPG");
                    sprite = al_load_bitmap("./imagens/sprite_certasl.PNG");
                }

                // Controle do pulo
                if (event.keyboard.keycode == ALLEGRO_KEY_UP && !is_jumping && !is_attacking) {
                    vel_y = -10;
                    is_jumping = true;
                }

                // Controle do movimento para a direita
                if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT && !is_attacking) {
                    moving_right = true;
                    facing_left = false;
                    current_frame_y = 200; // Andando para a direita
                }

                // Controle do movimento para a esquerda
                if (event.keyboard.keycode == ALLEGRO_KEY_LEFT && !is_attacking) {
                    moving_left = true;
                    facing_left = true;
                    current_frame_y = 25; // Andando para a esquerda
                }

                // Controle do ataque
                if (event.keyboard.keycode == ALLEGRO_KEY_X && !is_attacking) {
                    is_attacking = true;
                    frame = 0; // Inï¿½cio da animaï¿½ï¿½o de ataque
                    current_frame_y = facing_left ? 1340 : 1500; // Ataque para a direï¿½ï¿½o correta
                }
            }

            if (event.type == ALLEGRO_EVENT_KEY_UP) {
                // Libera o movimento apï¿½s soltar as teclas de direï¿½ï¿½o
                if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                    moving_right = false;
                }
                if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                    moving_left = false;
                }
            }

            // Atualizaï¿½ï¿½o da posiï¿½ï¿½o apenas se nï¿½o estiver atacando
            if (!is_attacking) {
                if (moving_right) pos_x += 2;
                if (moving_left) pos_x -= 2;
            }

            // Controle de fï¿½sica do pulo
            if (is_jumping) {
                pos_y += vel_y;
                vel_y += gravity;
                if (pos_y >= ground_y) {
                    pos_y = ground_y;
                    vel_y = 0;
                    is_jumping = false;
                }
            }

            // Animaï¿½ï¿½o de ataque
            if (is_attacking) {
                frame += 0.2;
                if (frame >= 6) { // Finaliza o ataque apï¿½s 6 frames
                    is_attacking = false; // Termina o ataque
                    frame = 0; // Reseta a animaï¿½ï¿½o

                    // Retorna ao estado parado ou andando
                    if (moving_right) {
                        current_frame_y = 200; // Andando para a direita
                    } else if (moving_left) {
                        current_frame_y = 25; // Andando para a esquerda
                    } else {
                        current_frame_y = 380; // Estado parado
                    }
                }
                if (facing_left==true && pos_x-70 <=lobo_x+40 && lobo_x <= pos_x-10 ){//ataque esquerda
                    vida_lobo -= 1.5;


                }
                 if (facing_left==false && pos_x+90 >=lobo_x-40 && lobo_x >= pos_x+10 ){//ataque direita (fazer)
                    vida_lobo -= 1.5;


                }
            }

            // Estado parado
            if (!moving_left && !moving_right && !is_jumping && !is_attacking) {
                current_frame_y = 380;
                frame = 0; // Reseta a animaï¿½ï¿½o
            } else if (!is_attacking) {
                // Animaï¿½ï¿½o de movimento
                frame += 0.07;
                if (frame > 6) frame = 3.0;
            }
        }
        // Renderizaï¿½ï¿½o
        al_clear_to_color(al_map_rgb(0, 0, 0));
        if (in_menu) {
            al_draw_bitmap(Menu, 0, 0, 0);
        } else {

            //mexendo lobo

            frame_lobo += 0.07;
            if(frame_lobo>tamanhos){
                frame_lobo = 1.0;
            }

            if(lobo_x<pos_x-10){
                lobo_direita = true;
                lobo_esquerda = false;
                lobo_ataque = false;
                ultimo_clique = true;
                largura_lobo = 138;

            }else if(lobo_x>pos_x+50){
                lobo_direita = false;
                lobo_esquerda = true;
                lobo_ataque = false;
                ultimo_clique = false;
                largura_lobo = 138;

            }else if(lobo_x == pos_x-10 || lobo_x == pos_x+50){
                lobo_ataque = true;
                lobo_direita = false;
                lobo_esquerda = false;
                tamanhos = 4;
            }
            if(lobo_direita){
                lobo_x += 1;
                current_frame_y_lobo = 179;
                lobo_y = 450;
            }
            if(lobo_esquerda){
                lobo_x -= 1;
                current_frame_y_lobo = 281;
                lobo_y = 450;
            }
            if(!lobo_direita && !lobo_esquerda && lobo_ataque ){
                if(!ultimo_clique){
                    current_frame_y_lobo = 964;
                    largura_lobo = 150;
                    lobo_y = 430;
;                }
                if(ultimo_clique){
                    current_frame_y_lobo = 844;
                    largura_lobo = 150;
                    lobo_y = 430;
                }
                vida_samurai -= 0.5;
            }

            if(vida_samurai <=0){
                    in_menu = true;
                    vida_lobo = 150;
                    vida_samurai = 200;
                    contador = 0;
                    al_stop_samples(); // Para o ï¿½udio do jogo
                    al_play_sample(menu_audio, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
                     Menu = al_load_bitmap("./imagens/Menu.JPG");
                    lobo_x = -50;
            }



            al_draw_bitmap(Menu, 0, 0, 0);
            al_draw_bitmap_region(sprite, 180 * (int)frame, current_frame_y, 150, 160, pos_x, pos_y, 0);

            // Desenha o personagem
            if(vida_lobo<=0){
                lobo_direita = false;
                lobo_ataque = false;
                lobo_esquerda = false;
                ultimo_clique = false;
                vida_lobo = 150;
                int k = rand() % 2;
                if (k == 0){
                    lobo_x = -100;
                }
                if (k == 1){
                    lobo_x = +800;
                }
                contador +=1;

            }
            if(vida_lobo<=90){

                al_draw_textf(vida,al_map_rgb(255,0,0),lobo_x+40,410,0,"%d",vida_lobo);
            }
            if(vida_lobo>90){
                al_draw_textf(vida,al_map_rgb(0,255,0),lobo_x+40,410,0,"%d",vida_lobo);
            }
            al_draw_textf(fonte,al_map_rgb(255,255,255),20,20,0,"SCORE: %d",contador);
            if(vida_samurai>=100){

                al_draw_textf(vida,al_map_rgb(0,255,0),550,20,0,"VIDA: %d",vida_samurai);
            }
            if(vida_samurai<100){
                al_draw_textf(vida,al_map_rgb(255,0,0),550,20,0,"VIDA: %d",vida_samurai);
            }

            al_draw_bitmap_region(lobo,largura_lobo*(int)frame_lobo,current_frame_y_lobo,130,120,(int)lobo_x,(int)lobo_y,0);
        }

        if(contador == 10){

            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_text(fonte, al_map_rgb(255, 255, 255), 300, 200, 0, "Parabens!");
            al_draw_text(fonte, al_map_rgb(255, 255, 255), 180, 270, 0, "Voce terminou o jogo.");



            al_stop_samples();

        }
        if(pos_x <= -50){
            pos_x = -50;
        }
        if(pos_x >= 672){
            pos_x = 672;
        }

        al_flip_display();

    }

    // Fecha tuso
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_bitmap(Menu);
    al_destroy_bitmap(sprite);
    al_destroy_timer(timer);
    al_destroy_sample(menu_audio);
    al_destroy_sample(jogo_audio);

    al_destroy_bitmap(lobo);
    al_destroy_font(fonte);
    al_destroy_font(vida);

    return 0;
}
